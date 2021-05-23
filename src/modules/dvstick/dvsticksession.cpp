#include "dvsticksession.hpp"
#include <cstring>
#include <iostream>

using namespace DvStick;

DvStickSession::DvStickSession() {
    stick = new DvStick("/dev/ttyUSB0", 921600);
}

size_t DvStickSession::process(char* input, char* output, size_t size) {
    return stick->decode(input, output, size);
}
