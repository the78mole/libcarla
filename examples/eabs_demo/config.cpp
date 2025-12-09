#include "config.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

Config& Config::GetInstance() {
    static Config instance;
    return instance;
}

Config::Config() {
    LoadDefaults();
}

void Config::LoadDefaults() {
    // CARLA connection
    carla_host_ = "localhost";
    carla_port_ = 2000;
    role_name_ = "hero";
    sync_mode_ = false;

    // MQTT
    mqtt_host_ = "localhost";
    mqtt_port_ = 1883;

    // Logging
    log_enabled_ = false;
    log_to_file_ = true;
    log_file_ = "apply_eabs.log";

    // EABS thresholds
    ttc_warning_ = 2.5;
    ttc_mild_braking_ = 2.0;
    ttc_strong_brakes_ = 1.5;
    lateral_extra_margin_ = 0.5;
    min_v_rel_ = 0.05;
}

void Config::Initialize(const std::string& ini_file, int argc, char* argv[]) {
    LoadDefaults();
    
    if (!ini_file.empty()) {
        LoadFromIni(ini_file);
    }
    
    ParseCommandLine(argc, argv);
}

// Helper function to trim whitespace
static std::string Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

void Config::LoadFromIni(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open config file: " << filename << std::endl;
        return;
    }

    std::string line;
    std::string current_section;

    while (std::getline(file, line)) {
        line = Trim(line);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        // Parse section headers
        if (line[0] == '[' && line.back() == ']') {
            current_section = Trim(line.substr(1, line.length() - 2));
            continue;
        }

        // Parse key=value pairs
        size_t pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }

        std::string key = Trim(line.substr(0, pos));
        std::string value = Trim(line.substr(pos + 1));

        // Remove quotes from value if present
        if (value.length() >= 2 && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.length() - 2);
        }

        // Parse based on section and key
        if (current_section == "carla") {
            if (key == "host") {
                carla_host_ = value;
            } else if (key == "port") {
                carla_port_ = static_cast<uint16_t>(std::stoi(value));
            } else if (key == "role_name") {
                role_name_ = value;
            } else if (key == "sync_mode") {
                sync_mode_ = (value == "true" || value == "1" || value == "yes");
            }
        } else if (current_section == "mqtt") {
            if (key == "host") {
                mqtt_host_ = value;
            } else if (key == "port") {
                mqtt_port_ = std::stoi(value);
            }
        } else if (current_section == "logging") {
            if (key == "enabled") {
                log_enabled_ = (value == "true" || value == "1" || value == "yes");
            } else if (key == "to_file") {
                log_to_file_ = (value == "true" || value == "1" || value == "yes");
            } else if (key == "file") {
                log_file_ = value;
            }
        } else if (current_section == "eabs") {
            if (key == "ttc_warning") {
                ttc_warning_ = std::stod(value);
            } else if (key == "ttc_mild_braking") {
                ttc_mild_braking_ = std::stod(value);
            } else if (key == "ttc_strong_brakes") {
                ttc_strong_brakes_ = std::stod(value);
            } else if (key == "lateral_extra_margin") {
                lateral_extra_margin_ = std::stod(value);
            } else if (key == "min_v_rel") {
                min_v_rel_ = std::stod(value);
            }
        }
    }
}

void Config::ParseCommandLine(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if ((arg == "-h" || arg == "--host") && (i + 1 < argc)) {
            carla_host_ = argv[++i];
        } else if ((arg == "-p" || arg == "--port") && (i + 1 < argc)) {
            carla_port_ = static_cast<uint16_t>(std::stoi(argv[++i]));
        } else if (arg == "--rolename" && (i + 1 < argc)) {
            role_name_ = argv[++i];
        } else if (arg == "--sync") {
            sync_mode_ = true;
        } else if (arg == "--mqtt-host" && (i + 1 < argc)) {
            mqtt_host_ = argv[++i];
        } else if (arg == "--mqtt-port" && (i + 1 < argc)) {
            mqtt_port_ = std::stoi(argv[++i]);
        } else if (arg == "--log-enabled") {
            log_enabled_ = true;
        } else if (arg == "--log-disabled") {
            log_enabled_ = false;
        } else if (arg == "--ttc-warning" && (i + 1 < argc)) {
            ttc_warning_ = std::stod(argv[++i]);
        } else if (arg == "--ttc-mild" && (i + 1 < argc)) {
            ttc_mild_braking_ = std::stod(argv[++i]);
        } else if (arg == "--ttc-strong" && (i + 1 < argc)) {
            ttc_strong_brakes_ = std::stod(argv[++i]);
        } else if (arg == "--help") {
            std::cout
                << "Usage: eabs_demo [options]\n"
                << "Options:\n"
                << "  -h, --host HOST        CARLA server host (default: localhost)\n"
                << "  -p, --port PORT        CARLA server port (default: 2000)\n"
                << "      --rolename NAME    Ego vehicle role_name (default: hero)\n"
                << "      --sync             Synchronous mode flag\n"
                << "      --mqtt-host HOST   MQTT broker host (default: localhost)\n"
                << "      --mqtt-port PORT   MQTT broker port (default: 1883)\n"
                << "      --log-enabled      Enable logging\n"
                << "      --log-disabled     Disable logging\n"
                << "      --ttc-warning SEC  TTC warning threshold (default: 2.5)\n"
                << "      --ttc-mild SEC     TTC mild braking threshold (default: 2.0)\n"
                << "      --ttc-strong SEC   TTC strong braking threshold (default: 1.5)\n"
                << "\n"
                << "Configuration file: eabs_demo.ini (optional)\n";
            std::exit(0);
        }
    }
}
