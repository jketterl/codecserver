#include "dvstickdriver.hpp"
#include "registry.hpp"

using namespace DvStick;

std::string Driver::getIdentifier(){
    return "dv3k";
}

std::vector<CodecServer::Device*> Driver::scanForDevices() {
    return { new Device("/dev/ttyUSB0", 921600) };
}

Device* Driver::buildFromConfiguration(std::map<std::string, std::string> config){
    return nullptr;
}


static int registration = CodecServer::Registry::registerDriver(new Driver());