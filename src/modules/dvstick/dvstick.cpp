#include "dvstick.hpp"
#include "registry.hpp"

using namespace CodecServer;

static int registration = Registry::registerCodec("ambe", new DvStick());