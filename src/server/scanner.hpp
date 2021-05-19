#pragma once

#include <vector>
#include <string>

namespace CodecServer {

    class Scanner {
        public:
            void scanModules();
        private:
            std::vector<std::string> getSearchPaths();
            std::vector<std::string> getLibs();
            void loadLibrary(std::string path);
    };

}