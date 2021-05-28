#include "config.hpp"
#include <iostream>
#include <set>

using namespace CodecServer;

Config::Config(std::string path) {
    std::ifstream input(path);
    if (input.fail()) {
        throw ConfigException("error opening config file");
    }
    read(input);
}

void Config::read(std::ifstream& input) {
    std::string line;
    std::set<char> commentChars({';', '#'});

    std::map<std::string, std::string> current_map;
    std::string current_name;
    while (getline(input, line)) {
        // ignore empty lines
        if (line.length() == 0) {
            continue;
        }

        // ignore comments
        if (commentChars.find(line.at(0)) != commentChars.end()) {
            continue;
        }

        if (line.at(0) == '[' && line.at(line.length() -1) == ']') {
            if (current_name != "") {
                sections[current_name] = current_map;
            }
            current_name = line.substr(1, line.length() - 2);
            current_map = std::map<std::string, std::string>();
            continue;
        }

        size_t pos = line.find("=");
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            current_map[key] = value;
            continue;
        }

        std::cerr << "invalid line in config: " << line << "\n";
    }

    if (current_name != "") {
        sections[current_name] = current_map;
    }
}