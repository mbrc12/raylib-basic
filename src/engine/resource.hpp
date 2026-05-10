#pragma once

#include <memory>

#include "raylib.h"

namespace engine {

template<typename T>
struct DeleterFor : std::default_delete<T> {};

template<>
struct DeleterFor<Texture2D> {
    void operator()(Texture2D* t) const {
        UnloadTexture(*t);
        delete t;
    }
};

template<>
struct DeleterFor<Font> {
    void operator()(Font* f) const {
        UnloadFont(*f);
        delete f;
    }
};

template<typename T, typename Deleter = DeleterFor<T>>
class Resource {
  public:
    Resource() : m_ptr(nullptr, Deleter{}) {}

    Resource(T* ptr) : m_ptr(ptr, Deleter{}) {}
    Resource(T* ptr, Deleter d) : m_ptr(ptr, d) {}

    explicit Resource(T value) : m_ptr(new T(value), Deleter{}) {}
    Resource(T value, Deleter d) : m_ptr(new T(value), d) {}

    Resource(const Resource&) = default;
    Resource(Resource&&) noexcept = default;
    Resource& operator=(const Resource&) = default;
    Resource& operator=(Resource&&) noexcept = default;

    T* operator->() { return m_ptr.get(); }
    const T* operator->() const { return m_ptr.get(); }
    T& operator*() { return *m_ptr; }
    const T& operator*() const { return *m_ptr; }

    size_t use_count() const { return m_ptr.use_count(); }
    explicit operator bool() const { return m_ptr != nullptr; }

  private:
    std::shared_ptr<T> m_ptr;
};

} // namespace engine
