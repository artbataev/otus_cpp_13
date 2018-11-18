#include "database.h"


std::ostream& operator<<(std::ostream& stream, query_result_t result) {
    for (const auto& elem: result)
        stream << elem.id << "," << elem.name_a << "," << elem.name_b << "\n";
    return stream;
}

Database::Database():allowed_names{"A", "B"}{};

void Database::truncate(const std::string& db_name) {
    if (allowed_names.count(db_name) == 0)
        throw std::logic_error("Dababase name " + db_name + " not allowed");
    auto& table = db_name == "A" ? A : B;
    auto& table_mutex = db_name == "A" ? A_mutex : B_mutex;

    std::lock_guard<std::mutex> guard{table_mutex};
    table.clear();
}

bool Database::insert(const std::string& db_name, int id, const std::string& name) {
    if (allowed_names.count(db_name) == 0)
        throw std::logic_error("Dababase name " + db_name + " not allowed");
    auto& table = db_name == "A" ? A : B;
    auto& table_mutex = db_name == "A" ? A_mutex : B_mutex;

    std::lock_guard<std::mutex> guard{table_mutex};
    if (table.count(id) == 0) {
        table[id] = name;
        return true;
    } else
        return false;
}

query_result_t Database::intersection() {
    query_result_t result;
    std::scoped_lock guard{A_mutex, B_mutex};

    auto A_pointer = A.cbegin();
    auto B_pointer = B.cbegin();
    while (A_pointer != A.cend() && B_pointer != B.cend()) {
        if (A_pointer->first == B_pointer->first) {
            result.push_back({A_pointer->first, A_pointer->second, B_pointer->second});
            A_pointer++;
            B_pointer++;
        } else if (A_pointer->first < B_pointer->first)
            A_pointer++;
        else
            B_pointer++;
    }
    return result;
}

query_result_t Database::symmetric_difference() {
    query_result_t result;
    std::scoped_lock guard{A_mutex, B_mutex};

    auto A_pointer = A.cbegin();
    auto B_pointer = B.cbegin();
    while (A_pointer != A.cend() || B_pointer != B.cend()) {
        if (A_pointer == A.cend()) {
            result.push_back({B_pointer->first, "", B_pointer->second});
            B_pointer++;
        } else if (B_pointer == B.cend()) {
            result.push_back({A_pointer->first, A_pointer->second, ""});
            A_pointer++;
        } else if (A_pointer->first < B_pointer->first) {
            result.push_back({A_pointer->first, A_pointer->second, ""});
            A_pointer++;
        } else if (B_pointer->first < A_pointer->first) {  
            result.push_back({B_pointer->first, "", B_pointer->second});
            B_pointer++;
        } else { // B_pointer->first == A_pointer->first
            A_pointer++;
            B_pointer++;
        }
    }
    return result;
}
