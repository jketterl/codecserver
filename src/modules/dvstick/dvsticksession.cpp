#include "dvsticksession.hpp"
#include <cstring>
#include <iostream>

using namespace DvStick;

DvStickSession::DvStickSession(Device* stick, unsigned char channel) {
    this->stick = stick;
    this->channel = channel;
}

size_t DvStickSession::process(char* input, char* output, size_t size) {
    return stick->decode(input, output, size);
}

void DvStickSession::end() {
    stick->releaseChannel(channel);
}
