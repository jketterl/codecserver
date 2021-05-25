#pragma once

#include "request.pb.h"
#include "session.hpp"
#include <vector>
#include <string>

using namespace CodecServer::proto;

namespace CodecServer {
    class Device {
        public:
            virtual std::vector<std::string> getCodecs() = 0;
            virtual Session* startSession(Request* request) = 0;
    };
}