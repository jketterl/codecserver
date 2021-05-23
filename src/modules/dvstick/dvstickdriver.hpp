#pragma once

#include "driver.hpp"
#include "dvstickdevice.hpp"

namespace DvStick {

    class Driver: public CodecServer::Driver {
        public:
            std::string getIdentifier() override;
            std::vector<CodecServer::Device*> scanForDevices() override;
            // TODO config typing
            Device* buildFromConfiguration(void* config) override;
    };

}