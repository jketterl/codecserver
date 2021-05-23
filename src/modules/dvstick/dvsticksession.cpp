#include "dvsticksession.hpp"
#include <cstring>
#include <iostream>

using namespace DvStick;

DvStickSession::DvStickSession(Device* stick) {
    this->stick = stick;
}

size_t DvStickSession::process(char* input, char* output, size_t size) {
    return stick->decode(input, output, size);
}
