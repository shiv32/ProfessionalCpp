#include <iostream>
#include <fstream>
#include <memory>

std::string filePath{"../app.txt"};

class Settings
{
    std::ifstream m_InputStream{};

public:
    Settings(const char *fileName) : m_InputStream{fileName}
    {
    }

    std::ifstream &GetStream()
    {
        return m_InputStream;
    }
};

namespace Wrong
{
    std::ifstream &GetStreamInfo()
    {
        auto settings = std::make_shared<Settings>(filePath.c_str());

        auto &stream = settings->GetStream();

        if (stream.is_open())
        {
            std::cout << "[Wrong][GetStreamInfo] Stream is open\n";
        }
        else
        {
            std::cout << "[Wrong][GetStreamInfo] Stream is already closed\n";
        }

        return stream;
    }

    void Main()
    {
        auto &stream = GetStreamInfo();

        if (stream.is_open())
        {
            std::cout << "[Wrong][main] Stream is open\n";
        }
        else
        {
            std::cout << "[Wrong][main] Stream is already closed\n";
        }
    }
}

namespace Correct
{
    using StreamPtr = std::shared_ptr<std::ifstream>;

    StreamPtr GetStreamInfo()
    {
        auto settings = std::make_shared<Settings>(filePath.c_str());

        StreamPtr stream{settings, &settings->GetStream()};

        if (stream->is_open())
        {
            std::cout << "[Correct][GetStreamInfo] Stream is open\n";
        }
        else
        {
            std::cout << "[Correct][GetStreamInfo] Stream is already closed\n";
        }

        return stream;
    }

    void Main()
    {
        auto stream = GetStreamInfo();
        
        if (stream->is_open())
        {
            std::cout << "[Correct][main] Stream is open\n";
        }
        else
        {
            std::cout << "[Correct][main] Stream is already closed\n";
        }
    }
}

int main(int argc, const char *argv[])
{
    system("clear && printf '\e[3J'"); // clean the terminal before output in linux

    Wrong::Main();

    Correct::Main();

    return 0;
}