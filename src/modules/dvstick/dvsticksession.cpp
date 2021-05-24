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
    switch (channel->getCodecIndex()) {
        case 33:
            CodecServer::proto::FramingHint* framing = new CodecServer::proto::FramingHint();
            framing->set_channelbits(72);
            framing->set_channelbytes(9);
            framing->set_audiosamples(160);
            framing->set_audiobytes(320);
            return framing;
    }
    return nullptr;
}
