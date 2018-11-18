#include <iostream>
#include "server.h"
#include "boost/asio.hpp"
#include "database.h"
#include "string_utils.h"

namespace ba = boost::asio;

AsyncProcessor::AsyncProcessor(ba::ip::tcp::socket socket_) :
        client_socket{std::move(socket_)},
        allowed_commands{"INSERT", "TRUNCATE", "INTERSECTION", "SYMMETRIC_DIFFERENCE"},
        allowed_tables{"A", "B"} {}

void AsyncProcessor::receive_and_process() {
    // https://stackoverflow.com/questions/34719233/why-capture-this-as-well-as-shared-pointer-to-this-in-lambdas/34719861
    self = shared_from_this();

    ba::async_read_until(client_socket, buffer_data, '\n',
                         [this](const boost::system::error_code& ec, size_t size_read) {
                             receive_and_process_loop(ec, size_read);
                         });
}


void AsyncProcessor::receive_and_process_loop(const boost::system::error_code& ec, size_t size_read) {
    if (ec) {
        self = nullptr;
        return;
    }
    auto& db = Database::get_db();

    std::string full_command;
    std::getline(std::istream(&buffer_data), full_command);
    std::vector<std::string> command_with_arguments = split(full_command, ' ');

    try {
        if (command_with_arguments.empty())
            throw std::logic_error("incorrect buffer");
        auto& command = command_with_arguments[0];
        if (command == "INTERSECTION") {
            if (command_with_arguments.size() != 1)
                throw std::logic_error("INTERSECTION command should have no arguments");
            std::cout << db.intersection();
        } else if (command == "SYMMETRIC_DIFFERENCE") {
            if (command_with_arguments.size() != 1)
                throw std::logic_error("SYMMETRIC_DIFFERENCE command should have no arguments");
            std::cout << db.symmetric_difference();
        } else if (command == "TRUNCATE") {
            if (command_with_arguments.size() != 2)
                throw std::logic_error("TRUNCATE command should have 1 argument");
            auto table = command_with_arguments[1];
            if (allowed_tables.count(table) == 0)
                throw std::logic_error("Unknown table " + table);
            db.truncate(table);
        } else if (command == "INSERT") {
            if (command_with_arguments.size() != 4)
                throw std::logic_error("INSERT command should have 3 arguments");
            std::string table = command_with_arguments[1];
            if (allowed_tables.count(table) == 0)
                throw std::logic_error("Unknown table " + table);
            int id = std::stoi(command_with_arguments[2]);
            if (!db.insert(table, id, command_with_arguments[3]))
                throw std::logic_error("duplicate " + std::to_string(id));
        } else
            throw std::logic_error("Unknown command " + command);

        std::cout << "OK\n";
    } catch (const std::logic_error& e) {
        std::cout << "ERR " << e.what() << "\n";
    }

    ba::async_read_until(client_socket, buffer_data, '\n',
                         [this](const boost::system::error_code& ec, size_t size_read) {
                             receive_and_process_loop(ec, size_read);
                         });

}

AsyncProcessor::~AsyncProcessor() {
    client_socket.close();
}


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
            client_handler_ptr->receive_and_process();
        }
        accept();
    });
}
