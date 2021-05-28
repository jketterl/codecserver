#pragma once

#include <string>

namespace CodecServer {

    class Server {
        public:
            int main(int argc, char** argv);
            void handle_signal(int signal);
            void serve();
        private:
            bool parseOptions(int argc, char** argv);
            void printUsage();
            void printVersion();
            void stop();
            std::string configFile = CONFIG;
            int sock;
            bool run = true;
    };

}