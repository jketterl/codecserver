#pragma once

#include <string>
#include <map>

namespace CodecServer {

    class Request {
        public:
            Request(std::string codec, std::map<std::string, std::string> args);
            std::string getCodec();
            std::map<std::string, std::string> getArgs();
            std::string getArg(std::string name);
    };

}