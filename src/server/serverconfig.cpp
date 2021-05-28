#include "serverconfig.hpp"

using namespace CodecServer;

std::vector<std::string> ServerConfig::getDevices() {
    return getSections("device");
}

std::map<std::string, std::string> ServerConfig::getDeviceConfig(std::string key) {
    return getSection("device:" + key);
}