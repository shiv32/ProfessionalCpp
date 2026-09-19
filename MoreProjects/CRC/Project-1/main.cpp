#include <iostream>
#include <string>
#include <cstdint>

uint8_t calculateCRC8(const std::string &data)
{
    uint8_t crc = 0x00;
    const uint8_t polynomial = 0x07; // CRC-8 polynomial

    for (uint8_t byte : data)
    {
        crc ^= byte;

        for (int bit = 0; bit < 8; ++bit)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ polynomial;
            else
                crc <<= 1;
        }
    }

    return crc;
}

int main()
{
    system("clear && printf '\e[3J'"); // clean the terminal before output in linux

    std::string data = "Hello";

    uint8_t crc = calculateCRC8(data);

    std::cout << "Data: " << data << '\n';
    std::cout << "CRC:  0x"
              << std::hex
              << static_cast<int>(crc)
              << '\n';

    return 0;
}
