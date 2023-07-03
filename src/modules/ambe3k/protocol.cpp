#include "protocol.hpp"
#include <cassert>
#include <iostream>

using namespace Ambe3K::Protocol;

Field::Field(char *data): data(data) {}

ChecksumField::ChecksumField(char *data): Field(data) {
    data[0] = AMBE3K_PARITY_BYTE;
}

size_t ChecksumField::getLength() {
    return 2;
}

void ChecksumField::setChecksum(char checksum) {
    data[1] = checksum;
}

bool ChecksumField::isCorrect(char checksum) {
    return data[1] == checksum;
}

ReadyField::ReadyField(char *data): Field(data) {
    data[0] = AMBE3K_CONTROL_READY;
}

size_t ReadyField::getLength() {
    return 1;
}

ResetField::ResetField(char *data): Field(data) {
    data[0] = AMBE3K_CONTROL_RESET;
}

size_t ResetField::getLength() {
    return 1;
}

ProdIdRequestField::ProdIdRequestField(char *data): Field(data) {
    data[0] = AMBE3K_CONTROL_PRODID;
}

size_t ProdIdRequestField::getLength() {
    return 1;
}

ProdIdResponseField::ProdIdResponseField(char *data):
    Field(data),
    // this is scary. we're relying on a \0 in data to stop us from going beyond its end...
    productId(data + 1)
{}

size_t ProdIdResponseField::getLength() {
    return productId.length() + 2;
}

std::string ProdIdResponseField::getProductId() {
    return productId;
}

VersionStringRequestField::VersionStringRequestField(char *data): Field(data) {
    data[0] = AMBE3K_CONTROL_VERSTRING;
}

size_t VersionStringRequestField::getLength() {
    return 1;
}

VersionStringResponseField::VersionStringResponseField(char *data): Field(data) {}

size_t VersionStringResponseField::getLength() {
    return 49;
}

std::string VersionStringResponseField::getVersionId() {
    return { data + 1, 48 };
}

ChannelField::ChannelField(char* data): Field(data) {}

ChannelField::ChannelField(char *data, unsigned char channel): ChannelField(data) {
    assert(channel <= 3);
    data[0] = 0x40 + channel;
}

size_t ChannelField::getLength() {
    return 1;
}

unsigned char ChannelField::getChannel() {
    return data[0] - 0x40;
}

ChannelResponseField::ChannelResponseField(char *data): Field(data) {
    assert(data[1] == 0);
}

size_t ChannelResponseField::getLength() {
    return 2;
}

unsigned char ChannelResponseField::getChannel() {
    return data[0] - 0x40;
}

RateTRequestField::RateTRequestField(char *data, unsigned char index): Field(data) {
    data[0] = AMBE3K_CONTROL_RATET;
    data[1] = index;
}

size_t RateTRequestField::getLength() {
    return 2;
}

RateTResponseField::RateTResponseField(char *data): Field(data) {
    assert(data[1] == 0);
}

size_t RateTResponseField::getLength() {
    return 2;
}

RatePRequestField::RatePRequestField(char *data, short *cwds): Field(data) {
    data[0] = AMBE3K_CONTROL_RATEP;
    auto output = (short*)(data + 1);
    for (int i = 0; i < 6; i++) {
        output[i] = htons(cwds[i]);
    }
}

size_t RatePRequestField::getLength() {
    return 13;
}

RatePResponseField::RatePResponseField(char *data): Field(data) {
    assert(data[1] == 0);
}

size_t RatePResponseField::getLength() {
    return 2;
}

InitRequestField::InitRequestField(char *data, unsigned char direction): Field(data) {
    data[0] = AMBE3K_CONTROL_INIT;
    data[1] = direction;
}

size_t InitRequestField::getLength() {
    return 2;
}

InitResponseField::InitResponseField(char *data): Field(data) {
    assert(data[1] == 0);
}

size_t InitResponseField::getLength() {
    return 2;
}

ChanDField::ChanDField(char *data, char *channelData, unsigned char bits): Field(data), bits(bits) {
    data[0] = AMBE3K_CHANNEL_CHAND;
    data[1] = bits;
    memcpy(data + 2, channelData, (bits + 7) / 8);
}

ChanDField::ChanDField(char *data): Field(data), bits((unsigned char) data[1]) {}

size_t ChanDField::getLength() {
    return 2 + (bits + 7) / 8;
}

size_t ChanDField::getChannelData(char *output) {
    size_t size = (bits + 7) / 8;
    memcpy(output, data + 2, size);
    return size;
}

SpeechDField::SpeechDField(char *data, char *speechData, unsigned char samples): Field(data), samples(samples) {
    data[0] = AMBE3K_SPEECH_SPEECHD;
    data[1] = samples;
    // TODO: this probably needs htons()
    memcpy(data + 2, speechData, samples * 2);
}

SpeechDField::SpeechDField(char *data): Field(data), samples((unsigned char) data[1]) {}

size_t SpeechDField::getLength() {
    return 2 + samples * 2;
}

size_t SpeechDField::getSpeechData(short *output) {
    auto src = (short*) (data + 2);
    for (int i = 0; i < samples; i++) {
        output[i] = ntohs(src[i]);
    }
    return samples * 2;
}

Packet::Packet(char* newData, size_t size): data(newData), dataSize(size) {
    // minimum size without parity
    assert(size >= 4);
    if (size > 4) {
        payload = data + 4;
    } else {
        // packet has no data
        payload = nullptr;
    }
}

Packet::Packet(size_t size): Packet((char*) calloc(size, 1), size) {
    // start byte is constant
    data[0] = AMBE3K_START_BYTE;

    // need to fix endianness
    *(short*)(data + 1) = htons(dataSize - 4);
}

Packet::Packet(size_t size, char type): Packet(size) {
    data[3] = type;
}

Packet::~Packet() {
    free(data);
    delete checksum;
}

Packet* Packet::parse(char* data, size_t size) {
    Packet* p;
    char type = data[3];
    if (type == AMBE3K_TYPE_CONTROL) {
        p = new ControlPacket(data, size);
    } else if (type == AMBE3K_TYPE_AUDIO) {
        p = new SpeechPacket(data, size);
    } else if (type == AMBE3K_TYPE_AMBE) {
        p = new ChannelPacket(data, size);
    } else {
        std::cerr << "Warning: unexpected packet (type = " << type << ")\n";
        return nullptr;
    }
    p->scanFields();
    if (!p->isChecksumValid()) {
        std::cerr << "Warning: packet had wrong checksum\n";
        delete p;
        return nullptr;
    }
    return p;
}

void Packet::scanFields() {
    char* current = payload;
    size_t remaining = dataSize - 4;
    while (remaining > 0) {
        Field* f = buildField(current);
        if (f == nullptr) {
            std::cerr << "Error: packet contains an unexpected field (opcode = " << + *current << "; dataSize = " << dataSize << "; remaining = " << remaining << ")\n";
            // that's a break because we can't continue
            break;
        }
        size_t l = f->getLength();
        if (l > remaining) {
            std::cerr << "Error: field length exceeds packet size (we need " << l << "; we got " << remaining << ")\n";
            delete f;
            break;
        }
        current += l;
        remaining -= l;
        if (dynamic_cast<ChecksumField*>(f) && remaining > 0) {
            std::cerr << "Warning: checksum field is not at packet end as expected (remaining: " << remaining << ")\n";
        }
        storeField(f);
    }
}

Field* Packet::buildField(char *current) {
    switch (*current) {
        case AMBE3K_PARITY_BYTE:
            return new ChecksumField(current);
        case 0x40:
        case 0x41:
        case 0x42:
            return new ChannelField(current);
    }
    return nullptr;
}

void Packet::storeField(Field* field) {
    if (auto cf = dynamic_cast<ChecksumField*>(field)) {
        checksum = cf;
    } else if (auto cr = dynamic_cast<ChannelField*>(field)) {
        channel = cr;
    } else {
        std::cerr << "Error: unexpected field in storeField()\n";
    }
}

char Packet::getChecksum() {
    char parity = 0;
    for (int i = 0; i < dataSize - 2; i++) {
        parity ^= data[i + 1];
    }
    return parity;
}

void Packet::updateChecksum() {
    if (checksum == nullptr) return;
    checksum->setChecksum(getChecksum());
}

bool Packet::hasChecksum() {
    return checksum != nullptr;
}

bool Packet::isChecksumValid() {
    // packets without checksum shall pass without checking
    if (!hasChecksum()) return true;
    return checksum->isCorrect(getChecksum());
}

bool Packet::hasChannel() {
    return channel != nullptr;
}

unsigned char Packet::getChannel() {
    if (hasChannel()) return channel->getChannel();
    return 0;
}

void Packet::writeTo(int fd) {
    updateChecksum();
    write(fd, data, dataSize);
}

Packet* Packet::receiveFrom(int fd) {
    char* buf = (char*) malloc(1024);
    short remain = 4;
    for (int i = 0; i < 10; i++) {
        remain -= read(fd, buf + (4 - remain), remain);
        if (remain == 0) {
            break;
        }
    }

    if (remain > 0 || buf[0] != AMBE3K_START_BYTE) {
        return nullptr;
    }

    short payloadLength = ntohs(*((short*) &buf[1]));
    remain = payloadLength;
    buf = (char*) realloc(buf, remain + 4);

    for (int i = 0; i < 10; i++) {
        remain -= read(fd, buf + (4 + payloadLength - remain), remain);
        if (remain == 0) {
            break;
        }
    }

    return parse(buf, payloadLength + 4);
}

size_t Packet::getPayloadLength() {
    return dataSize - 4 - hasChecksum() * 2;
}

ControlPacket::ControlPacket(size_t bytes): Packet(bytes, AMBE3K_TYPE_CONTROL) {}

ControlPacket::~ControlPacket() {
    delete ready;
    delete prodid;
    delete version;
    delete channelResponse;
    delete ratePResponse;
    delete rateTResponse;
    delete initResponse;
}

Field* ControlPacket::buildField(char *current) {
    switch (*current) {
        case AMBE3K_CONTROL_READY:
            return new ReadyField(current);
        case AMBE3K_CONTROL_PRODID:
            return new ProdIdResponseField(current);
        case AMBE3K_CONTROL_VERSTRING:
            return new VersionStringResponseField(current);
        case AMBE3K_CONTROL_RATET:
            return new RateTResponseField(current);
        case AMBE3K_CONTROL_RATEP:
            return new RatePResponseField(current);
        case AMBE3K_CONTROL_INIT:
            return new InitResponseField(current);
        case 0x40:
        case 0x41:
        case 0x42:
            return new ChannelResponseField(current);
        default:
            return Packet::buildField(current);
    }
}

void ControlPacket::storeField(Field* field) {
    if (auto rf = dynamic_cast<ReadyField*>(field)) {
        ready = rf;
    } else if (auto pr = dynamic_cast<ProdIdResponseField*>(field)) {
        prodid = pr;
    } else if (auto vr = dynamic_cast<VersionStringResponseField*>(field)) {
        version = vr;
    } else if (auto rpr = dynamic_cast<RatePResponseField*>(field)) {
        ratePResponse = rpr;
    } else if (auto rtr = dynamic_cast<RateTResponseField*>(field)) {
        rateTResponse = rtr;
    } else if (auto ir = dynamic_cast<InitResponseField*>(field)) {
        initResponse = ir;
    } else if (auto cr = dynamic_cast<ChannelResponseField*>(field)) {
        channelResponse = cr;
    } else {
        Packet::storeField(field);
    }
}

bool ControlPacket::hasReadyField() {
    return ready != nullptr;
}

bool ControlPacket::hasProductId() {
    return prodid != nullptr;
}

std::string ControlPacket::getProductId() {
    return prodid->getProductId();
}

bool ControlPacket::hasVersionString() {
    return version != nullptr;
}

std::string ControlPacket::getVersionString() {
    return version->getVersionId();
}

bool ControlPacket::hasRatePResponse() {
    return ratePResponse != nullptr;
}

bool ControlPacket::hasRateTResponse() {
    return rateTResponse != nullptr;
}

bool ControlPacket::hasInitResponse() {
    return initResponse != nullptr;
}

bool ControlPacket::hasChannel() {
    return channelResponse != nullptr;
}

unsigned char ControlPacket::getChannel() {
    return channelResponse->getChannel();
}

ResetPacket::ResetPacket(): ControlPacket(7) {
    size_t offset = 0;
    reset = new ResetField(payload + offset);
    offset += reset->getLength();
    checksum = new ChecksumField(payload + offset);
}

ResetPacket::~ResetPacket() {
    delete reset;
}

ProdIdRequest::ProdIdRequest(): ControlPacket(7) {
    size_t offset = 0;
    request = new ProdIdRequestField(payload + offset);
    offset += request->getLength();
    checksum = new ChecksumField(payload + offset);
}

ProdIdRequest::~ProdIdRequest() {
    delete request;
}

VersionStringRequest::VersionStringRequest(): ControlPacket(7) {
    size_t offset = 0;
    request = new VersionStringRequestField(payload + offset);
    offset += request->getLength();
    checksum = new ChecksumField(payload + offset);
}

VersionStringRequest::~VersionStringRequest() {
    delete request;
}

SetupRequest::SetupRequest(unsigned char channel, unsigned char index, unsigned char direction): ControlPacket(11) {
    assert(channel <= 3);
    size_t offset = 0;
    this->channel = new ChannelField(payload + offset, channel);
    offset += this->channel->getLength();
    request = new RateTRequestField(payload + offset, index);
    offset += request->getLength();
    init = new InitRequestField(payload + offset, direction);
    offset += init->getLength();
    checksum = new ChecksumField(payload + offset);
}

SetupRequest::SetupRequest(unsigned char channel, short* cwds, unsigned char direction): ControlPacket(22) {
    assert(channel <= 3);
    size_t offset = 0;
    this->channel = new ChannelField(payload + offset, channel);
    offset += this->channel->getLength();
    request = new RatePRequestField(payload + offset, cwds);
    offset += request->getLength();
    init = new InitRequestField(payload + offset, direction);
    offset += init->getLength();
    checksum = new ChecksumField(payload + offset);
}

SetupRequest::~SetupRequest() {
    delete channel;
    delete request;
    delete init;
}

ChannelPacket::ChannelPacket(unsigned char channel, char* channelData, unsigned char bits):
    Packet((int) ((bits + 7) / 8) + 9, AMBE3K_TYPE_AMBE)
{
    size_t offset = 0;
    this->channel = new ChannelField(payload + offset, channel);
    offset += this->channel->getLength();
    chanD = new ChanDField(payload + offset, channelData, bits);
    offset += chanD->getLength();
    checksum = new ChecksumField(payload + offset);
}

ChannelPacket::~ChannelPacket() {
    delete channel;
    delete chanD;
}

Field* ChannelPacket::buildField(char *current) {
    switch (*current) {
        case AMBE3K_CHANNEL_CHAND:
            return new ChanDField(current);
        default:
            return Packet::buildField(current);
    }
}

void ChannelPacket::storeField(Field* field) {
    if (auto cd = dynamic_cast<ChanDField*>(field)) {
        chanD = cd;
    } else {
        Packet::storeField(field);
    }
}

SpeechPacket::SpeechPacket(unsigned char channel, char* speechData, unsigned char samples):
    Packet(samples * 2 + 9, AMBE3K_TYPE_AUDIO)
{
    size_t offset = 0;
    this->channel = new ChannelField(payload + offset, channel);
    offset += this->channel->getLength();
    speechD = new SpeechDField(payload + offset, speechData, samples);
    offset += speechD->getLength();
    checksum = new ChecksumField(payload + offset);
}

SpeechPacket::~SpeechPacket() {
    delete channel;
    delete speechD;
}

Field *SpeechPacket::buildField(char* current) {
    switch (*current) {
        case AMBE3K_SPEECH_SPEECHD:
            return new SpeechDField(current);
        default:
            return Packet::buildField(current);
    }
}

void SpeechPacket::storeField(Field *field) {
    if (auto sd = dynamic_cast<SpeechDField*>(field)) {
        speechD = sd;
    } else {
        Packet::storeField(field);
    }
}

size_t SpeechPacket::getSpeechData(short* output) {
    if (speechD == nullptr) return 0;
    return speechD->getSpeechData(output);
}

size_t ChannelPacket::getChannelData(char* output) {
    if (chanD == nullptr) return 0;
    return chanD->getChannelData(output);
}