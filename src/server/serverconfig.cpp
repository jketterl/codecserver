#include "serverconfig.hpp"

using namespace CodecServer;

std::vector<std::string> ServerConfig::getDevices() {
    return getSections("device");
}

std::map<std::string, std::string> ServerConfig::getDeviceConfig(const std::string& key) {
    return getSection("device:" + key);
}

std::vector<std::string> ServerConfig::getDrivers() {
    return getSections("driver");
}

std::map<std::string, std::string> ServerConfig::getDriverConfig(const std::string& key) {
    return getSection("driver:" + key);
}