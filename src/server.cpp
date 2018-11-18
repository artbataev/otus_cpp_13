#include <iostream>
#include "server.h"
#include "boost/asio.hpp"
#include "database.h"

namespace ba = boost::asio;

AsyncProcessor::AsyncProcessor(ba::ip::tcp::socket socket_) :
        client_socket{std::move(socket_)},
        buffer_data{0}{
    auto& db = Database::get_db();
    while (true) {
        try {
            char data[256];
            size_t len = client_socket.read_some(ba::buffer(data));
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
                    ok = db.insert("A", id, name);
                    if (!ok)
                        message = "duplicate " + std::to_string(id);
                } else if (table == "B") {
                    ok = db.insert("B", id, name);
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
                    db.truncate("A");
                else if (table == "B")
                    db.truncate("B");
                else {
                    ok = false;
                    message = "Unknown Table";
                }

            } else if (command == "INTERSECTION") {
                std::cout << db.intersection();
            } else if (command == "SYMMETRIC_DIFFERENCE") {
                std::cout << db.symmetric_difference();
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

//void AsyncProcessor::receive_and_process() {
//    // https://stackoverflow.com/questions/34719233/why-capture-this-as-well-as-shared-pointer-to-this-in-lambdas/34719861
//    self = shared_from_this();
//    client_socket.async_read_some(ba::buffer(buffer_data),
//                                  [this](const boost::system::error_code& ec, size_t size_read) {
//                                      receive_and_process_loop(ec, size_read);
//                                  });
//}
//
//
//void AsyncProcessor::receive_and_process_loop(const boost::system::error_code& ec, size_t size_read) {
//    if (ec) {
//        client_socket.close();
//        async::disconnect(handle);
//        self = nullptr;
//        return;
//    }
//    async::receive(handle, buffer_data, size_read);
//    client_socket.async_read_some(ba::buffer(buffer_data),
//                                  [this](const boost::system::error_code& ec, size_t size_read) {
//                                      receive_and_process_loop(ec, size_read);
//                                  });
//}


Server::Server(int port_) :
        port{port_},
        server_endpoint{ba::ip::tcp::v4(), static_cast<unsigned short>(port_)},
        server_acceptor{server_service, server_endpoint},
        server_socket{server_service} {}


void Server::run() {
    ba::signal_set signal_set(server_service, SIGTERM, SIGINT); // interrupt, kill
    signal_set.async_wait(
            [this](const boost::system::error_code& ec, int signal_number) {
                server_service.stop();
            });
    accept();
    server_service.run();
}


void Server::accept() {
    server_acceptor.async_accept(server_socket, [this](const boost::system::error_code& ec) {
        if (!ec) {
            auto client_handler_ptr = std::make_shared<AsyncProcessor>(std::move(server_socket));
//            client_handler_ptr->receive_and_process();
        }
        accept();
    });
}
