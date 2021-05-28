#include "registry.hpp"
#include <iostream>

using namespace CodecServer;

int Registry::registerDriver(Driver* driver) {
    return Registry::get()->_registerDriver(driver);
}

int Registry::_registerDriver(Driver* driver) {
    std::cout << "registering new driver: " << driver->getIdentifier() << "\n";
    drivers[driver->getIdentifier()] = driver;

    std::cout << "scanning for \"" << driver->getIdentifier() << "\" devices...\n";
    for (Device* device: driver->scanForDevices()) {
        registerDevice(device);
    }
    std::cout << "device scan complete.\n";

    return 0;
}

void Registry::loadDeviceFromConfig(std::map<std::string, std::string> config) {
    if (config.find("driver") == config.end()) {
        std::cerr << "unable to load device: driver not specified\n";
        return;
    }
    if (drivers.find(config["driver"]) == drivers.end()) {
        std::cerr << "unable to load device: driver \"" << config["driver"] << "\" not available\n";
        return;
    }
    Driver* driver = drivers[config["driver"]];
    Device* device = driver->buildFromConfiguration(config);
    if (device != nullptr) {
        registerDevice(device);
    }
}

void Registry::registerDevice(Device* device) {
    std::cout << "registering new device for codecs: ";

    for (std::string codec: device->getCodecs()) {
        std::cout << codec << ", ";
        devices[codec].push_back(device);
    }
    std::cout << "\n";
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