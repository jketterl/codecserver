#include "dvstickcodec.hpp"
#include "registry.hpp"
#include "dvsticksession.hpp"

using namespace DvStick;

std::vector<std::string> DvStickDevice::getCodecs() {
    return { "ambe" };
}

CodecServer::Session* DvStickDevice::startSession(CodecServer::Request* request) {
    return new DvStickSession();
}

static int registration = CodecServer::Registry::registerDevice(new DvStickDevice());