#pragma once

#include "ambe3kdevice.hpp"
#include "blockingqueue.hpp"
#include "protocol.hpp"

namespace Ambe3K {

    // forward declaration since those two classes are interdependent
    class Device;

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

}