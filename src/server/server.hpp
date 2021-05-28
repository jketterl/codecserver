#pragma once

#include <string>
#include <vector>
#include "socketserver.hpp"

namespace CodecServer {

    class Server {
        public:
            int main(int argc, char** argv);
            void handle_signal(int signal);
        private:
            bool parseOptions(int argc, char** argv);
            void printUsage();
            void printVersion();
            void stop();
            std::string configFile = CONFIG;
            std::vector<SocketServer*> servers;
    };

}