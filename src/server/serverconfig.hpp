#pragma once

#include "config.hpp"
#include <string>
#include <vector>
#include <map>

namespace CodecServer {

    class ServerConfig: public Config {
        public:
            explicit ServerConfig(std::string path): Config(path) {};
            std::vector<std::string> getDevices();
            std::map<std::string, std::string> getDeviceConfig(const std::string& key);
            std::vector<std::string> getDrivers();
            std::map<std::string, std::string> getDriverConfig(const std::string& key);
    };

}