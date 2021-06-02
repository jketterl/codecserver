#include "registry.hpp"
#include <iostream>
#include <algorithm>

using namespace CodecServer;

int Registry::registerDriver(Driver* driver) {
    return Registry::get()->_registerDriver(driver);
}

int Registry::_registerDriver(Driver* driver) {
    if (drivers.find(driver->getIdentifier()) != drivers.end()) {
        std::cerr << "failed to register driver \"" << driver->getIdentifier() << "\": already registered!\n";
        return -1;
    }

    std::cout << "registering new driver: \"" << driver->getIdentifier() << "\"\n";
    drivers[driver->getIdentifier()] = driver;


    return 0;
}

void Registry::configureDriver(std::string driver, std::map<std::string, std::string> config) {
    if (drivers.find(driver) == drivers.end()) {
        std::cerr << "cannot configure driver \"" << driver << "\": not registered";
        return;
    }

    drivers[driver]->configure(config);
}

void Registry::autoDetectDevices() {
    for (auto pair: drivers) {
        Driver* driver = pair.second;
        std::cout << "scanning for \"" << driver->getIdentifier() << "\" devices...\n";
        for (Device* device: driver->scanForDevices()) {
            registerDevice(device);
        }
    }
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
    try {
        Device* device = driver->buildFromConfiguration(config);
        if (device != nullptr) {
            registerDevice(device);
        }
    } catch (const DeviceException e) {
        std::cerr << "unable to create device: " << e.what() << "\n";
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

void Registry::unregisterDevice(Device* device) {
    std::cout << "unregistering device\n";
    for (auto entry: devices) {
        auto pos = std::find(entry.second.begin(), entry.second.end(), device);
        if (pos != entry.second.end()) {
            entry.second.erase(pos);
        }
    }
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