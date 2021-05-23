#pragma once

#include "request.hpp"
#include "session.hpp"
#include <vector>
#include <string>

namespace CodecServer {
    class Device {
        public:
            virtual std::vector<std::string> getCodecs() = 0;
            virtual Session* startSession(Request* request) = 0;
    };
}