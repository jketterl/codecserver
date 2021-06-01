#pragma once

#include "request.pb.h"
#include "session.hpp"
#include <vector>
#include <string>

using namespace CodecServer::proto;

namespace CodecServer {
    class DeviceException: public std::runtime_error {
        public:
            DeviceException(std::string message);
            DeviceException(std::string message, int err);
    };

    class Device {
        public:
            virtual std::vector<std::string> getCodecs() = 0;
            virtual Session* startSession(Request* request) = 0;
    };
}