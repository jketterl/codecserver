#include "request.hpp"

using namespace CodecServer;

Request::Request(Direction dir, std::map<std::string, std::string> args) {
}

Request::Request(Direction dir) {
    Request(dir, std::map<std::string, std::string>());
}