#pragma once

#include "codec.hpp"

namespace DvStick {

    class DvStickCodec : public CodecServer::Codec {
        public:
            virtual CodecServer::Session* startSession(CodecServer::Request* request) override;
    };

}
