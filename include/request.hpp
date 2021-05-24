#pragma once

#include <string>
#include <map>

#define DIRECTION_DECODE 1
#define DIRECTION_ENCODE 2

namespace CodecServer {

    class Request {
        public:
            Request(unsigned char dir, std::map<std::string, std::string> args);
            Request(unsigned char dir);
            unsigned char getDirection();
            std::map<std::string, std::string> getArgs();
            std::string getArg(std::string name);
        private:
            unsigned char dir;
            std::map<std::string, std::string> args;
    };

}