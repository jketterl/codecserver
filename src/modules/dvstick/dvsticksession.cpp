#include "dvsticksession.hpp"
#include <cstring>

using namespace CodecServer;

size_t DvStickSession::process(char* input, char* output, size_t size) {
    std::memcpy(output, input, size);
    return size;
}
