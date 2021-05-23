#pragma once

#include "driver.hpp"
#include "dvstickdevice.hpp"

namespace DvStick {

    class Driver: public CodecServer::Driver {
        public:
            std::string getIdentifier();
            std::vector<CodecServer::Device*> scanForDevices();
            // TODO config typing
            Device* buildFromConfiguration(void* config);
    };

}