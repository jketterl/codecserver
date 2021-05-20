#pragma once

namespace CodecServer {

    class Cli {
        public:
            int main(int argc, char** argv);
        private:
            int sock;
    };

}