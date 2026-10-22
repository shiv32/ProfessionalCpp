
#include "packet.h"

#include <iostream>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

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
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);

    if (serverFd < 0)
    {
        std::cerr << "socket failed\n";
        return 1;
    }

    int reuse = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR,
               &reuse, sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(serverFd, reinterpret_cast<sockaddr *>(&address),
             sizeof(address)) < 0 ||
        listen(serverFd, 5) < 0)
    {
        std::cerr << "bind/listen failed: "
                  << std::strerror(errno) << '\n';
        close(serverFd);
        return 1;
    }

    std::cout << "UAV simulator listening on port 8080\n";

    int clientFd = accept(serverFd, nullptr, nullptr);

    if (clientFd < 0)
    {
        std::cerr << "accept failed\n";
        close(serverFd);
        return 1;
    }

    bool armed = false;
    bool flying = false;
    uint16_t altitudeCm = 0;
    uint16_t battery = 100;

    std::cout << "GCS connected\n";

    Packet packet{};

    while (receiveAll(clientFd, packet.data(), packet.size()))
    {
        if (!validPacket(packet) || packet[3] != COMMAND)
        {
            std::cerr << "Invalid packet\n";
            break;
        }

        const uint16_t seq = readU16(packet, 4);
        const uint8_t cmd = packet[6];

        std::cout << "Received command " << int(cmd)
                  << ", sequence " << seq << '\n';

        switch (cmd)
        {
        case ARM:
            if (!flying)
                armed = true;
            break;

        case TAKEOFF:
            if (armed)
            {
                flying = true;
                altitudeCm = 1000;
                if (battery > 0)
                    --battery;
            }
            break;

        case LAND:
            altitudeCm = 0;
            flying = false;
            armed = false;
            break;

        case STATUS:
            break;

        default:
            std::cerr << "Unknown command\n";
            continue;
        }

        Packet telemetry = makePacket(
            TELEMETRY, seq, 0,
            armed, flying, altitudeCm, battery);

        if (!sendAll(clientFd, telemetry.data(), telemetry.size()))
        {
            std::cerr << "send failed\n";
            break;
        }
    }

    close(clientFd);
    close(serverFd);
    return 0;
}
