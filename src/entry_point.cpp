#include "boost/asio.hpp"
#include <iostream>
#include <thread>
#include <string>
#include "db_table.h"

namespace ba = boost::asio;


void client_session(ba::ip::tcp::socket service_socket) {
    DBTable A;
    DBTable B;
    while (true) {
        try {
            char data[256];
            size_t len = service_socket.read_some(ba::buffer(data));
            std::string full_command{data, len};
            std::string command;
            std::stringstream command_stream(full_command);
            command_stream >> command;

            bool ok = true;
            std::string message;
            if (command == "INSERT") {
                std::string table;
                int id;
                std::string name;
                command_stream >> table >> id >> name;
                if (table == "A") {
                    ok = A.insert(id, name);
                    if (!ok)
                        message = "duplicate " + std::to_string(id);
                } else if (table == "B") {
                    ok = B.insert(id, name);
                    if (!ok)
                        message = "duplicate " + std::to_string(id);
                } else {
                    ok = false;
                    message = "Unknown Table";
                }

            } else if (command == "TRUNCATE") {
                std::string table;
                command_stream >> table;
                if (table == "A")
                    A.truncate();
                else if (table == "B")
                    B.truncate();
                else {
                    ok = false;
                    message = "Unknown Table";
                }

            } else if (command == "INTERSECTION") {
                std::cout << A.intersection(B);
            } else if (command == "SYMMETRIC_DIFFERENCE") {
                std::cout << A.symmetric_difference(B);
            } else {
                ok = false;
                message = "Unknown Command";
            }

            if (ok)
                std::cout << "OK\n";
            else
                std::cout << "ERR " << message << "\n";

        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    ba::io_service service;
    ba::ip::tcp::endpoint service_endpoint(ba::ip::tcp::v4(), std::atoi(argv[1]));
    ba::ip::tcp::acceptor service_acceptor(service, service_endpoint);

    while (true) {
        auto sock = ba::ip::tcp::socket(service);
        service_acceptor.accept(sock);
        std::thread(client_session, std::move(sock)).detach();
    }
    return 0;
}
