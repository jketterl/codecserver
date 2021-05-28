#pragma once

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

namespace CodecServer {

    class ConfigException: public std::runtime_error {
        public:
            ConfigException(const char* msg): std::runtime_error(msg) {}
    };

    class Config {
        public:
            Config(std::string path);
            std::vector<std::string> getServers();
            std::map<std::string, std::string> getServerConfig(std::string key);
        protected:
            void read(std::ifstream& input);
            std::map<std::string, std::map<std::string, std::string>> sections;
            std::vector<std::string> getSections(std::string type);
            std::map<std::string, std::string> getSection(std::string name);
    };

}