#pragma once

#include <map>
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
        private:
            void read(std::ifstream& input);
            std::map<std::string, std::map<std::string, std::string>> sections;
    };

}