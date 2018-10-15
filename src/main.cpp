#include "boost/asio.hpp"
#include <cstdlib>
#include <iostream>
#include <thread>
#include <map>
#include <string>
#include <sstream>

namespace ba = boost::asio;

struct ResultItem {
    int id;
    std::string name_a;
    std::string name_b;
};

using result_t = std::vector<ResultItem>;

class Table {
public:
    void truncate() {
        data.clear();
    }

    bool insert(int id, const std::string& name) {
        if (data.count(id) == 0) {
            data[id] = name;
            return true;
        } else
            return false;
    }

    result_t intersection(const Table& other) {
        result_t result;
        auto current_pointer = data.cbegin();
        auto other_pointer = other.data.cbegin();
        while (current_pointer != data.cend() && other_pointer != other.data.cend()) {
            if (current_pointer->first == other_pointer->first) {
                result.push_back({current_pointer->first, current_pointer->second, other_pointer->second});
                current_pointer++;
                other_pointer++;
            } else if (current_pointer->first < other_pointer->first)
                current_pointer++;
            else
                other_pointer++;
        }
        return result;
    }

    result_t symmetric_difference(const Table& other) {
        result_t result;
        auto current_pointer = data.cbegin();
        auto other_pointer = other.data.cbegin();
        while (current_pointer != data.cend() || other_pointer != other.data.cend()) {
            if (current_pointer == data.cend()) {
                result.push_back({other_pointer->first, "", other_pointer->second});
                other_pointer++;
            } else if (other_pointer == other.data.cend()) {
                result.push_back({current_pointer->first, current_pointer->second, ""});
                current_pointer++;
            } else if (current_pointer->first < other_pointer->first) {
                result.push_back({current_pointer->first, current_pointer->second, ""});
                current_pointer++;
            } else if (other_pointer->first < current_pointer->first) {
                result.push_back({other_pointer->first, "", other_pointer->second});
                other_pointer++;
            }
        }
        return result;
    }

private:
    std::map<int, std::string> data;
};

std::ostream& operator<<(std::ostream& stream, result_t result) {
    for (const auto& elem: result)
        stream << elem.id << "," << elem.name_a << "," << elem.name_b << "\n";

    return stream;
}

void client_session(ba::ip::tcp::socket service_socket) {
    Table A;
    Table B;
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

//            std::cout << "received " << len << " : " << full_command << std::endl;
//            ba::write(service_socket, ba::buffer("pong", 4));
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
