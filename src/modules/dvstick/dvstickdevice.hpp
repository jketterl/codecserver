#pragma once

#include "device.hpp"

namespace DvStick {

    class Device: public CodecServer::Device {
        public:
            Device(std::string tty, unsigned int baudRate);
            std::vector<std::string> getCodecs();
            CodecServer::Session* startSession(CodecServer::Request* request) override;
            size_t decode(char* input, char* output, size_t size);
        private:
            void open(std::string tty, unsigned int baudRate);
            void init();
            int fd;
    };

}
