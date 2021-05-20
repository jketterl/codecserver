#pragma once

#include "codec.hpp"
#include <string>
#include <vector>

namespace CodecServer {

    class Registry {
        public:
            static Registry* get();
            static int registerCodec(std::string identifier, Codec* codec);
            std::vector<Codec*> findCodecs(std::string identifier);
        private:
            std::map<std::string, std::vector<Codec*>> codecs;
            int _registerCodec(std::string identifier, Codec* codec);
    };

    static Registry* sharedRegistry = nullptr;
}