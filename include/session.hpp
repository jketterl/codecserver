#pragma once

#include <unistd.h>

namespace CodecServer {

    class Session {
        public:
            virtual size_t process(char* input, char* output, size_t size) = 0;
    };

}