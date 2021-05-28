#pragma once

#include "driver.hpp"
#include "dvstickdevice.hpp"

namespace DvStick {

    class Driver: public CodecServer::Driver {
        public:
            std::string getIdentifier() override;
            Device* buildFromConfiguration(std::map<std::string, std::string> config) override;
    };

}