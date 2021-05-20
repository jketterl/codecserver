#pragma once

namespace CodecServer {

    class Server {
        public:
            int main(int argc, char** argv);
            void handle_signal(int signal);
            void serve();
        private:
            void stop();
            int sock;
            bool run = true;
    };

}