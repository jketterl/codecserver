#pragma once

#include "session.hpp"
#include "dvstick.hpp"
#include <unistd.h>

namespace DvStick {

    class DvStickSession: public CodecServer::Session {
        public:
            DvStickSession();
            virtual size_t process(char* input, char* output, size_t size);
        private:
            DvStick* stick;
    };

}