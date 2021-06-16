#include "channel.hpp"

using namespace Ambe3K;
using namespace Ambe3K::Protocol;

Channel::Channel(Device* device, unsigned char index) {
    this->device = device;
    this->index = index;
}

Channel::~Channel() {
    release();
}

unsigned char Channel::getFramingBits() {
    switch (getCodecIndex()) {
        case 33:
            return 72;
        case 34:
            return 49;
        case 59:
            return 144;
    }
    short* cwds = getRateP();
    if (cwds != nullptr) {
        short dstar[] = {0x0130, 0x0763, 0x4000, 0x0000, 0x0000, 0x0048};
        if (std::memcmp(cwds, &dstar, sizeof(short) * 6) == 0) {
            return 72;
        }
    }
    return 0;
}

void Channel::decode(char* input, size_t size) {
    // need to make a copy since we do queue the data, and `input` doesn't live long enough
    char* data = (char*) malloc(size);
    memcpy(data, input, size);
    device->writePacket(new ChannelPacket(index, data, size * 8));
}

void Channel::encode(char* input, size_t size) {
    // need to make a copy since we do queue the data, and `input` doesn't live long enough
    char* data = (char*) malloc(size);
    memcpy(data, input, size);
    device->writePacket(new SpeechPacket(index, data, size / 2));
}

void Channel::receive(SpeechPacket* packet) {
    // TODO lock to make sure that queue doesn't go away after...
    if (outQueue == nullptr) {
        delete packet;
        std::cerr << "received packet while channel is inactive. recent shutdown?\n";
        return;
    }
    try {
        outQueue->push(packet);
    } catch (QueueFullException) {
        std::cerr << "channel queue full. shutting down queue...\n";
        delete packet;
        delete outQueue;
        outQueue = nullptr;
    }
}

void Channel::receive(ChannelPacket* packet) {
    // TODO lock to make sure that queue doesn't go away after...
    if (outQueue == nullptr) {
        delete packet;
        std::cerr << "received packet while channel is inactive. recent shutdown?\n";
        return;
    }
    try {
        outQueue->push(packet);
    } catch (QueueFullException) {
        std::cerr << "channel queue full. shutting down queue...\n";
        delete packet;
        delete outQueue;
        outQueue = nullptr;
    }
}

size_t Channel::read(char* output) {
    while (true) {
        Packet* packet = outQueue->pop();
        if (packet == nullptr) {
            return 0;
        }

        // TODO: this loses the typing. callers of read() will not know if the response is channel or speech data.

        SpeechPacket* speech = dynamic_cast<SpeechPacket*>(packet);
        if (speech != nullptr) {
            size_t size = speech->getSpeechData(output);
            delete speech;
            return size;
        }

        ChannelPacket* channel = dynamic_cast<ChannelPacket*>(packet);
        if (channel != nullptr) {
            size_t size = channel->getChannelData(output);
            delete channel;
            return size;
        }

        std::cerr << "dropping one unexpected packet from channel queue\n";
        delete packet;
    }
}

unsigned char Channel::getIndex() {
    return index;
}

bool Channel::isBusy() {
    return busy;
}

void Channel::reserve() {
    busy = true;
    outQueue = new BlockingQueue<Packet*>(10);
}

void Channel::release() {
    if (outQueue != nullptr) {
        delete outQueue;
        outQueue=nullptr;
    }
    busy = false;
}

void Channel::setup(unsigned char codecIndex, unsigned char direction) {
    this->codecIndex = codecIndex;
    if (ratep != nullptr) delete(ratep);
    ratep = nullptr;
    device->writePacket(new SetupRequest(index, codecIndex, direction));
}

void Channel::setup(short* cwds, unsigned char direction) {
    codecIndex = 0;
    if (ratep != nullptr && ratep != cwds) delete(ratep);
    ratep = cwds;
    device->writePacket(new SetupRequest(index, cwds, direction));
}

unsigned char Channel::getCodecIndex() {
    return codecIndex;
}

short* Channel::getRateP() {
    return ratep;
}