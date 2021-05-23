#include "dvstickdriver.hpp"
#include "registry.hpp"

using namespace DvStick;

std::string Driver::getIdentifier(){
    return "dv3k";
}

std::vector<CodecServer::Device*> Driver::scanForDevices() {
    return { new Device("/dev/ttyUSB0", 921600) };
}

// TODO config typing
Device* Driver::buildFromConfiguration(void* config){
    return nullptr;
}


static int registration = CodecServer::Registry::registerDriver(new Driver());