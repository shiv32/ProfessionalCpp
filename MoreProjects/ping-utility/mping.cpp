/*
    Usage:

    ./mping facebook.com
*/

#include <arpa/inet.h>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include <algorithm>
#include <cmath>

// ============================================================
// Global signal handling
// ============================================================

volatile std::sig_atomic_t g_running = 1;

void signalHandler(int)
{
    g_running = 0;
}

// ============================================================
// Configuration
// ============================================================

struct PingConfig
{
    std::string host;

    int timeoutMs = 1000;
    int intervalMs = 1000;
};

// ============================================================
// Ping Result
// ============================================================

struct PingResult
{
    bool sent = false;
    bool received = false;

    double rttMs = 0.0;
};

// ============================================================
// Statistics
// ============================================================

class Statistics
{
private:
    std::uint64_t sent_ = 0;
    std::uint64_t received_ = 0;

    double minRtt_ = 0.0;
    double maxRtt_ = 0.0;
    double totalRtt_ = 0.0;

    double previousRtt_ = 0.0;

    double jitterTotal_ = 0.0;
    std::uint64_t jitterSamples_ = 0;

public:

    void recordSent()
    {
        ++sent_;
    }

    void recordReceived(double rttMs)
    {
        ++received_;

        if (received_ == 1)
        {
            minRtt_ = rttMs;
            maxRtt_ = rttMs;
        }
        else
        {
            minRtt_ =
                std::min(minRtt_, rttMs);

            maxRtt_ =
                std::max(maxRtt_, rttMs);

            // Simple RTT variation measurement.
            jitterTotal_ +=
                std::abs(rttMs - previousRtt_);

            ++jitterSamples_;
        }

        totalRtt_ += rttMs;

        previousRtt_ = rttMs;
    }

    std::uint64_t sent() const
    {
        return sent_;
    }

    std::uint64_t received() const
    {
        return received_;
    }

    std::uint64_t lost() const
    {
        if (sent_ < received_)
            return 0;

        return sent_ - received_;
    }

    double lossPercent() const
    {
        if (sent_ == 0)
            return 0.0;

        return
            static_cast<double>(lost()) *
            100.0 /
            static_cast<double>(sent_);
    }

    double minRtt() const
    {
        return minRtt_;
    }

    double maxRtt() const
    {
        return maxRtt_;
    }

    double avgRtt() const
    {
        if (received_ == 0)
            return 0.0;

        return
            totalRtt_ /
            static_cast<double>(received_);
    }

    double jitter() const
    {
        if (jitterSamples_ == 0)
            return 0.0;

        return
            jitterTotal_ /
            static_cast<double>(jitterSamples_);
    }
};

// ============================================================
// RAII Socket
// ============================================================

class Socket
{
private:
    int fd_ = -1;

public:

    explicit Socket(int fd)
        : fd_(fd)
    {
    }

    ~Socket()
    {
        close();
    }

    Socket(const Socket&) = delete;

    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept
        : fd_(other.fd_)
    {
        other.fd_ = -1;
    }

    Socket& operator=(Socket&& other) noexcept
    {
        if (this != &other)
        {
            close();

            fd_ = other.fd_;

            other.fd_ = -1;
        }

        return *this;
    }

    bool valid() const
    {
        return fd_ >= 0;
    }

    int get() const
    {
        return fd_;
    }

    void close()
    {
        if (fd_ >= 0)
        {
            ::close(fd_);
            fd_ = -1;
        }
    }
};

// ============================================================
// Host Resolver Strategy
// ============================================================

class IHostResolver
{
public:

    virtual ~IHostResolver() = default;

    virtual sockaddr_in resolve(
        const std::string& host) = 0;
};

// ============================================================
// IPv4 Host Resolver
// ============================================================

class IPv4HostResolver : public IHostResolver
{
public:

    sockaddr_in resolve(
        const std::string& host) override
    {
        addrinfo hints{};

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;

        addrinfo* result = nullptr;

        int rc =
            getaddrinfo(
                host.c_str(),
                nullptr,
                &hints,
                &result);

        if (rc != 0 || result == nullptr)
        {
            throw std::runtime_error(
                "Unable to resolve host: " +
                host);
        }

        sockaddr_in address =
            *reinterpret_cast<sockaddr_in*>(
                result->ai_addr);

        freeaddrinfo(result);

        return address;
    }
};

// ============================================================
// Ping Transport Strategy
// ============================================================

class IPingTransport
{
public:

    virtual ~IPingTransport() = default;

    virtual PingResult ping(
        const sockaddr_in& destination,
        std::uint16_t sequence,
        int timeoutMs) = 0;
};

// ============================================================
// ICMP Transport
// ============================================================

class IcmpTransport : public IPingTransport
{
private:

    static constexpr std::size_t PAYLOAD_SIZE = 32;

    // --------------------------------------------------------
    // ICMP checksum
    // --------------------------------------------------------

    static std::uint16_t checksum(
        const void* data,
        std::size_t length)
    {
        const auto* bytes =
            static_cast<const std::uint8_t*>(data);

        std::uint32_t sum = 0;

        while (length > 1)
        {
            sum +=
                (static_cast<std::uint16_t>(bytes[0]) << 8) |
                static_cast<std::uint16_t>(bytes[1]);

            bytes += 2;

            length -= 2;
        }

        if (length == 1)
        {
            sum +=
                static_cast<std::uint16_t>(bytes[0])
                << 8;
        }

        while (sum >> 16)
        {
            sum =
                (sum & 0xFFFF) +
                (sum >> 16);
        }

        return static_cast<std::uint16_t>(~sum);
    }

    // --------------------------------------------------------
    // Validate ICMP reply
    // --------------------------------------------------------

    bool validReply(
        const char* buffer,
        ssize_t length,
        bool rawSocket,
        std::uint16_t expectedId,
        std::uint16_t expectedSequence) const
    {
        if (length <= 0)
            return false;

        const icmphdr* icmp = nullptr;

        // ====================================================
        // RAW socket
        //
        // Packet:
        //
        // IP header
        // ICMP header
        // payload
        // ====================================================

        if (rawSocket)
        {
            if (length <
                static_cast<ssize_t>(
                    sizeof(iphdr)))
            {
                return false;
            }

            const iphdr* ip =
                reinterpret_cast<
                    const iphdr*>(buffer);

            std::size_t ipHeaderLength =
                static_cast<std::size_t>(
                    ip->ihl) * 4;

            if (ipHeaderLength <
                sizeof(iphdr))
            {
                return false;
            }

            if (length <
                static_cast<ssize_t>(
                    ipHeaderLength +
                    sizeof(icmphdr)))
            {
                return false;
            }

            icmp =
                reinterpret_cast<
                    const icmphdr*>(
                    buffer +
                    ipHeaderLength);

            // Raw socket:
            // validate identifier.

            if (ntohs(
                    icmp->un.echo.id) !=
                expectedId)
            {
                return false;
            }
        }

        // ====================================================
        // SOCK_DGRAM ICMP
        //
        // Linux gives us ICMP packets without the normal
        // IP header.
        //
        // IMPORTANT:
        // Don't reject based on ICMP ID here.
        // ====================================================

        else
        {
            if (length <
                static_cast<ssize_t>(
                    sizeof(icmphdr)))
            {
                return false;
            }

            icmp =
                reinterpret_cast<
                    const icmphdr*>(buffer);
        }

        // ----------------------------------------------------
        // Must be Echo Reply
        // ----------------------------------------------------

        if (icmp->type != ICMP_ECHOREPLY)
            return false;

        if (icmp->code != 0)
            return false;

        // ----------------------------------------------------
        // Sequence must match
        // ----------------------------------------------------

        if (ntohs(
                icmp->un.echo.sequence) !=
            expectedSequence)
        {
            return false;
        }

        return true;
    }

public:

    PingResult ping(
        const sockaddr_in& destination,
        std::uint16_t sequence,
        int timeoutMs) override
    {
        PingResult result;

        // ====================================================
        // Try ICMP datagram socket first
        // ====================================================

        Socket socketFd(
            ::socket(
                AF_INET,
                SOCK_DGRAM,
                IPPROTO_ICMP));

        bool rawSocket = false;

        // ====================================================
        // Fallback to raw ICMP socket
        // ====================================================

        if (!socketFd.valid())
        {
            socketFd =
                Socket(
                    ::socket(
                        AF_INET,
                        SOCK_RAW,
                        IPPROTO_ICMP));

            rawSocket = true;
        }

        if (!socketFd.valid())
        {
            return result;
        }

        // ====================================================
        // Set receive timeout
        // ====================================================

        timeval timeout{};

        timeout.tv_sec =
            timeoutMs / 1000;

        timeout.tv_usec =
            (timeoutMs % 1000) * 1000;

        if (setsockopt(
                socketFd.get(),
                SOL_SOCKET,
                SO_RCVTIMEO,
                &timeout,
                sizeof(timeout)) < 0)
        {
            return result;
        }

        // ====================================================
        // Create ICMP packet
        // ====================================================

        struct Packet
        {
            icmphdr header;

            char payload[PAYLOAD_SIZE];
        };

        Packet packet{};

        packet.header.type =
            ICMP_ECHO;

        packet.header.code =
            0;

        const std::uint16_t processId =
            static_cast<std::uint16_t>(
                getpid() & 0xFFFF);

        packet.header.un.echo.id =
            htons(processId);

        packet.header.un.echo.sequence =
            htons(sequence);

        // Payload

        for (std::size_t i = 0;
             i < PAYLOAD_SIZE;
             ++i)
        {
            packet.payload[i] =
                static_cast<char>(
                    'A' + (i % 26));
        }

        packet.header.checksum = 0;

        packet.header.checksum =
            checksum(
                &packet,
                sizeof(packet));

        // ====================================================
        // Send packet
        // ====================================================

        auto start =
            std::chrono::steady_clock::now();

        ssize_t sentBytes =
            sendto(
                socketFd.get(),
                &packet,
                sizeof(packet),
                0,
                reinterpret_cast<
                    const sockaddr*>(
                    &destination),
                sizeof(destination));

        // ----------------------------------------------------
        // Actual send failed
        // ----------------------------------------------------

        if (sentBytes < 0)
        {
            result.sent = false;
            result.received = false;

            return result;
        }

        // ----------------------------------------------------
        // Packet successfully sent
        // ----------------------------------------------------

        result.sent = true;

        // ====================================================
        // Wait for reply
        // ====================================================

        char buffer[2048];

        while (true)
        {
            sockaddr_in replyAddress{};

            socklen_t replyLength =
                sizeof(replyAddress);

            ssize_t receivedBytes =
                recvfrom(
                    socketFd.get(),
                    buffer,
                    sizeof(buffer),
                    0,
                    reinterpret_cast<
                        sockaddr*>(
                        &replyAddress),
                    &replyLength);

            // ------------------------------------------------
            // Timeout
            // ------------------------------------------------

            if (receivedBytes < 0)
            {
                result.received = false;

                return result;
            }

            // ------------------------------------------------
            // Ignore unrelated ICMP packet
            // ------------------------------------------------

            if (!validReply(
                    buffer,
                    receivedBytes,
                    rawSocket,
                    processId,
                    sequence))
            {
                continue;
            }

            // =================================================
            // Valid reply
            // =================================================

            auto end =
                std::chrono::steady_clock::now();

            result.received = true;

            result.rttMs =
                std::chrono::duration<
                    double,
                    std::milli>(
                    end - start)
                    .count();

            return result;
        }
    }
};

// ============================================================
// Terminal Dashboard
// ============================================================

class TerminalDashboard
{
private:

    std::string host_;
    std::string address_;

    static constexpr int START_ROW = 7;

    void moveTo(int row)
    {
        std::cout
            << "\033["
            << row
            << ";1H";
    }

    void clearLine()
    {
        std::cout << "\033[2K";
    }

    void printLine(
        int row,
        const std::string& text)
    {
        moveTo(row);

        clearLine();

        std::cout << text;
    }

    std::string formatMs(double value) const
    {
        std::ostringstream out;

        out
            << std::fixed
            << std::setprecision(2)
            << value
            << " ms";

        return out.str();
    }

    std::string getStatus(
        const Statistics& stats,
        bool online) const
    {
        if (!online)
            return "OFFLINE";

        double loss =
            stats.lossPercent();

        double avg =
            stats.avgRtt();

        double jitter =
            stats.jitter();

        // Poor

        if (loss >= 5.0 ||
            avg >= 100.0)
        {
            return "POOR";
        }

        // Fair

        if (loss >= 1.0 ||
            avg >= 50.0 ||
            jitter >= 20.0)
        {
            return "FAIR";
        }

        // Good

        return "GOOD";
    }

public:

    TerminalDashboard(
        std::string host,
        std::string address)
        : host_(std::move(host)),
          address_(std::move(address))
    {
    }

    // --------------------------------------------------------
    // Initial dashboard
    // --------------------------------------------------------

    void initialize()
    {
        // Clear terminal once.

        std::cout
            << "\033[2J"
            << "\033[H";

        std::cout
            << "============================================\n"
            << "              INTERNET QUALITY              \n"
            << "============================================\n";

        std::cout
            << "Host       : "
            << host_
            << "\n";

        std::cout
            << "Address    : "
            << address_
            << "\n\n";

        printLine(
            START_ROW,
            "Connection : --");

        printLine(
            START_ROW + 1,
            "Last reply : --");

        printLine(
            START_ROW + 2,
            "Packets    : 0 sent / 0 received");

        printLine(
            START_ROW + 3,
            "Loss       : 0.00%");

        printLine(
            START_ROW + 5,
            "Latency    : current --");

        printLine(
            START_ROW + 6,
            "             min     --");

        printLine(
            START_ROW + 7,
            "             avg     --");

        printLine(
            START_ROW + 8,
            "             max     --");

        printLine(
            START_ROW + 10,
            "Jitter     : --");

        printLine(
            START_ROW + 12,
            "Status     : UNKNOWN");

        printLine(
            START_ROW + 14,
            "Press Ctrl+C to stop.");

        std::cout.flush();
    }

    // --------------------------------------------------------
    // Update dashboard
    // --------------------------------------------------------

    void update(
        const Statistics& stats,
        const PingResult& result)
    {
        bool online =
            result.received;

        // ====================================================
        // Connection
        // ====================================================

        printLine(
            START_ROW,
            std::string("Connection : ") +
            (online ? "ONLINE" : "OFFLINE"));

        // ====================================================
        // Last reply
        // ====================================================

        if (result.received)
        {
            printLine(
                START_ROW + 1,
                "Last reply : " +
                    formatMs(result.rttMs));
        }
        else
        {
            printLine(
                START_ROW + 1,
                "Last reply : timeout");
        }

        // ====================================================
        // Packets
        // ====================================================

        printLine(
            START_ROW + 2,
            "Packets    : " +
                std::to_string(stats.sent()) +
                " sent / " +
                std::to_string(stats.received()) +
                " received");

        // ====================================================
        // Loss
        // ====================================================

        {
            std::ostringstream out;

            out
                << "Loss       : "
                << std::fixed
                << std::setprecision(2)
                << stats.lossPercent()
                << "%";

            printLine(
                START_ROW + 3,
                out.str());
        }

        // ====================================================
        // Current latency
        // ====================================================

        if (result.received)
        {
            printLine(
                START_ROW + 5,
                "Latency    : current " +
                    formatMs(result.rttMs));
        }
        else
        {
            printLine(
                START_ROW + 5,
                "Latency    : current --");
        }

        // ====================================================
        // Min / Avg / Max
        // ====================================================

        if (stats.received() > 0)
        {
            printLine(
                START_ROW + 6,
                "             min     " +
                    formatMs(stats.minRtt()));

            printLine(
                START_ROW + 7,
                "             avg     " +
                    formatMs(stats.avgRtt()));

            printLine(
                START_ROW + 8,
                "             max     " +
                    formatMs(stats.maxRtt()));
        }
        else
        {
            printLine(
                START_ROW + 6,
                "             min     --");

            printLine(
                START_ROW + 7,
                "             avg     --");

            printLine(
                START_ROW + 8,
                "             max     --");
        }

        // ====================================================
        // Jitter
        // ====================================================

        if (stats.received() > 1)
        {
            printLine(
                START_ROW + 10,
                "Jitter     : " +
                    formatMs(stats.jitter()));
        }
        else
        {
            printLine(
                START_ROW + 10,
                "Jitter     : --");
        }

        // ====================================================
        // Status
        // ====================================================

        printLine(
            START_ROW + 12,
            "Status     : " +
                getStatus(stats, online));

        std::cout.flush();
    }
};

// ============================================================
// Ping Service - Facade
// ============================================================

class PingService
{
private:

    PingConfig config_;

    std::unique_ptr<IHostResolver> resolver_;

    std::unique_ptr<IPingTransport> transport_;

    Statistics statistics_;

public:

    PingService(
        PingConfig config,
        std::unique_ptr<IHostResolver> resolver,
        std::unique_ptr<IPingTransport> transport)
        : config_(std::move(config)),
          resolver_(std::move(resolver)),
          transport_(std::move(transport))
    {
    }

    void run()
    {
        // ----------------------------------------------------
        // Initial DNS resolution
        // ----------------------------------------------------

        sockaddr_in destination =
            resolver_->resolve(
                config_.host);

        char addressBuffer[INET_ADDRSTRLEN]{};

        inet_ntop(
            AF_INET,
            &destination.sin_addr,
            addressBuffer,
            sizeof(addressBuffer));

        TerminalDashboard dashboard(
            config_.host,
            addressBuffer);

        dashboard.initialize();

        std::uint16_t sequence = 1;

        // ====================================================
        // Continuous ping
        // ====================================================

        while (g_running)
        {
            // ------------------------------------------------
            // Resolve host every cycle.
            //
            // This allows recovery when network/DNS comes
            // back after disconnect.
            // ------------------------------------------------

            bool resolved = true;

            try
            {
                destination =
                    resolver_->resolve(
                        config_.host);

                inet_ntop(
                    AF_INET,
                    &destination.sin_addr,
                    addressBuffer,
                    sizeof(addressBuffer));
            }
            catch (...)
            {
                resolved = false;
            }

            PingResult result;

            if (resolved)
            {
                result =
                    transport_->ping(
                        destination,
                        sequence++,
                        config_.timeoutMs);
            }
            else
            {
                result.sent = false;
                result.received = false;
            }

            // ------------------------------------------------
            // IMPORTANT
            //
            // Count only packets that were actually sent.
            // ------------------------------------------------

            if (result.sent)
            {
                statistics_.recordSent();
            }

            // ------------------------------------------------
            // Count only valid replies.
            // ------------------------------------------------

            if (result.received)
            {
                statistics_.recordReceived(
                    result.rttMs);
            }

            // ------------------------------------------------
            // Update dashboard
            // ------------------------------------------------

            dashboard.update(
                statistics_,
                result);

            // ------------------------------------------------
            // Wait
            //
            // Sleep in small chunks so Ctrl+C responds
            // quickly.
            // ------------------------------------------------

            int remaining =
                config_.intervalMs;

            while (remaining > 0 &&
                   g_running)
            {
                int sleepTime =
                    std::min(
                        remaining,
                        100);

                std::this_thread::sleep_for(
                    std::chrono::milliseconds(
                        sleepTime));

                remaining -= sleepTime;
            }
        }

        showFinalStatistics();
    }

private:

    void showFinalStatistics()
    {
        std::cout
            << "\n\n"
            << "============================================\n"
            << "                 FINAL RESULT               \n"
            << "============================================\n";

        std::cout
            << "Packets : "
            << statistics_.sent()
            << " sent, "
            << statistics_.received()
            << " received\n";

        std::cout
            << "Loss    : "
            << std::fixed
            << std::setprecision(2)
            << statistics_.lossPercent()
            << "%\n";

        if (statistics_.received() > 0)
        {
            std::cout
                << "Min RTT : "
                << statistics_.minRtt()
                << " ms\n"

                << "Avg RTT : "
                << statistics_.avgRtt()
                << " ms\n"

                << "Max RTT : "
                << statistics_.maxRtt()
                << " ms\n"

                << "Jitter  : "
                << statistics_.jitter()
                << " ms\n";
        }

        std::cout
            << "============================================\n";
    }
};

// ============================================================
// Main
// ============================================================

int main(int argc, char* argv[])
{
    // --------------------------------------------------------
    // Arguments
    // --------------------------------------------------------

    if (argc != 2)
    {
        std::cout
            << "Usage: "
            << argv[0]
            << " <host>\n\n"

            << "Example:\n"
            << "  "
            << argv[0]
            << " google.com\n";

        return 1;
    }

    // --------------------------------------------------------
    // Ctrl+C
    // --------------------------------------------------------

    std::signal(
        SIGINT,
        signalHandler);

    // --------------------------------------------------------
    // Configuration
    // --------------------------------------------------------

    PingConfig config;

    config.host =
        argv[1];

    config.timeoutMs =
        1000;

    config.intervalMs =
        1000;

    // --------------------------------------------------------
    // Create components
    // --------------------------------------------------------

    auto resolver =
        std::make_unique<
            IPv4HostResolver>();

    auto transport =
        std::make_unique<
            IcmpTransport>();

    // --------------------------------------------------------
    // Start service
    // --------------------------------------------------------

    try
    {
        PingService service(
            config,
            std::move(resolver),
            std::move(transport));

        service.run();
    }
    catch (const std::exception& ex)
    {
        std::cerr
            << "Error: "
            << ex.what()
            << '\n';

        return 1;
    }

    return 0;
}


