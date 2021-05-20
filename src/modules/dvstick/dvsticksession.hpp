#pragma once

#include "session.hpp"

namespace CodecServer {

    class DvStickSession: public Session {
        public:
            virtual char* decode(char* data);
            virtual char* encode(char* data);
    };

}