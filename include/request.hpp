#pragma once

#include <string>
#include <map>

namespace CodecServer {

    enum Direction { ENCODE, DECODE };

    class Request {
        public:
            Request(Direction dir, std::map<std::string, std::string> args);
            Request(Direction dir);
            std::map<std::string, std::string> getArgs();
            std::string getArg(std::string name);
    };

}