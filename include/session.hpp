#pragma once

namespace CodecServer {

    class Session {
        public:
            virtual char* process(char* data) = 0;
    };

}