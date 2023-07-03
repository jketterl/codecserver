#include "ambe3kregistry.hpp"

using namespace Ambe3K;

Registry* Registry::get() {
    if (sharedInstance == nullptr) {
        sharedInstance = new Registry();
    }
    return sharedInstance;
}

void Registry::addDevice(const std::string& node, Registration* device) {
    devices[node] = device;
}

bool Registry::hasDevice(const std::string& node) {
    return findByNode(node) != nullptr;
}

Registration* Registry::findByNode(const std::string& node) {
    auto it = devices.find(node);
    if (it != devices.end()) {
        return it->second;
    }
    return nullptr;
}