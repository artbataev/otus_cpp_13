#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <thread>
#include <mutex>

struct QueryResultItem {
    int id;
    std::string name_a;
    std::string name_b;
};

using query_result_t = std::vector<QueryResultItem>;

std::ostream& operator<<(std::ostream& stream, query_result_t result);

class Database {
    using table_t = std::map<int, std::string>;
public:
    Database(Database const&) = delete;

    Database& operator=(Database const&) = delete;

    static Database& get_db() {
        static Database db;
        return db;
    }

    void truncate(const std::string& db_name);

    bool insert(const std::string& db_name, int id, const std::string& name);

    query_result_t intersection();

    query_result_t symmetric_difference();

private:
    table_t A;
    table_t B;
    std::mutex A_mutex;
    std::mutex B_mutex;
    std::set<std::string> allowed_names;
    Database();
    ~Database() = default;
};
