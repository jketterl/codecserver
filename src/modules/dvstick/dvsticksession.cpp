#include "dvsticksession.hpp"
#include <cstring>
#include <iostream>

using namespace DvStick;

DvStickSession::DvStickSession(Channel* channel) {
    this->channel = channel;
}

size_t DvStickSession::process(char* input, char* output, size_t size) {
    return channel->decode(input, output, size);
}

void DvStickSession::end() {
    channel->release();
}
