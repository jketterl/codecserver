#pragma once

#include "device.hpp"
#include "driver.hpp"
#include "ambe3kdevice.hpp"

namespace Ambe3K {

    class Driver: public CodecServer::Driver {
        public:
            explicit Driver();
            std::string getIdentifier() override;
            Device* buildFromConfiguration(std::map<std::string, std::string> config) override;
    };

}