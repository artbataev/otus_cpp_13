#include <vector>
#include <iostream>
#include "db_table.h"

std::ostream& operator<<(std::ostream& stream, query_result_t result) {
    for (const auto& elem: result)
        stream << elem.id << "," << elem.name_a << "," << elem.name_b << "\n";
    return stream;
}

void DBTable::truncate() {
    data.clear();
}

bool DBTable::insert(int id, const std::string& name) {
    if (data.count(id) == 0) {
        data[id] = name;
        return true;
    } else
        return false;
}

query_result_t DBTable::intersection(const DBTable& other) {
    query_result_t result;
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

query_result_t DBTable::symmetric_difference(const DBTable& other) {
    query_result_t result;
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

