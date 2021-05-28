#pragma once

#include <map>
#include <string>

namespace CodecServer {

    class Config {
        public:
            Config(std::string path);
        private:
            void read(int fd);
            std::map<std::string, std::map<std::string, std::string>> sections;
    };

}