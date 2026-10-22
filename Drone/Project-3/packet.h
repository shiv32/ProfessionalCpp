
#pragma once

#include <array>
#include <cstdint>

constexpr std::size_t PACKET_SIZE = 16;
constexpr uint8_t VERSION = 1;

enum PacketType : uint8_t
{
    COMMAND = 1,
    TELEMETRY = 2
};

enum CommandId : uint8_t
{
    ARM = 1,
    TAKEOFF = 2,
    LAND = 3,
    STATUS = 4
};

using Packet = std::array<uint8_t, PACKET_SIZE>;

inline void writeU16(Packet &p, int pos, uint16_t value)
{
    p[pos] = static_cast<uint8_t>(value >> 8);
    p[pos + 1] = static_cast<uint8_t>(value & 0xFF);
}

inline uint16_t readU16(const Packet &p, int pos)
{
    return (uint16_t(p[pos]) << 8) | p[pos + 1];
}

inline uint16_t checksum16(const Packet &p)
{
    uint32_t sum = 0;

    for (int i = 0; i < 14; ++i)
        sum += p[i];

    return static_cast<uint16_t>(sum & 0xFFFF);
}

inline Packet makePacket(
    uint8_t type,
    uint16_t sequence,
    uint8_t command = 0,
    bool armed = false,
    bool flying = false,
    uint16_t altitudeCm = 0,
    uint16_t battery = 100)
{
    Packet p{};

    p[0] = 0x55;
    p[1] = 0x56;
    p[2] = VERSION;
    p[3] = type;

    writeU16(p, 4, sequence);

    p[6] = command;
    p[7] = armed ? 1 : 0;
    p[8] = flying ? 1 : 0;

    writeU16(p, 9, altitudeCm);
    writeU16(p, 11, battery);

    p[13] = 0; // Reserved
    writeU16(p, 14, checksum16(p));

    return p;
}

inline bool validPacket(const Packet &p)
{
    return p[0] == 0x55 &&
           p[1] == 0x56 &&
           p[2] == VERSION &&
           readU16(p, 14) == checksum16(p);
}
