#include <iostream>
#include <thread>
#include <string>
#include "server.h"


int main(int argc, char *argv[]) {
    bool executed_correctly = true;
    std::stringstream whats_wrong;

    int port = 0;

    if (argc != 2) {
        executed_correctly = false;
        whats_wrong << "Incorrect number of arguments";
    } else {
        try {
            port = std::stoi(argv[1]);
            if (port < 0) {
                whats_wrong << "Port must be non-negative";
                executed_correctly = false;
            }
        } catch (std::exception&) {
            executed_correctly = false;
            whats_wrong << "Port must be a number";
        }
    }

    if (!executed_correctly) {
        std::cout << "Error: " << whats_wrong.str() << std::endl;
        std::cout << "Execute with 1 argument: port, e.g.:" << std::endl;
        std::cout << argv[0] << " 9000" << std::endl;
        exit(0);
    }

    Server server{port};
    server.run();
    return 0;
}
