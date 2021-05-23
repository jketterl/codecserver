#pragma once

#include "connection.hpp"

namespace CodecServer {

    class Cli {
        public:
            int main(int argc, char** argv);
        private:
            Connection* connection;
            bool run = true;
    };

}