#include "registry.hpp"
#include <iostream>

using namespace CodecServer;

int Registry::registerCodec(std::string identifier, Codec* codec) {
    return Registry::get()->_registerCodec(identifier, codec);
}

int Registry::_registerCodec(std::string identifier, Codec* codec) {
    std::cout << "codec registered: " << identifier << "\n";

    codecs[identifier].push_back(codec);

    return 0;
}

std::vector<Codec*> Registry::findCodecs(std::string identifier) {
    return codecs[identifier];
}

Registry* Registry::get() {
    if (sharedRegistry == nullptr) {
        sharedRegistry = new Registry();
    }
    return sharedRegistry;
}