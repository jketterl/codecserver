#pragma once

#include "device.hpp"
#include <string>
#include <vector>

namespace CodecServer {

    class Registry {
        public:
            static Registry* get();
            static int registerDevice(Device* device);
            std::vector<Device*> findDevices(std::string identifier);
        private:
            std::map<std::string, std::vector<Device*>> devices;
            int _registerDevice(Device* device);
    };

    static Registry* sharedRegistry = nullptr;
}