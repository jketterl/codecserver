#include "registry.hpp"
#include <iostream>

using namespace CodecServer;

int Registry::registerDevice(Device* device) {
    return Registry::get()->_registerDevice(device);
}

int Registry::_registerDevice(Device* device) {
    std::cout << "registering new device for codecs: ";

    for (std::string codec: device->getCodecs()) {
        std::cout << codec << ", ";
        devices[codec].push_back(device);
    }
    std::cout << "\n";

    return 0;
}

std::vector<Device*> Registry::findDevices(std::string identifier) {
    return devices[identifier];
}

Registry* Registry::get() {
    if (sharedRegistry == nullptr) {
        sharedRegistry = new Registry();
    }
    return sharedRegistry;
}