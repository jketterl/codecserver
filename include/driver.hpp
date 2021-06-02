#pragma once

#include "device.hpp"
#include <string>
#include <vector>
#include <map>

namespace CodecServer {

    class Driver {
        public:
            virtual std::string getIdentifier() = 0;
            // default implementation is a NOOP
            virtual std::vector<Device*> scanForDevices() { return {}; };
            virtual Device* buildFromConfiguration(std::map<std::string, std::string> config) = 0;
            virtual void configure(std::map<std::string, std::string> config) {};
    };

}