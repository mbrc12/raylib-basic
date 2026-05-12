#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

// BEWARE: This registry stores everything as pointers
namespace engine::registry {

template<typename T>
using bucket_type = std::unordered_map<std::string, T*>;

template<typename T>
bucket_type<T>& bucket() {
    static bucket_type<T> items;
    return items;
}

template<typename T> T* get(std::string_view name = {}) {
    auto& entries = bucket<T>();
    auto it = entries.find(std::string(name));
    if (it == entries.end()) {
        throw std::out_of_range("engine::registry item not found");
    }

    return it->second;
}

template<typename T> void store(T* value, std::string_view name = {}) {
    auto& entries = bucket<T>();
    auto [it, _] = entries.insert_or_assign(std::string(name), value);
}

} // namespace engine::registry
