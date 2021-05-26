#include "dvsticksession.hpp"
#include <cstring>
#include <iostream>

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
    std::map<std::string, std::string> args(settings.args().begin(), settings.args().end());
    std::string indexStr = args["index"];
    std::cout << "renegotiating: index: " << indexStr << "; direction: ";
    unsigned char index = std::stoi(indexStr);
    unsigned char direction = 0;
    for (int direction: settings.directions()) {
        if (direction == Settings_Direction_ENCODE) {
            direction |= DV3K_DIRECTION_ENCODE;
            std::cout << "enccode ";
        } else if (direction == Settings_Direction_DECODE) {
            direction |= DV3K_DIRECTION_DECODE;
            std::cout << "decode ";
        }
    }
    std::cout << "\n";
    channel->setup(index, direction);
}
