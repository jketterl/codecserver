#pragma once

#include <unistd.h>

namespace CodecServer {

    class Session {
        public:
            virtual void start() {};
            virtual size_t process(char* input, char* output, size_t size) = 0;
            virtual void end() {};
    };

}