#include "ambe3kdriver.hpp"
#include "registry.hpp"
#include "udevmonitor.hpp"
#include <iostream>

using namespace Ambe3K;

Driver::Driver() {
    new Ambe3K::Udev::Monitor();
}

std::string Driver::getIdentifier(){
    return "ambe3k";
}

Device* Driver::buildFromConfiguration(std::map<std::string, std::string> config){
    if (config.find("tty") == config.end()) {
        std::cerr << "unable to create ambe3k device: tty not specified\n";
        return nullptr;
    }
    if (config.find("baudrate") == config.end()) {
        std::cerr << "unable to create ambe3k device: baudrade not specified\n";
        return nullptr;
    }
    unsigned int baudrate;
    try {
        baudrate = stoul(config["baudrate"]);
    } catch (std::invalid_argument) {
        std::cerr << "unable to create ambe3k device: cannot parse baudrate\n";
    }
    return new Device(config["tty"], baudrate);
}


static int registration = CodecServer::Registry::registerDriver(new Driver());