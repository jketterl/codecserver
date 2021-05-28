#pragma once

#include <unistd.h>
#include "framing.pb.h"
#include "request.pb.h"

using namespace CodecServer::proto;

namespace CodecServer {

    class Session {
        public:
            virtual void start() {};
            virtual void process(char* input, size_t size) = 0;
            virtual size_t read(char* output) = 0;
            virtual void end() {};
            virtual FramingHint* getFraming() { return nullptr; }
            virtual void renegotiate(Settings settings) {};
    };

}