#include "ambe3ksession.hpp"
#include <cstring>
#include <iostream>
#include <sstream>

using namespace Ambe3K;

Ambe3KSession::Ambe3KSession(Channel* channel) {
    this->channel = channel;
}

void Ambe3KSession::encode(char* input, size_t size) {
    channel->encode(input, size);
}

void Ambe3KSession::decode(char* input, size_t size) {
    channel->decode(input, size);
}

size_t Ambe3KSession::read(char* output) {
    return channel->read(output);
}

void Ambe3KSession::end() {
    channel->release();
}

CodecServer::proto::FramingHint* Ambe3KSession::getFraming() {
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

void Ambe3KSession::renegotiate(CodecServer::proto::Settings settings) {
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
        }
    } else {
        std::cout << "invalid parameters\n";
    }
}

short* Ambe3KSession::parseRatePString(const std::string& input) {
    if (input.length() != 29) return nullptr;
    std::vector<std::string> parts;
    size_t pos_start = 0, pos_end;
    while ((pos_end = input.find(':', pos_start)) != std::string::npos) {
        parts.push_back(input.substr(pos_start, pos_end - pos_start));
        pos_start = pos_end + 1;
    }
    parts.push_back(input.substr(pos_start));

    if (parts.size() != 6) return nullptr;

    auto data = (short*) malloc(sizeof(short) * 6);
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
