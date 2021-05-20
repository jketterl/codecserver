#pragma once

#include "session.hpp"

namespace CodecServer {

    class DvStickSession: public Session {
        public:
            virtual char* process(char* data);
    };

}