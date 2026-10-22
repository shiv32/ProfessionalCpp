
#include "packet.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

bool receiveAll(int fd, void *data, std::size_t size)
{
    auto *p = static_cast<uint8_t *>(data);
    std::size_t received = 0;

    while (received < size)
    {
        ssize_t n = recv(fd, p + received, size - received, 0);

        if (n <= 0)
            return false;

        received += static_cast<std::size_t>(n);
    }

    return true;
}

bool sendAll(int fd, const void *data, std::size_t size)
{
    auto *p = static_cast<const uint8_t *>(data);
    std::size_t sent = 0;

    while (sent < size)
    {
        ssize_t n = send(fd, p + sent, size - sent, 0);

        if (n <= 0)
            return false;

        sent += static_cast<std::size_t>(n);
    }

    return true;
}

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0)
    {
        std::cerr << "socket failed\n";
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);

    if (connect(fd, reinterpret_cast<sockaddr *>(&address),
                sizeof(address)) < 0)
    {
        std::cerr << "connect failed: "
                  << std::strerror(errno) << '\n';
        close(fd);
        return 1;
    }

    std::cout << "Connected to UAV simulator\n";

    uint16_t sequence = 1;

    while (true)
    {
        std::cout << "\n=== GCS MENU ===\n"
                  << "1. Arm\n"
                  << "2. Takeoff\n"
                  << "3. Land\n"
                  << "4. Request telemetry\n"
                  << "0. Exit\n"
                  << "Select: ";

        int choice;
        if (!(std::cin >> choice))
            break;

        if (choice == 0)
            break;

        uint8_t command = 0;

        switch (choice)
        {
        case 1:
            command = ARM;
            break;
        case 2:
            command = TAKEOFF;
            break;
        case 3:
            command = LAND;
            break;
        case 4:
            command = STATUS;
            break;
        default:
            std::cout << "Invalid selection\n";
            continue;
        }

        Packet packet = makePacket(COMMAND, sequence++, command);

        std::cout << "TX: ";
        for (uint8_t byte : packet)
        {
            std::cout << std::hex << std::setw(2)
                      << std::setfill('0') << int(byte) << ' ';
        }
        std::cout << std::dec << '\n';

        if (!sendAll(fd, packet.data(), packet.size()))
        {
            std::cerr << "Send failed\n";
            break;
        }

        Packet response{};

        if (!receiveAll(fd, response.data(), response.size()))
        {
            std::cerr << "Server disconnected\n";
            break;
        }

        if (!validPacket(response) ||
            response[3] != TELEMETRY)
        {
            std::cerr << "Invalid telemetry packet\n";
            break;
        }

        std::cout << "\n--- UAV TELEMETRY ---\n"
                  << "Sequence: " << readU16(response, 4) << '\n'
                  << "Armed: "
                  << (response[7] ? "YES" : "NO") << '\n'
                  << "Flying: "
                  << (response[8] ? "YES" : "NO") << '\n'
                  << "Altitude: "
                  << readU16(response, 9) / 100.0 << " m\n"
                  << "Battery: "
                  << readU16(response, 11) << "%\n";
    }

    close(fd);
    return 0;
}
