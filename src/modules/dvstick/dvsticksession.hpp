#pragma once

#include "session.hpp"
#include <unistd.h>

namespace CodecServer {

    class DvStickSession: public Session {
        public:
            virtual size_t process(char* input, char* output, size_t size);
    };

}