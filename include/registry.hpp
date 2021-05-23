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
            std::vector<Device*> findDevices(std::string identifier);
        private:
            std::map<std::string, Driver*> drivers;
            std::map<std::string, std::vector<Device*>> devices;
            int _registerDriver(Driver* driver);
            void registerDevice(Device* device);
    };

    static Registry* sharedRegistry = nullptr;
}