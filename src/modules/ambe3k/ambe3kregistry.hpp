#pragma once

#include "ambe3kdevice.hpp"

#include <map>

namespace Ambe3K {

    struct Registration {
        std::map<std::string, std::string> config;
        Device* device = nullptr;
    };

    class Registry {
        public:
            static Registry* get();
            void addDevice(const std::string& node, Registration* device);
            bool hasDevice(const std::string& node);
            Registration* findByNode(const std::string& node);
        private:
            std::map<std::string, Registration*> devices;
    };

    static Registry* sharedInstance = nullptr;

}