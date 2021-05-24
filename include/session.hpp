#pragma once

#include <unistd.h>

namespace CodecServer {

    class Session {
        public:
            virtual void start() {};
            virtual void process(char* input, size_t size) = 0;
            virtual size_t read(char* output) = 0;
            virtual void end() {};
    };

}