#pragma once

#include <map>
#include <string>
#include <thread>

namespace CodecServer {

    class SocketServer {
        public:
            SocketServer(std::map<std::string, std::string> config) {}
            virtual ~SocketServer();
            virtual void start();
            virtual void stop();
            virtual void run();
            virtual void join();
        protected:
            int sock;
            bool dorun = true;
        private:
            std::thread thread;
    };

}