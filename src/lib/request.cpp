#include "request.hpp"

using namespace CodecServer;

Request::Request(unsigned char dir, std::map<std::string, std::string> args) {
    this->dir = dir;
    this->args = args;
}

Request::Request(unsigned char dir) {
    Request(dir, {});
}

unsigned char Request::getDirection() {
    return dir;
}

std::map<std::string, std::string> Request::getArgs() {
    return args;
}

std::string Request::getArg(std::string argName) {
    return args[argName];
}