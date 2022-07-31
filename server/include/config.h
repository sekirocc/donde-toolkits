#pragma once

#include "nlohmann/json.hpp"

#include <fstream>
#include <iostream>
#include <string>

using json = nlohmann::json;

class Config {
  public:
    inline std::string get_log_level() { return _log_level; };
    inline std::string get_device_id() { return _device_id; };
    inline std::string get_pipeline_config() { return _pipeline_config; };

    static void init(const std::string path);
    static Config getInstance();

  private:
    Config() {
        assert(_config_filepath.size() != 0);
        // load json from filepath;
    }
    static std::string _config_filepath;
    std::string _log_level;
    std::string _device_id;
    std::string _pipeline_config;
};
