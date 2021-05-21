#pragma once

#include <string>
#include <unistd.h>
#include <stdlib.h>

#define DV3K_START_BYTE 0x61
#define DV3K_PARITY_BYTE 0x2f

#define DV3K_TYPE_CONTROL 0x00
#define DV3K_TYPE_AMBE 0x01
#define DV3K_TYPE_AUDIO 0x02

#define DV3K_CONTROL_RATEP 0x0A
#define DV3K_CONTROL_PRODID 0x30
#define DV3K_CONTROL_VERSTRING 0x31
#define DV3K_CONTROL_RESET 0x33
#define DV3K_CONTROL_READY 0x39
#define DV3K_CONTROL_CHANFMT 0x15

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
            ControlPacket(size_t bytes): Packet(bytes) { setType(DV3K_TYPE_CONTROL); }
            ControlPacket(char* payload, size_t bytes): Packet(payload, bytes) {}
    };

    class ResetPacket: public ControlPacket {
        public:
            ResetPacket(): ControlPacket(7) { payload[0] = DV3K_CONTROL_RESET; }
    };

    class ReadyPacket: public ControlPacket {
        public:
            ReadyPacket(char* payload, size_t bytes): ControlPacket(payload, bytes) {}
    };

    class ProdIdRequest: public ControlPacket {
        public:
            ProdIdRequest(): ControlPacket(7) { payload[0] = DV3K_CONTROL_PRODID; }
    };

    class ProdIdResponse: public ControlPacket {
        public:
            ProdIdResponse(char* payload, size_t bytes): ControlPacket(payload, bytes) {}
            std::string getProductId();
    };

    class VersionStringRequest: public ControlPacket {
        public:
            VersionStringRequest(): ControlPacket(7) { payload[0] = DV3K_CONTROL_VERSTRING; }
    };

    class VersionStringResponse: public ControlPacket {
        public:
            VersionStringResponse(char* payload, size_t bytes): ControlPacket(payload, bytes) {}
            std::string getVersionString();
    };

}