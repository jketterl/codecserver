#pragma once

namespace CodecServer {

    class Session {
        public:
            virtual char* decode(char* data) = 0;
            virtual char* encode(char* data) = 0;
    };

}