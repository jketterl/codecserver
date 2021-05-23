#pragma once

#include "device.hpp"

namespace DvStick {

    class Channel;

    class Device: public CodecServer::Device {
        public:
            Device(std::string tty, unsigned int baudRate);
            std::vector<std::string> getCodecs() override;
            CodecServer::Session* startSession(CodecServer::Request* request) override;
            // TODO move back to private once queue is implemented
            int fd;
        private:
            void open(std::string tty, unsigned int baudRate);
            void init();
            std::vector<Channel*> channels;
    };

    class Channel {
        public:
            Channel(Device* device, unsigned char index);
            size_t decode(char* input, char* output, size_t size);
            unsigned char getIndex();
            bool isBusy();
            void reserve();
            void release();
        private:
            bool busy = false;
            Device* device;
            unsigned char index;
    };

}
