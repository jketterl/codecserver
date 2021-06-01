#pragma once

#include <unistd.h>
#include "proto/framing.pb.h"
#include "proto/request.pb.h"

using namespace CodecServer::proto;

namespace CodecServer {

    class Session {
        public:
            virtual void start() {};
            virtual void decode(char* input, size_t size) = 0;
            virtual void encode(char* input, size_t size) = 0;
            virtual size_t read(char* output) = 0;
            virtual void end() {};
            virtual FramingHint* getFraming() { return nullptr; }
            virtual void renegotiate(Settings settings) {};
    };

}