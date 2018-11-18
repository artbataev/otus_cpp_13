#pragma once

#include "boost/asio.hpp"
#include <unordered_set>

namespace ba = boost::asio;

class AsyncProcessor : public std::enable_shared_from_this<AsyncProcessor> {
public:
    explicit AsyncProcessor(ba::ip::tcp::socket socket_);

    void receive_and_process();
    ~AsyncProcessor();

private:
    void receive_and_process_loop(const boost::system::error_code& ec, size_t size_read);

    ba::ip::tcp::socket client_socket;
    ba::streambuf buffer_data;
    std::shared_ptr<AsyncProcessor> self;
    const std::unordered_set<std::string> allowed_commands;
    const std::unordered_set<std::string> allowed_tables;
};

class Server {
public:
    explicit Server(int port_);

    void run();

    void accept();


private:
    int port;
    ba::io_service server_service;
    ba::ip::tcp::endpoint server_endpoint;
    ba::ip::tcp::acceptor server_acceptor;
    ba::ip::tcp::socket server_socket;
};
