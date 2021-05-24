#pragma once

#include "session.hpp"
#include "dvstickdevice.hpp"
#include "framing.pb.h"
#include <unistd.h>

namespace DvStick {

    class DvStickSession: public CodecServer::Session {
        public:
            DvStickSession(Channel* channel);
            void process(char* input, size_t size) override;
            size_t read(char* output) override;
            void end() override;
            CodecServer::proto::FramingHint* getFraming() override;
        private:
            Channel* channel;
    };

}