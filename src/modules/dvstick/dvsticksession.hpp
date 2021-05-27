#pragma once

#include "session.hpp"
#include "dvstickdevice.hpp"
#include "framing.pb.h"
#include "request.pb.h"
#include <unistd.h>

using namespace CodecServer::proto;

namespace DvStick {

    class DvStickSession: public CodecServer::Session {
        public:
            DvStickSession(Channel* channel);
            void process(char* input, size_t size) override;
            size_t read(char* output) override;
            void end() override;
            FramingHint* getFraming() override;
            void renegotiate(Settings settings) override;
        private:
            Channel* channel;
            short* parseRatePString(std::string input);
    };

}