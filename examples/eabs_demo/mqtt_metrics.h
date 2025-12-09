// mqtt_metrics.h

#pragma once

#include <mosquitto.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

// MQTT message
struct MetricMessage {
  std::string topic;
  std::string payload;
  int qos     = 0;
  bool retain = false;
};

// RAII init/cleanup for libmosquitto (once per process)
struct MosquittoLibGuard {
  MosquittoLibGuard()  { mosquitto_lib_init();  }
  ~MosquittoLibGuard() { mosquitto_lib_cleanup(); }
};

std::int64_t NowNanoseconds() {
  using clock = std::chrono::system_clock;
  auto now = clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
}

class MqttMetrics {
public:
  MqttMetrics(std::string host,
              int         port,
              std::string client_id,
              std::size_t max_queue_size = 1024,
              int         keepalive      = 60)
    : _host(std::move(host)),
      _port(port),
      _client_id(std::move(client_id)),
      _max_queue(max_queue_size),
      _keepalive(keepalive),
      _running(false),
      _dropped(0) {
  }

  ~MqttMetrics() {
    Stop();
  }

  // Start background worker thread
  void Start() {
    bool expected = false;
    if (!_running.compare_exchange_strong(expected, true)) {
      return; // already running
    }
    _worker = std::thread(&MqttMetrics::WorkerLoop, this);
  }

  // Stop worker, flush any remaining messages, join thread
  void Stop() {
    bool expected = true;
    if (!_running.compare_exchange_strong(expected, false)) {
      return; // already stopped
    }
    {
      std::lock_guard<std::mutex> lock(_mtx);
      _cv.notify_all();
    }
    if (_worker.joinable()) {
      _worker.join();
    }
  }

  // Non-blocking enqueue; drops messages if queue is full
  void Enqueue(MetricMessage msg) {
    if (!_running.load(std::memory_order_relaxed)) {
      return;
    }
    {
      std::lock_guard<std::mutex> lock(_mtx);
      if (_queue.size() >= _max_queue) {
        ++_dropped;
        return;
      }
      _queue.emplace_back(std::move(msg));
    }
    _cv.notify_one();
  }

  // Mark a timestamped event with value
  void SendTimestamp(const std::string &tag, std::int64_t value) {
    auto ns = NowNanoseconds();

    // Simple JSON: {"ts":123..., "event":"...", "value":value}
    std::string payload = std::string("{\"ts\":")
                          + std::to_string(ns)
                          + ",\"event\":\"" + tag + "\""
                          + ",\"value\":" + std::to_string(value)
                          + "}";

    MetricMessage msg;
    msg.topic   = "eabs/nxp/events";
    msg.payload = std::move(payload);
    msg.qos     = 0;
    msg.retain  = false;
    Enqueue(std::move(msg));
  }

  std::uint64_t DroppedCount() const {
    return _dropped.load(std::memory_order_relaxed);
  }

private:
  void WorkerLoop() {
    static MosquittoLibGuard s_lib_guard;  // ensures libmosquitto init once

    mosquitto *mosq = mosquitto_new(_client_id.c_str(), true, nullptr);
    if (!mosq) {
      std::cerr << "MqttMetrics: mosquitto_new failed\n";
      return;
    }

    bool connected = false;

    auto try_connect = [&]() {
      int rc = mosquitto_connect(mosq, _host.c_str(), _port, _keepalive);
      if (rc != MOSQ_ERR_SUCCESS) {
        std::cerr << "MqttMetrics: connect failed: " << mosquitto_strerror(rc)
                  << " (host=" << _host << ", port=" << _port << ")\n";
        return false;
      }
      connected = true;
      return true;
    };

    // Initial connection attempt (with basic retry/backoff)
    while (_running.load() && !connected) {
      if (try_connect()) {
        break;
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Main loop
    while (_running.load() || !QueueEmpty()) {
      MetricMessage msg;
      bool has_msg = false;

      {
        std::unique_lock<std::mutex> lock(_mtx);

        // Wake at least every 100ms to call mosquitto_loop
        _cv.wait_for(lock, std::chrono::milliseconds(100), [&] {
          return !_queue.empty() || !_running.load();
        });

        if (!_queue.empty()) {
          msg = std::move(_queue.front());
          _queue.pop_front();
          has_msg = true;
        }
      }

      if (connected && has_msg) {
        int rc = mosquitto_publish(
          mosq,
          /*mid*/ nullptr,
          msg.topic.c_str(),
          static_cast<int>(msg.payload.size()),
          msg.payload.data(),
          msg.qos,
          msg.retain
        );

        if (rc != MOSQ_ERR_SUCCESS) {
          std::cerr << "MqttMetrics: publish error: "
                    << mosquitto_strerror(rc) << "\n";
          // treat as disconnect, we’ll reconnect below
          connected = false;
        }
      }

      // Handle keepalive, pings, etc.
      if (connected) {
        int rc = mosquitto_loop(mosq, /*timeout_ms*/ 0, /*max_packets*/ 1);
        if (rc != MOSQ_ERR_SUCCESS) {
          std::cerr << "MqttMetrics: loop error: "
                    << mosquitto_strerror(rc) << "\n";
          connected = false;
        }
      }

      // Reconnect if needed and still running or there is data to flush
      if (!connected && (_running.load() || !QueueEmpty())) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        try_connect();
      }
    }

    if (connected) {
      mosquitto_disconnect(mosq);
    }
    mosquitto_destroy(mosq);
  }

  bool QueueEmpty() {
    std::lock_guard<std::mutex> lock(_mtx);
    return _queue.empty();
  }

  // Config
  std::string _host;
  int         _port;
  std::string _client_id;
  std::size_t _max_queue;
  int         _keepalive;

  // State
  std::atomic<bool>        _running;
  std::thread              _worker;
  std::deque<MetricMessage> _queue;
  std::mutex               _mtx;
  std::condition_variable  _cv;
  std::atomic<std::uint64_t> _dropped;
};