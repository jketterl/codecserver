#include "dvsticksession.hpp"
#include <cstring>
#include <iostream>
#include <sstream>

using namespace DvStick;

DvStickSession::DvStickSession(Channel* channel) {
    this->channel = channel;
}

void DvStickSession::process(char* input, size_t size) {
    channel->process(input, size);
}

size_t DvStickSession::read(char* output) {
    return channel->read(output);
}

void DvStickSession::end() {
    channel->release();
}

CodecServer::proto::FramingHint* DvStickSession::getFraming() {
    unsigned char bits = channel->getFramingBits();
    if (bits == 0) {
        return nullptr;
    }
    CodecServer::proto::FramingHint* framing;
    framing = new CodecServer::proto::FramingHint();
    framing->set_channelbits(bits);
    framing->set_channelbytes((int)((bits + 7) / 8));
    framing->set_audiosamples(160);
    framing->set_audiobytes(320);
    return framing;
}

void DvStickSession::renegotiate(CodecServer::proto::Settings settings) {
    std::cout << "renegotiating: direction:";
    std::map<std::string, std::string> args(settings.args().begin(), settings.args().end());

    unsigned char direction = 0;
    for (int dir: settings.directions()) {
        if (dir == Settings_Direction_ENCODE) {
            direction |= AMBE3K_DIRECTION_ENCODE;
            std::cout << " enccode";
        } else if (dir == Settings_Direction_DECODE) {
            direction |= AMBE3K_DIRECTION_DECODE;
            std::cout << " decode";
        }
    }

    std::cout << "; ";

    if (args.find("index") != args.end()) {
        std::string indexStr = args["index"];
        std::cout << "index: " << indexStr << "\n";
        unsigned char index = std::stoi(indexStr);
        channel->setup(index, direction);
    } else if (args.find("ratep") != args.end()) {
        std::string ratepStr = args["ratep"];
        short* rateP = parseRatePString(ratepStr);
        if (rateP == nullptr) {
            std::cout << "invalid ratep string\n";
        } else {
            std::cout << "ratep: " << ratepStr << "\n";
            channel->setup(rateP, direction);
            free(rateP);
        }
    } else {
        std::cout << "invalid parameters\n";
    }
}

short* DvStickSession::parseRatePString(std::string input) {
    if (input.length() != 29) return nullptr;
    std::vector<std::string> parts;
    size_t pos_start = 0, pos_end;
    while ((pos_end = input.find(":", pos_start)) != std::string::npos) {
        parts.push_back(input.substr(pos_start, pos_end - pos_start));
        pos_start = pos_end + 1;
    }
    parts.push_back(input.substr(pos_start));

    if (parts.size() != 6) return nullptr;

    short* data = (short*) malloc(sizeof(short) * 6);
    for (int i = 0; i < parts.size(); i++) {
        std::string part = parts[i];
        if (part.length() != 4) {
            free(data);
            return nullptr;
        }

        std::stringstream ss;
        ss << std::hex << part;
        ss >> data[i];
    }

    return data;
}
