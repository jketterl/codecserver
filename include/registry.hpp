#pragma once

#include <string>
#include "codec.hpp"

namespace CodecServer {

    class Registry {
        public:
            static int registerCodec(std::string identifier, Codec* codec);
    };

}