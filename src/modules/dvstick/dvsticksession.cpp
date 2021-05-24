#include "dvsticksession.hpp"
#include <cstring>
#include <iostream>

using namespace DvStick;

DvStickSession::DvStickSession(Channel* channel) {
    this->channel = channel;
}

void DvStickSession::process(char* input, size_t size) {
    channel->process(input, size);
}

size_t DvStickSession::read(char* output) {
    return channel->read(output);
}

void DvStickSession::end() {
    channel->release();
}
