
/*
    Compile this code and run with below command in linux to keep system awake/no sleep.
    
    systemd-inhibit --why="Running C++ simulation" ./sim
*/

#include <iostream>
#include <thread>
#include <chrono>

int main()
{
    std::cout << "Simulation running... Press Ctrl+C to stop\n";

    while (true)
    {
        std::cout << "Its keep system awake... Press Ctrl+C to stop\n";
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return 0;
}
