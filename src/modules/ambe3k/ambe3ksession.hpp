#pragma once

#include "session.hpp"
#include "ambe3kdevice.hpp"
#include "framing.pb.h"
#include "request.pb.h"
#include <unistd.h>

using namespace CodecServer::proto;

namespace Ambe3K {

    class Ambe3KSession: public CodecServer::Session {
        public:
            Ambe3KSession(Channel* channel);
            void encode(char* input, size_t size) override;
            void decode(char* input, size_t size) override;
            size_t read(char* output) override;
            void end() override;
            FramingHint* getFraming() override;
            void renegotiate(Settings settings) override;
        private:
            Channel* channel;
            short* parseRatePString(std::string input);
    };

}