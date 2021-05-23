#pragma once

#include "device.hpp"

namespace DvStick {

    class Device: public CodecServer::Device {
        public:
            Device(std::string tty, unsigned int baudRate);
            std::vector<std::string> getCodecs() override;
            CodecServer::Session* startSession(CodecServer::Request* request) override;
            size_t decode(char* input, char* output, size_t size);
            void releaseChannel(unsigned char channel);
        private:
            void open(std::string tty, unsigned int baudRate);
            void init();
            int fd;
            std::vector<bool> channelState;
    };

}
