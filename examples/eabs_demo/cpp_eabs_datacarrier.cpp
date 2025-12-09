// cpp_eabs_datacarrier.cpp
//
// Connects to a CARLA server, finds the ego vehicle by role_name,
// computes the EABS status, and writes it into a "data carrier"
// actor (static.prop.box01) by encoding the status in location.x.

#include <carla/client/Actor.h>
#include <carla/client/ActorBlueprint.h>
#include <carla/client/ActorList.h>
#include <carla/client/BlueprintLibrary.h>
#include <carla/client/Client.h>
#include <carla/client/Map.h>
#include <carla/client/TimeoutException.h>
#include <carla/client/Timestamp.h>
#include <carla/client/Vehicle.h>
#include <carla/client/World.h>
#include <carla/client/WorldSnapshot.h>

#include <carla/Memory.h>
#include <carla/geom/Location.h>
#include <carla/geom/Transform.h>
#include <carla/geom/Rotation.h>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <vector>
#include <sched.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "mqtt_metrics.h"
#include "config.h"

namespace cc  = carla::client;
namespace cg  = carla::geom;
using carla::SharedPtr;
using namespace std::chrono_literals;

// ----------------------------------------------------------------------------
// Logging
// ----------------------------------------------------------------------------

static std::ofstream g_log_file;

template<typename... Args>
void log_print(Args&&... args) {
  const Config& cfg = Config::GetInstance();
  if (!cfg.log_enabled()) return;

  std::ostream *os = &std::cout;
  if (cfg.log_to_file() && g_log_file.is_open()) {
    os = &g_log_file;
  }

  // C++11-friendly pack expansion.
  (void)std::initializer_list<int>{
    ((*os << std::forward<Args>(args)), 0)...
  };
  *os << std::endl;
  os->flush();
}

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------

std::string GetActorDisplayName(const cc::Actor &actor, std::size_t truncate = 250u) {
  // Use type id, similar spirit to Python version.
  const std::string &type_id = actor.GetTypeId();
  std::string name = type_id;
  if (name.size() > truncate) {
    name = name.substr(0, truncate - 1) + u8"\u2026";
  }
  return name;
}

double Distance3D(const cg::Location &a, const cg::Location &b) {
  const double dx = static_cast<double>(a.x) - static_cast<double>(b.x);
  const double dy = static_cast<double>(a.y) - static_cast<double>(b.y);
  const double dz = static_cast<double>(a.z) - static_cast<double>(b.z);
  return std::sqrt(dx*dx + dy*dy + dz*dz);
}

double VectorLength(const cg::Vector3D &v) {
  const double x = static_cast<double>(v.x);
  const double y = static_cast<double>(v.y);
  const double z = static_cast<double>(v.z);
  return std::sqrt(x*x + y*y + z*z);
}

// ----------------------------------------------------------------------------
// Global MQTT metrics instance
// ----------------------------------------------------------------------------
static std::unique_ptr<MqttMetrics> g_metrics;

void SendMqttEvent(const std::string &tag, std::int64_t value = 0) {
  if (g_metrics) {
    g_metrics->SendTimestamp(tag, value);
  }
}

// ----------------------------------------------------------------------------
// DataCarrierWriter: manages static.prop.box01 underground and writes status
// ----------------------------------------------------------------------------

class DataCarrierWriter {
public:
  explicit DataCarrierWriter(cc::World world)
    : _world(std::move(world)), _current_status(0) {
    SetupDataCarrier();
  }

  bool IsValid() const {
    return static_cast<bool>(_data_carrier);
  }

  bool WriteStatus(int status, std::uint64_t start_loop_time) {
    if (!_data_carrier) {
      log_print("Apply EABS C++: Data carrier not available");
      return false;
    }

    // Clamp to [0, 999]
    if (status < 0)   status = 0;
    if (status > 999) status = 999;

    try {
      cg::Location loc;
      loc.x = static_cast<float>(status);
      loc.y = 0.0f;
      loc.z = -1000.0f;

      cg::Rotation rot;
      rot.pitch = 0.0f;
      rot.yaw   = 0.0f;
      rot.roll  = 0.0f;

      cg::Transform tr(loc, rot);
      SendMqttEvent("eabs_processing_time", NowNanoseconds() - start_loop_time);
      _data_carrier->SetTransform(tr);

      log_print("Apply EABS C++: Status updated: ",
                _current_status, " -> ", status);
      _current_status = status;
      return true;
    } catch (const std::exception &e) {
      log_print("Apply EABS C++: Error writing status: ", e.what());
      return false;
    }
  }

  int ReadStatus() const {
    if (!_data_carrier) {
      return 0;
    }
    try {
      cg::Location loc = _data_carrier->GetLocation();
      int status = static_cast<int>(loc.x) % 1000;
      if (status < 0) status += 1000;
      return status;
    } catch (...) {
      return 0;
    }
  }

  void Cleanup() {
    if (_data_carrier) {
      try {
        _data_carrier->Destroy();
        log_print("Apply EABS C++: Data carrier destroyed");
      } catch (const std::exception &e) {
        log_print("Apply EABS C++: Error destroying data carrier: ", e.what());
      }
      _data_carrier.reset();
    }
  }

private:
  void SetupDataCarrier() {
    try {
      // Check if data carrier already exists
      auto actors = _world.GetActors();
      auto boxes  = actors->Filter("static.prop.box01");
      for (auto actor : *boxes) {
        if (!actor) continue;
        cg::Location loc = actor->GetLocation();
        if (loc.z < -900.0f) {
          _data_carrier = actor;
          log_print("Apply EABS C++: Found existing data carrier, reusing it");
          return;
        }
      }

      // Spawn new data carrier
      auto bp_lib = _world.GetBlueprintLibrary();
      auto maybe_bp = bp_lib->Find("static.prop.box01");
      if (!maybe_bp) {
        throw std::runtime_error("Blueprint 'static.prop.box01' not found");
      }
      const cc::ActorBlueprint &bp = *maybe_bp;

      cg::Location loc;
      loc.x = 0.0f;
      loc.y = 0.0f;
      loc.z = -1000.0f;

      cg::Rotation rot;
      rot.pitch = 0.0f;
      rot.yaw   = 0.0f;
      rot.roll  = 0.0f;

      cg::Transform tr(loc, rot);
      _data_carrier = _world.SpawnActor(bp, tr);
      log_print("Apply EABS C++: Created new data carrier actor");
    } catch (const std::exception &e) {
      log_print("Apply EABS C++: Could not create data carrier: ", e.what());
      _data_carrier.reset();
    }
  }

  cc::World _world;
  SharedPtr<cc::Actor> _data_carrier;
  int _current_status;
};

// ----------------------------------------------------------------------------
// ExternalController: monitors ego vehicle and writes EABS status
// ----------------------------------------------------------------------------

class ExternalController {
public:
  ExternalController(const std::string &host,
                     uint16_t port,
                     const std::string &role_name,
                     bool sync)
    : _host(host),
      _port(port),
      _role_name(role_name),
      _sync(sync),
      _client(host, port),
      _world(_client.GetWorld()),
      _data_writer(_world),
      _last_frame(std::numeric_limits<std::size_t>::max()) {

    _client.SetTimeout(10s);
    log_print("Apply EABS C++: Connected to CARLA at ",
              _host, ":", _port);
  }

  void Run() {
    log_print("====================================================================");
    log_print("CARLA External Controller - C++");
    log_print("Data Carrier Communication (Single Status Integer)");
    log_print("====================================================================");
    log_print("   Monitoring ego vehicle and writing status codes");
    log_print("   Press Ctrl+C to stop\n");

    try {
      while (true) {
        auto snapshot  = _world.GetSnapshot();
        const auto &ts = snapshot.GetTimestamp();
        std::size_t frame = ts.frame;

        if (frame == _last_frame) {
          // Same frame; wait a bit and try again.
          std::this_thread::sleep_for(1ms);
          continue;
        }
        _current_timestamp = ts;

        // Ensure ego vehicle exists
        if (!_ego_vehicle || !_ego_vehicle->IsAlive()) {
          if (!FindEgoVehicle()) {
            log_print("Apply EABS C++: Waiting for ego vehicle with role_name=",
                      _role_name, " ...");
            std::this_thread::sleep_for(500ms);
            _last_frame = frame;
            continue;
          }
        }

        // Check if ego actor still valid
        try {
          (void)_ego_vehicle->GetLocation();
        } catch (const std::exception &e) {
          log_print("Apply EABS C++: Ego vehicle invalid/destroyed: ", e.what());
          _ego_vehicle.reset();
          _last_frame = frame;
          continue;
        }

        // Compute EABS status
        const auto loop_start = std::chrono::high_resolution_clock::now();
        int status = CalculateEABSStatus();
        const auto loop_end   = std::chrono::high_resolution_clock::now();
        const double elapsed  = std::chrono::duration<double>(loop_end - loop_start).count();

        // Write status
        _data_writer.WriteStatus(status, start_loop_time);

        // Debug info
        std::string status_name = StatusName(status);
        log_print("Apply EABS C++: FRAME=", frame,
                  " Status=", status, " (", status_name, ")",
                  " loop_time=", elapsed, " s");

        _last_frame = frame;
      }
    } catch (const cc::TimeoutException &e) {
      log_print("Apply EABS C++: TimeoutException: ", e.what());
      throw;
    } catch (const std::exception &e) {
      log_print("Apply EABS C++: Exception in Run(): ", e.what());
      throw;
    }
  }

  void Cleanup() {
    _data_writer.Cleanup();
  }

private:
  std::int64_t start_loop_time;

  bool FindEgoVehicle() {
    auto actors   = _world.GetActors();
    auto vehicles = actors->Filter("vehicle.*");

    for (auto actor : *vehicles) {
      if (!actor) continue;

      const auto &attrs = actor->GetAttributes();
      for (const auto &attr : attrs) {
        // ActorAttributeValueAccess::As<std::string>() gives string value.
        if (attr.GetId() == "role_name" &&
            attr.As<std::string>() == _role_name) {
          _ego_vehicle = actor;
          log_print("Apply EABS C++: Found ego vehicle (ID=",
                    actor->GetId(), ", type=", actor->GetTypeId(), ")");
          return true;
        }
      }
    }
    _ego_vehicle.reset();
    return false;
  }

  // Returns up to max_count nearest vehicles (excluding ego) with distances.
  struct NearestCandidate {
    SharedPtr<cc::Actor> actor;
    double distance;
  };

  std::vector<NearestCandidate> GetNearestVehicles(std::size_t max_count) {
    std::vector<NearestCandidate> result;
    auto ego = _ego_vehicle;
    if (!ego) {
      return result;
    }
    auto actors_all = _world.GetActors();
    start_loop_time = NowNanoseconds();
    if (!actors_all) {
      return result;
    }
    auto vehicles = actors_all->Filter("vehicle.*");
    result.reserve(16);
    const cg::Location ego_loc = ego->GetLocation();
    for (auto other : *vehicles) {
      if (!other) continue;
      if (other->GetId() == ego->GetId()) continue;
      const cg::Location oth_loc = other->GetLocation();
      const double dist = Distance3D(ego_loc, oth_loc);
      result.push_back({other, dist});
    }
    if (result.empty()) {
      return result;
    }
    std::sort(result.begin(), result.end(), [](const NearestCandidate &a, const NearestCandidate &b){ return a.distance < b.distance; });
    if (result.size() > max_count) {
      result.resize(max_count);
    }
    return result;
  }

  int CalculateEABSStatus() {
    // Get thresholds from configuration
    const Config& cfg = Config::GetInstance();
    const double ttc_warning       = cfg.ttc_warning();
    const double ttc_mild_braking  = cfg.ttc_mild_braking();
    const double ttc_strong_brakes = cfg.ttc_strong_brakes();
    const double lateral_extra_margin = cfg.lateral_extra_margin();
    const double min_v_rel            = cfg.min_v_rel();

    auto ego = _ego_vehicle;
    if (!ego) {
      return 0; // not connected
    }

    // --- Ego state -----------------------------------------------------
    const cg::Location ego_loc  = ego->GetLocation();
    const cg::Vector3D ego_vel  = ego->GetVelocity();
    const cg::Transform ego_tf  = ego->GetTransform();
    cg::Vector3D fwd            = ego_tf.GetForwardVector();
    cg::Vector3D right          = ego_tf.GetRightVector();

    auto normalize_vec = [](cg::Vector3D v) {
      const double len = std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
      if (len > 1e-6) {
        v.x = static_cast<float>(v.x / len);
        v.y = static_cast<float>(v.y / len);
        v.z = static_cast<float>(v.z / len);
      }
      return v;
    };

    fwd   = normalize_vec(fwd);
    right = normalize_vec(right);

    // --- Lane width ----------------------------------------
    double lane_width = 3.5; // fallback
    try {
      auto map = _world.GetMap();
      if (map) {
        auto ego_wp = map->GetWaypoint(ego_loc);
        if (ego_wp) {
          lane_width = ego_wp->GetLaneWidth();
        }
      }
    } catch (...) {
      // keep default lane_width
    }

    // --- Collect nearest vehicles --------------------------------------
    auto candidates = GetNearestVehicles(5u);
    if (candidates.empty()) {
      log_print("Apply EABS C++: outside conditions (no other vehicles) frame=",
                _current_timestamp.frame,
                " platform_timestamp=", _current_timestamp.platform_timestamp);
      return 1;
    }

    double best_ttc   = std::numeric_limits<double>::infinity();
    bool   have_ttc   = false;
    int    eabs_count = 0;

    for (const auto &cand : candidates) {
      auto other = cand.actor;
      if (!other) continue;
      ++eabs_count;

      const cg::Location oth_loc = other->GetLocation();
      const cg::Vector3D oth_vel = other->GetVelocity();
      const std::string  oth_type = GetActorDisplayName(*other, 22);

      // Relative position in world coordinates
      cg::Vector3D rel_pos;
      rel_pos.x = oth_loc.x - ego_loc.x;
      rel_pos.y = oth_loc.y - ego_loc.y;
      rel_pos.z = oth_loc.z - ego_loc.z;

      // Project into ego frame
      const double longitudinal =
        rel_pos.x * fwd.x + rel_pos.y * fwd.y + rel_pos.z * fwd.z;
      const double lateral =
        rel_pos.x * right.x + rel_pos.y * right.y + rel_pos.z * right.z;

      // Ignore objects behind ego or overlapping
      if (longitudinal <= 0.0) {
        continue;
      }

      // Ignore vehicles not in (roughly) same lane
      if (std::fabs(lateral) > (0.5 * lane_width + lateral_extra_margin)) {
        continue;
      }

      // Relative velocity along ego forward
      cg::Vector3D rel_vel;
      rel_vel.x = ego_vel.x - oth_vel.x;
      rel_vel.y = ego_vel.y - oth_vel.y;
      rel_vel.z = ego_vel.z - oth_vel.z;

      const double v_rel =
        rel_vel.x * fwd.x + rel_vel.y * fwd.y + rel_vel.z * fwd.z;

      // Not closing or closing too slowly → ignore
      if (v_rel <= min_v_rel) {
        continue;
      }

      const double ttc = longitudinal / v_rel; // seconds

      log_print("Apply EABS C++ TTC candidate: frame=",
                _current_timestamp.frame,
                " long=", longitudinal, "m",
                " lat=", lateral, "m",
                " v_rel=", v_rel, "m/s",
                " TTC=", ttc, "s",
                " other=", oth_type);

      if (ttc < best_ttc) {
        best_ttc = ttc;
        have_ttc = true;
      }
    }

    int status = 1;

    if (have_ttc) {
      if (best_ttc < ttc_strong_brakes) {
        status = 4;
      } else if (best_ttc < ttc_mild_braking) {
        status = 3;
      } else if (best_ttc < ttc_warning) {
        status = 2;
      } else {
        status = 1;
      }
    }

    log_print("Apply EABS C++: final status frame=",
              _current_timestamp.frame,
              " platform_timestamp=", _current_timestamp.platform_timestamp,
              " best_ttc=", (have_ttc ? best_ttc : -1.0),
              " status=", status);

    return status;
  }

  static std::string StatusName(int status) {
    switch (status) {
      case 0: return "not connected";
      case 1: return "activated";
      case 2: return "Warning";
      case 3: return "Mild Braking";
      case 4: return "Strong Braking";
      default: return "Unknown";
    }
  }

  // Config
  std::string _host;
  uint16_t    _port;
  std::string _role_name;
  bool        _sync;

  // CARLA
  cc::Client  _client;
  cc::World   _world;
  SharedPtr<cc::Actor> _ego_vehicle;
  DataCarrierWriter    _data_writer;

  // Timing
  carla::client::Timestamp _current_timestamp{};
  std::size_t _last_frame;
};

// ----------------------------------------------------------------------------
// Command-line parsing
// ----------------------------------------------------------------------------

// Configuration is now handled by Config singleton

// ----------------------------------------------------------------------------
// main()
// ----------------------------------------------------------------------------

int main(int argc, char *argv[]) {
  try {
    // Initialize configuration from INI file and command line
    // Try system config first, then local config
    Config& cfg = Config::GetInstance();
    const char* config_file = "/etc/libcarla/eabs_demo.ini";
    std::ifstream test_file(config_file);
    if (!test_file.good()) {
      config_file = "eabs_demo.ini";
    }
    test_file.close();
    cfg.Initialize(config_file, argc, argv);

    // Open log file if logging to file is enabled
    if (cfg.log_enabled() && cfg.log_to_file()) {
      g_log_file.open(cfg.log_file());
    }

    // Create and start MQTT metrics with configured values
    g_metrics = std::make_unique<MqttMetrics>(
        cfg.mqtt_host(),
        cfg.mqtt_port(),
        "eabs_cpp_client");
    g_metrics->Start();

    log_print("Apply EABS C++: starting with");
    log_print("  host      = ", cfg.carla_host());
    log_print("  port      = ", cfg.carla_port());
    log_print("  rolename  = ", cfg.role_name());
    log_print("  sync flag = ", (cfg.sync_mode() ? "true" : "false"));
    log_print("  mqtt host = ", cfg.mqtt_host());
    log_print("  mqtt port = ", cfg.mqtt_port());
    log_print("  ttc warn  = ", cfg.ttc_warning());
    log_print("  ttc mild  = ", cfg.ttc_mild_braking());
    log_print("  ttc strong= ", cfg.ttc_strong_brakes());
    log_print("  sched priority = ", cfg.sched_priority());
    log_print("  sched policy   = ", cfg.sched_policy());
    log_print("  sched deadline runtime = ", cfg.sched_deadline_runtime(), " us");

    // Set real-time scheduling (optional, may require privileges)
    // Scheduler Policies: 
    // Deadline: SCHED_DEADLINE (6) - highest priority, requires additional attributes
    // Realtime: SCHED_FIFO (1), SCHED_RR (2) - static priority scheduling
    // Fair: SCHED_OTHER/SCHED_NORMAL (0) - default time-sharing
    
    const int policy = cfg.sched_policy();
    
    if (policy == 6) {  // SCHED_DEADLINE
        // For SCHED_DEADLINE, we need to use sched_setattr with deadline attributes
        #ifdef __NR_sched_setattr
        struct sched_attr {
            uint32_t size;
            uint32_t sched_policy;
            uint64_t sched_flags;
            int32_t  sched_nice;
            uint32_t sched_priority;
            uint64_t sched_runtime;
            uint64_t sched_deadline;
            uint64_t sched_period;
        } attr;
        
        memset(&attr, 0, sizeof(attr));
        attr.size = sizeof(attr);
        attr.sched_policy = 6;  // SCHED_DEADLINE
        attr.sched_flags = 0;
        // For SCHED_DEADLINE: runtime <= deadline <= period
        attr.sched_runtime  = cfg.sched_deadline_runtime() * 1000ULL;  // convert µs to ns
        attr.sched_deadline = cfg.sched_deadline_runtime() * 1000ULL;  // same as runtime
        attr.sched_period   = cfg.sched_deadline_runtime() * 1000ULL;  // same as runtime
        
        if (syscall(__NR_sched_setattr, 0, &attr, 0) == -1) {
            std::cerr << "Failed to set SCHED_DEADLINE: " 
                      << strerror(errno) << std::endl;
            std::cerr << "Note: SCHED_DEADLINE requires CAP_SYS_NICE capability or root privileges" << std::endl;
        } else {
            log_print("Apply EABS C++: SCHED_DEADLINE set successfully (runtime=", 
                      cfg.sched_deadline_runtime(), "µs)");
        }
        #else
        std::cerr << "SCHED_DEADLINE not supported on this system" << std::endl;
        #endif
    } else {
        // For other schedulers, use standard sched_setscheduler
        struct sched_param param;
        param.sched_priority = cfg.sched_priority();
        
        if (sched_setscheduler(0, policy, &param) == -1) {
            std::cerr << "Failed to set scheduler and/or priority: " 
                      << strerror(errno) << std::endl;
            std::cerr << "Note: Real-time scheduling requires CAP_SYS_NICE capability or root privileges" << std::endl;
        } else {
            const char* policy_name = 
                policy == 0 ? "SCHED_OTHER" :
                policy == 1 ? "SCHED_FIFO" :
                policy == 2 ? "SCHED_RR" : "UNKNOWN";
            log_print("Apply EABS C++: Scheduler set to ", policy_name, 
                      " with priority ", cfg.sched_priority());
        }
    }



    ExternalController controller(cfg.carla_host(), cfg.carla_port(), cfg.role_name(), cfg.sync_mode());

    try {
      controller.Run();
    } catch (...) {
      controller.Cleanup();
      throw;
    }

    controller.Cleanup();
    return 0;

  } catch (const cc::TimeoutException &e) {
    std::cerr << "TimeoutException: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 2;
  }
}
