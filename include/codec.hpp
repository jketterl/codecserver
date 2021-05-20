#pragma once

#include "request.hpp"
#include "session.hpp"

namespace CodecServer {
    class Codec {
        public:
            virtual Session* startSession(Request* request) = 0;
    };
}