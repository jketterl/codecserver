#include "dvstick.hpp"
#include "registry.hpp"
#include "dvsticksession.hpp"

using namespace CodecServer;

Session* DvStick::startSession(Request* request) {
    return new DvStickSession();
}

static int registration = Registry::registerCodec("ambe", new DvStick());