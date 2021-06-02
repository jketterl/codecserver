#pragma once

#include "device.hpp"
#include "driver.hpp"
#include <string>
#include <vector>

namespace CodecServer {

    class Registry {
        public:
            static Registry* get();
            static int registerDriver(Driver* driver);
            void configureDriver(std::string driver, std::map<std::string, std::string> config);
            void loadDeviceFromConfig(std::map<std::string, std::string> config);
            void autoDetectDevices();
            std::vector<Device*> findDevices(std::string identifier);
            void unregisterDevice(Device* device);
        private:
            std::map<std::string, Driver*> drivers;
            std::map<std::string, std::vector<Device*>> devices;
            int _registerDriver(Driver* driver);
            void registerDevice(Device* device);
    };

    static Registry* sharedRegistry = nullptr;
}