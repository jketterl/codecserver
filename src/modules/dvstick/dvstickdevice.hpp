#pragma once

#include "device.hpp"
#include "protocol.hpp"
#include "blockingqueue.hpp"
#include "request.pb.h"
#include <termios.h>

namespace DvStick {

    class Channel;

    class QueueWorker;

    class Device: public CodecServer::Device {
        public:
            Device(std::string tty, unsigned int baudRate);
            ~Device();
            std::vector<std::string> getCodecs() override;
            CodecServer::Session* startSession(CodecServer::proto::Request* request) override;
            void writePacket(DvStick::Protocol::Packet* packet);
            void receivePacket(DvStick::Protocol::Packet* packet);
            // TODO privatize
            int fd;
        private:
            void open(std::string tty, unsigned int baudRate);
            speed_t convertBaudrate(unsigned int baudRate);
            void init();
            void createChannels(std::string prodId);
            void createChannels(unsigned int num);
            std::vector<Channel*> channels;
            BlockingQueue<DvStick::Protocol::Packet*>* queue;
            QueueWorker* worker;
    };

    class Channel {
        public:
            Channel(Device* device, unsigned char index);
            ~Channel();
            void encode(char* input, size_t size);
            void decode(char* input, size_t size);
            void receive(DvStick::Protocol::SpeechPacket* speech);
            void receive(DvStick::Protocol::ChannelPacket* channel);
            size_t read(char* output);
            unsigned char getIndex();
            bool isBusy();
            void reserve();
            void release();
            void setup(unsigned char codecIndex, unsigned char direction);
            void setup(short* cwds, unsigned char direction);
            unsigned char getCodecIndex();
            unsigned char getFramingBits();
        private:
            bool busy = false;
            Device* device;
            unsigned char index;
            unsigned char codecIndex;
            BlockingQueue<DvStick::Protocol::Packet*>* queue;
    };

    class QueueWorker {
        public:
            QueueWorker(Device* device, BlockingQueue<DvStick::Protocol::Packet*>* queue);
            ~QueueWorker();
        private:
            void run();
            Device* device;
            BlockingQueue<DvStick::Protocol::Packet*>* queue;
            bool dorun = true;
    };

}
