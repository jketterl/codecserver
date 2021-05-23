#pragma once

#include "device.hpp"

namespace DvStick {

    class DvStickDevice : public CodecServer::Device {
        public:
            std::vector<std::string> getCodecs();
            CodecServer::Session* startSession(CodecServer::Request* request) override;
    };

}
