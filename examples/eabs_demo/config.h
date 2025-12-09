#pragma once

#include <string>
#include <cstdint>

class Config {
public:
    // Singleton access
    static Config& GetInstance();

    // Delete copy/move constructors
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(Config&&) = delete;

    // Initialize from INI file and command-line arguments
    void Initialize(const std::string& ini_file, int argc, char* argv[]);

    // CARLA connection settings
    std::string carla_host() const { return carla_host_; }
    uint16_t carla_port() const { return carla_port_; }
    std::string role_name() const { return role_name_; }
    bool sync_mode() const { return sync_mode_; }

    // MQTT settings
    std::string mqtt_host() const { return mqtt_host_; }
    int mqtt_port() const { return mqtt_port_; }

    // Logging settings
    bool log_enabled() const { return log_enabled_; }
    bool log_to_file() const { return log_to_file_; }
    std::string log_file() const { return log_file_; }

    // EABS thresholds
    double ttc_warning() const { return ttc_warning_; }
    double ttc_mild_braking() const { return ttc_mild_braking_; }
    double ttc_strong_brakes() const { return ttc_strong_brakes_; }
    double lateral_extra_margin() const { return lateral_extra_margin_; }
    double min_v_rel() const { return min_v_rel_; }

private:
    Config();
    ~Config() = default;

    void LoadDefaults();
    void LoadFromIni(const std::string& filename);
    void ParseCommandLine(int argc, char* argv[]);

    // CARLA connection
    std::string carla_host_;
    uint16_t carla_port_;
    std::string role_name_;
    bool sync_mode_;

    // MQTT
    std::string mqtt_host_;
    int mqtt_port_;

    // Logging
    bool log_enabled_;
    bool log_to_file_;
    std::string log_file_;

    // EABS thresholds
    double ttc_warning_;
    double ttc_mild_braking_;
    double ttc_strong_brakes_;
    double lateral_extra_margin_;
    double min_v_rel_;
};
