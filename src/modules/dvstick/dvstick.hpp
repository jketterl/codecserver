#pragma once

#include "codec.hpp"

namespace CodecServer {

    class DvStick : public Codec {
        public:
            virtual Session* startSession(Request* request) override;
    };

}
