#include "RestServer.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>

static RestServer* server_instance = nullptr;

void signal_handler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received." << std::endl;
    if (server_instance) {
        server_instance->stop();
    }
    exit(signum);
}

int main() {
    try {
        RestServer server(8080);
        server_instance = &server;
        
        // Register signal handler
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        
        server.start();
        
        // Keep the server running
        while (server.isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        return 0;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
