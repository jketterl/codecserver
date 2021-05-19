#include "registry.hpp"
#include <iostream>

using namespace CodecServer;

int Registry::registerCodec(std::string identifier, Codec* codec) {
    std::cout << "codec registered: " << identifier << "\n";
    return 0;
};