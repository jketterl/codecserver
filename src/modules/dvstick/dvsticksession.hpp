#pragma once

#include "session.hpp"
#include "dvstickdevice.hpp"
#include <unistd.h>

namespace DvStick {

    class DvStickSession: public CodecServer::Session {
        public:
            DvStickSession(Device* stick, unsigned char channel);
            size_t process(char* input, char* output, size_t size) override;
            void end() override;
        private:
            Device* stick;
            unsigned char channel;
    };

}