#include "scanner.hpp"
#include <iostream>
#include <glob.h>
#include <dlfcn.h>

using namespace CodecServer;

void Scanner::scanModules() {
    std::cout << "now scanning for modules...\n";
    for (std::string lib : getLibs()) {
        loadLibrary(lib);
    }
}

std::vector<std::string> Scanner::getSearchPaths() {
    std::vector<std::string> searchPaths;
    searchPaths.push_back(MODULES_PATH);
    return searchPaths;
}

std::vector<std::string> Scanner::getLibs() {
    std::vector<std::string> libs;
    for (std::string searchPath : getSearchPaths()) {
        glob_t globResults;
        std::string pattern = searchPath + "/*.so";
        int ret = glob(pattern.c_str(), 0, NULL, &globResults);
        if (ret == 0) {
            for (size_t i = 0; i < globResults.gl_pathc; i++) {
                libs.push_back(globResults.gl_pathv[i]);
            }
        } else if (ret == GLOB_NOMATCH) {
            // pass
        } else {
            std::cerr << "glob() failed with ret=" << ret << " for pattern " << pattern << "\n";
        }

        globfree(&globResults);
    }
    return libs;
}

void Scanner::loadLibrary(std::string path) {
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (handle == NULL) {
        std::cerr << "dlopen() failed: " << dlerror() << "\n";
    }
}