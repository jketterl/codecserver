#pragma once

#include "session.hpp"
#include "dvstickdevice.hpp"
#include <unistd.h>

namespace DvStick {

    class DvStickSession: public CodecServer::Session {
        public:
            DvStickSession(Device* stick);
            virtual size_t process(char* input, char* output, size_t size);
        private:
            Device* stick;
    };

}