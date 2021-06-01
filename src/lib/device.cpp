#include "device.hpp"

using namespace CodecServer;

DeviceException::DeviceException(std::string message): std::runtime_error(message) {}

DeviceException::DeviceException(std::string message, int err): DeviceException(message + ": " + strerror(err)) {}