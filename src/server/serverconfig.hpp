#pragma once

#include "config.hpp"
#include <string>
#include <vector>
#include <map>

namespace CodecServer {

    class ServerConfig: public Config {
        public:
            ServerConfig(std::string path): Config(path) {};
            std::vector<std::string> getDevices();
            std::map<std::string, std::string> getDeviceConfig(std::string key);
    };

}