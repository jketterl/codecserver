#pragma once

namespace CodecServer {

    class Server {
        public:
            int main(int argc, char** argv);
            void serve();
        private:
            int sock;
            bool run = true;
    };

}