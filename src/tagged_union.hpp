#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>


template <typename... Ts>
    requires(sizeof...(Ts) > 0)
struct TaggedUnion {
    static constexpr size_t TagCount = sizeof...(Ts);

    enum class Tag : size_t { None = TagCount };

    Tag tag = Tag::None;

    template <typename T>
        requires(std::is_same_v<T, Ts> || ...)
    static constexpr Tag tag_of = []() constexpr {
        std::size_t index = 0;
        auto _ = ((std::is_same_v<T, Ts> ? true : (++index, false)) || ...);
        return Tag(index);
    }();

    ~TaggedUnion() {
        destroy();
    }

    template <typename T, typename... Args>
        requires(std::is_same_v<T, Ts> || ...)
    void set(Args&&... args) {
        destroy();
        new (&storage) T(std::forward<Args>(args)...);
        tag = tag_of<T>;
    }

    template <typename T>
        requires(std::is_same_v<T, Ts> || ...)
    bool is_current() const {
        return tag_of<T> == tag;
    }

    template <typename T>
        requires(std::is_same_v<T, Ts> || ...)
    T& get() {
        if (tag != tag_of<T>) throw std::bad_variant_access();
        return *reinterpret_cast<T*>(&storage);
    }

    template <typename T>
        requires(std::is_same_v<T, Ts> || ...)
    const T& get() const {
        if (tag != tag_of<T>) throw std::bad_variant_access();
        return *reinterpret_cast<const T*>(&storage);
    }

    template <typename Func>
    decltype(auto) apply(Func&& func) {
        return apply_impl(func, std::index_sequence_for<Ts...>{});
    }

  private:
    constexpr static const size_t storage_size = std::max({sizeof(Ts)...});
    alignas(Ts...) std::byte storage[storage_size];

    void destroy() {
        if (tag == Tag::None) return;
        destroy_impl(std::index_sequence_for<Ts...>{});
        tag = Tag::None;
    }

    template <std::size_t... Is>
    void destroy_impl(std::index_sequence<Is...>) {
        bool matched = false;
        (try_destroy<Is>(matched), ...);
    }

    template <std::size_t I>
    void try_destroy(bool& matched) {
        if (!matched && tag == Tag(I)) {
            matched = true;
            using T = std::tuple_element_t<I, std::tuple<Ts...>>;
            reinterpret_cast<T*>(&storage)->~T();
        }
    }

    template <typename Func, std::size_t... Is>
    decltype(auto) apply_impl(Func& func, std::index_sequence<Is...>) {
        using ReturnType = std::invoke_result_t<Func, std::tuple_element_t<0, std::tuple<Ts...>>&>;

        constexpr static auto table = [] {
            using Tup = std::tuple<Ts...>;
            using Fn = ReturnType (*)(std::byte(&)[storage_size], Func&);
            return std::array<Fn, sizeof...(Ts)>{
                +[](std::byte(&storage)[storage_size] , Func& f) -> ReturnType {
                    using T = std::tuple_element_t<Is, Tup>;
                    return f(*reinterpret_cast<T*>(&storage));
                } ...
            };
        }();

        return table[static_cast<size_t>(tag)](storage, func);
    }

};
