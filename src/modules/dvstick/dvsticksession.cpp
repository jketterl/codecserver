#include "dvsticksession.hpp"
#include <cstring>
#include <iostream>

using namespace DvStick;

DvStickSession::DvStickSession() {
    stick = new DvStick("/dev/ttyUSB1", 921600);
}

size_t DvStickSession::process(char* input, char* output, size_t size) {
    std::memcpy(output, input, size);
    return size;
}
