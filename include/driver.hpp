#pragma once

#include "device.hpp"
#include <string>
#include <vector>

namespace CodecServer {

    class Driver {
        public:
            virtual std::string getIdentifier() = 0;
            // default implementation is a NOOP
            virtual std::vector<Device*> scanForDevices() { return {}; };
            // TODO config typing
            virtual Device* buildFromConfiguration(void* config) = 0;
    };

}