#pragma once

#include "device.hpp"
#include "protocol.hpp"
#include "blockingqueue.hpp"

namespace DvStick {

    class Channel;

    class QueueWorker;

    class Device: public CodecServer::Device {
        public:
            Device(std::string tty, unsigned int baudRate);
            std::vector<std::string> getCodecs() override;
            CodecServer::Session* startSession(CodecServer::Request* request) override;
            void writePacket(DvStick::Protocol::Packet* packet);
            void receivePacket(DvStick::Protocol::Packet* packet);
            // TODO privatize
            int fd;
        private:
            void open(std::string tty, unsigned int baudRate);
            void init();
            std::vector<Channel*> channels;
            BlockingQueue<DvStick::Protocol::Packet*>* queue;
            QueueWorker* worker;
    };

    class Channel {
        public:
            Channel(Device* device, unsigned char index);
            void process(char* input, size_t size);
            void receive(DvStick::Protocol::SpeechPacket* speech);
            size_t read(char* output);
            unsigned char getIndex();
            bool isBusy();
            void reserve();
            void release();
        private:
            bool busy = false;
            Device* device;
            unsigned char index;
            BlockingQueue<DvStick::Protocol::SpeechPacket*>* queue;
    };

    class QueueWorker {
        public:
            QueueWorker(Device* device, BlockingQueue<DvStick::Protocol::Packet*>* queue);
        private:
            void run();
            Device* device;
            BlockingQueue<DvStick::Protocol::Packet*>* queue;
            bool dorun = true;
    };

}
