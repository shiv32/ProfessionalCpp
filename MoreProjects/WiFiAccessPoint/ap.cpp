/*
    Convert your wifi adapter into Access point if adapter has AP module.
    OS: Manjaro Linux XFCE
*/
#include <cstdlib>
#include <iostream>
#include <string>

bool execute(const std::string &cmd)
{
    std::cout << "\n$ " << cmd << "\n\n";
    return std::system(cmd.c_str()) == 0;
}

void dashboard()
{
    std::system("clear"); // Clear terminal

    std::cout << "\n========== Network Dashboard ==========\n";
    std::system("nmcli device status");
    std::cout << "------------------------------------------\n";
    std::system("sudo iptables -L FORWARD -n");
    std::cout << "=======================================\n";
}

int main()
{
    dashboard();

    std::string ifname;

    std::cout << "Interface (e.g. wlan0): ";
    std::cin >> ifname;

    while (true)
    {
        int choice;

        std::cout << "\n1. Start Hotspot\n"
                  << "2. Stop Hotspot\n"
                  << "3. Refresh Dashboard\n"
                  << "4. Unmanaged\n"
                  << "5. Managed\n"
                  << "0. Exit\n"
                  << "Choice: ";

        std::cin >> choice;

        switch (choice)
        {
        case 1:
            execute("sudo iptables -P FORWARD ACCEPT"); // Temporarily change the Linux forwarding policy to ACCEPT
            execute(
                "nmcli device wifi hotspot "
                "ifname " +
                ifname +
                " ssid AntHotspot "
                "password yourPassword");
            dashboard();
            break;

        case 2:
            execute("nmcli device disconnect " + ifname);
            dashboard();
            break;

        case 3:
            dashboard();
            break;
        case 4:
            std::system("sudo nmcli device set wlan0 managed no");
            dashboard();
            break;
        case 5:
            std::system("sudo nmcli device set wlan0 managed yes");
            dashboard();
            break;
        case 0:
            return 0;

        default:
            std::cout << "Invalid choice\n";
        }
    }
}