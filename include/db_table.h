#pragma once

#include <vector>
#include <string>
#include <map>
#include <iostream>

struct QueryResultItem {
    int id;
    std::string name_a;
    std::string name_b;
};

using query_result_t = std::vector<QueryResultItem>;

std::ostream& operator<<(std::ostream& stream, query_result_t result);


class DBTable {
public:
    void truncate();

    bool insert(int id, const std::string& name);

    query_result_t intersection(const DBTable& other);

    query_result_t symmetric_difference(const DBTable& other);

private:
    std::map<int, std::string> data;
};
