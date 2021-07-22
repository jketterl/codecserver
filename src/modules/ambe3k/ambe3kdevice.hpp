#pragma once

#include "device.hpp"
#include "protocol.hpp"
#include "channel.hpp"
#include "blockingqueue.hpp"
#include "queueworker.hpp"
#include "proto/request.pb.h"
#include <termios.h>

namespace Ambe3K {

    class QueueWorker;

    // forward declaration since those two classes are interdependent
    class Channel;

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
            BlockingQueue<Ambe3K::Protocol::Packet>* queue;
            QueueWorker* worker;
    };

}
