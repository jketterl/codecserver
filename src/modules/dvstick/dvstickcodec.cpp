#include "dvstickcodec.hpp"
#include "registry.hpp"
#include "dvsticksession.hpp"

using namespace DvStick;

CodecServer::Session* DvStickCodec::startSession(CodecServer::Request* request) {
    return new DvStickSession();
}

static int registration = CodecServer::Registry::registerCodec("ambe", new DvStickCodec());