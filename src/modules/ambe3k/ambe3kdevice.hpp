#pragma once

#include "device.hpp"
#include "protocol.hpp"
#include "blockingqueue.hpp"
#include "proto/request.pb.h"
#include <termios.h>

namespace Ambe3K {

    class Channel;

    class QueueWorker;

    class Device: public CodecServer::Device {
        public:
            Device(std::string tty, unsigned int baudRate);
            ~Device();
            std::vector<std::string> getCodecs() override;
            CodecServer::Session* startSession(CodecServer::proto::Request* request) override;
            void writePacket(Ambe3K::Protocol::Packet* packet);
            void receivePacket(Ambe3K::Protocol::Packet* packet);
            void onQueueError(std::string message);
        private:
            void open(std::string tty, unsigned int baudRate);
            speed_t convertBaudrate(unsigned int baudRate);
            void init();
            void createChannels(std::string prodId);
            void createChannels(unsigned int num);
            int fd;
            std::vector<Channel*> channels;
            BlockingQueue<Ambe3K::Protocol::Packet*>* queue;
            QueueWorker* worker;
    };

    class Channel {
        public:
            Channel(Device* device, unsigned char index);
            ~Channel();
            void encode(char* input, size_t size);
            void decode(char* input, size_t size);
            void receive(Ambe3K::Protocol::SpeechPacket* speech);
            void receive(Ambe3K::Protocol::ChannelPacket* channel);
            size_t read(char* output);
            unsigned char getIndex();
            bool isBusy();
            void reserve();
            void release();
            void setup(unsigned char codecIndex, unsigned char direction);
            void setup(short* cwds, unsigned char direction);
            unsigned char getCodecIndex();
            short* getRateP();
            unsigned char getFramingBits();
        private:
            bool busy = false;
            Device* device;
            unsigned char index;
            unsigned char codecIndex;
            short* ratep = nullptr;
            BlockingQueue<Ambe3K::Protocol::Packet*>* queue;
    };

    class QueueWorker {
        public:
            QueueWorker(Device* device, int fd, BlockingQueue<Ambe3K::Protocol::Packet*>* queue);
            ~QueueWorker();
        private:
            void run(int fd);
            Device* device;
            BlockingQueue<Ambe3K::Protocol::Packet*>* queue;
            bool dorun = true;
    };

}
