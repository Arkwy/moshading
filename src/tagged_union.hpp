#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <variant>


template <typename Target, typename VariadicHead, typename... VariadicTail>
constexpr size_t variadic_index_of() {
    if constexpr (std::is_same_v<Target, VariadicHead>) {
        return 0;
    } else {
        static_assert(sizeof...(VariadicTail), "Target type not found in type list");
        return 1 + variadic_index_of<Target, VariadicTail...>();
    }
}


template <typename... Ts>
struct TaggedUnion {
    static constexpr size_t TagCount = sizeof...(Ts);

    enum class Tag { None = TagCount };

    Tag tag = Tag::None;

    ~TaggedUnion() {
        destroy();
    }

    template <typename T, typename... Args>
    void set(Args&&... args) {
        destroy();
        new (&storage) T(std::forward<Args>(args)...);
        tag = Tag(variadic_index_of<T, Ts...>());
    }

    template <typename T>
    T& get() {
        if (tag != Tag(variadic_index_of<T, Ts...>())) throw std::bad_variant_access(); // or assert
        return *reinterpret_cast<T*>(&storage);
    }

    template <typename T>
    const T& get() const {
        if (tag != Tag(variadic_index_of<T, Ts...>())) throw std::bad_variant_access(); // or assert
        return *reinterpret_cast<const T*>(&storage);
    }

private:
    alignas(Ts...) std::byte storage[std::max({sizeof(Ts)...})];

    void destroy() {
        if (tag == Tag::None) return;
        destroy_impl(tag);
        tag = Tag::None;
    }

    void destroy_impl(Tag current) {
        destroy_at_index(current, std::index_sequence_for<Ts...>{});
    }

    template <std::size_t... Is>
    void destroy_at_index(Tag current, std::index_sequence<Is...>) {
        auto try_destroy = [&](auto I) {
            if (current == Tag(I)) {
                using T = std::tuple_element_t<I, std::tuple<Ts...>>;
                reinterpret_cast<T*>(&storage)->~T();
            }
        };

        (try_destroy(Is), ...);
    }
};

// Example:

// struct A { A(int) {} ~A() {} };
// struct B { B(float) {} ~B() {} };

// TaggedUnion<A, B> u;
// u.set<0>(42);     // construct A
// u.set<1>(3.14f);  // destroy A, construct B

// auto& b = u.get<1>();
