#pragma once

#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <cassert>
#include <arpa/inet.h> // htons

#define AMBE3K_START_BYTE 0x61
#define AMBE3K_PARITY_BYTE 0x2f

#define AMBE3K_TYPE_CONTROL 0x00
#define AMBE3K_TYPE_AMBE 0x01
#define AMBE3K_TYPE_AUDIO 0x02

#define AMBE3K_CONTROL_RATEP 0x0A
#define AMBE3K_CONTROL_RATET 0x09
#define AMBE3K_CONTROL_INIT 0x0B
#define AMBE3K_CONTROL_PRODID 0x30
#define AMBE3K_CONTROL_VERSTRING 0x31
#define AMBE3K_CONTROL_RESET 0x33
#define AMBE3K_CONTROL_READY 0x39
#define AMBE3K_CONTROL_CHANFMT 0x15
#define AMBE3K_CONTROL_SPCHFMT 0x16
#define AMBE3K_CONTROL_COMPAND 0x32

#define AMBE3K_DIRECTION_ENCODE 0x01
#define AMBE3K_DIRECTION_DECODE 0x02

#define AMBE3K_FIFO_MAX_PENDING 3

namespace DvStick::Protocol {

    class Packet {
        public:
            static Packet* parse(char* data, size_t bytes);
            Packet(char* payload, size_t bytes);
            Packet(size_t bytes): Packet((char*) calloc(bytes, 1), bytes) {};
            char verifyChecksum();
            void writeTo(int fd);
            static Packet* receiveFrom(int fd);
            // necessary to maintain polymorphism
            virtual ~Packet() {}
        protected:
            char getType();
            void setType(char type);
            void updateChecksum();
            size_t getPayloadLength();
            char* payload;
        private:
            char getChecksum();
            char* data;
            size_t dataSize;
    };

    class ControlPacket: public Packet {
        public:
            ControlPacket(size_t bytes);
            ControlPacket(char* payload, size_t bytes): Packet(payload, bytes) {}
    };

    class ResetPacket: public ControlPacket {
        public:
            ResetPacket();
    };

    class ReadyPacket: public ControlPacket {
        public:
            ReadyPacket(char* payload, size_t bytes): ControlPacket(payload, bytes) {}
    };

    class ProdIdRequest: public ControlPacket {
        public:
            ProdIdRequest();
    };

    class ProdIdResponse: public ControlPacket {
        public:
            ProdIdResponse(char* payload, size_t bytes): ControlPacket(payload, bytes) {}
            std::string getProductId();
    };

    class VersionStringRequest: public ControlPacket {
        public:
            VersionStringRequest();
    };

    class VersionStringResponse: public ControlPacket {
        public:
            VersionStringResponse(char* payload, size_t bytes): ControlPacket(payload, bytes) {}
            std::string getVersionString();
    };

    // DMR rate 3600/2450: rate index 33
    class SetupRequest: public ControlPacket{
        public:
            SetupRequest(unsigned char channel, unsigned char index, unsigned char direction);
            SetupRequest(unsigned char channel, short* cwds, unsigned char direction);
    };

    class RateTResponse: public ControlPacket {
        public:
            RateTResponse(char* payload, size_t size): ControlPacket(payload, size) {}
            unsigned char getChannel();
            char getResult();
    };

    class RatePResponse: public ControlPacket {
        public:
            RatePResponse(char* payload, size_t size): ControlPacket(payload, size) {}
            unsigned char getChannel();
            char getResult();
    };

    class ChannelPacket: public Packet {
        public:
            ChannelPacket(char* data, size_t size): Packet(data, size) {}
            ChannelPacket(unsigned char channel, char* channelData, unsigned char bits);
            size_t getChannelData(char* output);
            unsigned char getChannel();
    };

    class SpeechPacket: public Packet {
        public:
            SpeechPacket(char* data, size_t size): Packet(data, size) {}
            SpeechPacket(unsigned char channel, char* speechData, unsigned char samples);
            size_t getSpeechData(char* output);
            unsigned char getChannel();
    };


}