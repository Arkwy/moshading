#include <algorithm>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>
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

    ~TaggedUnion() { destroy(); }

    template <typename T, typename... Args>
    void set(Args&&... args) {
        destroy();
        new (&storage) T(std::forward<Args>(args)...);
        tag = Tag(variadic_index_of<T, Ts...>());
    }

    template <typename T>
    T& get() {
        if (tag != Tag(variadic_index_of<T, Ts...>())) throw std::bad_variant_access();  // or assert
        return *reinterpret_cast<T*>(&storage);
    }

    template <typename T>
    const T& get() const {
        if (tag != Tag(variadic_index_of<T, Ts...>())) throw std::bad_variant_access();  // or assert
        return *reinterpret_cast<const T*>(&storage);
    }

    template <typename Func>
    decltype(auto) apply(Func&& func) {
        return apply_impl(std::forward<Func>(func), std::index_sequence_for<Ts...>{});
    }

  private:
    alignas(Ts...) std::byte storage[std::max({sizeof(Ts)...})];

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
    decltype(auto) apply_impl(Func&& func, std::index_sequence<Is...>) {
        using ReturnType = std::invoke_result_t<Func, std::tuple_element_t<0, std::tuple<Ts...>>&>;

        static_assert(
            (std::is_same_v<
                 ReturnType,
                 std::invoke_result_t<Func, std::tuple_element_t<Is, std::tuple<Ts...>>&>> &&
             ...),
            "All types of this union does not return the same type for the applied function."
        );

        if constexpr (!std::is_void_v<ReturnType>) {
            bool matched = false;
            std::optional<ReturnType> result = std::nullopt;
            (try_apply<Func, ReturnType, Is>(func, matched, result), ...);
            assert(result.has_value());
            ReturnType realsult = std::move(result.value());
            return realsult;
        } else {
            bool matched = false;
            (try_apply<Func, Is>(func, matched), ...);
        }
    }

    template <typename Func, typename ReturnType, std::size_t I>
    void try_apply(Func& func, bool& matched, std::optional<ReturnType>& result) {
        using T = std::tuple_element_t<I, std::tuple<Ts...>>;
        if (!matched && tag == Tag(I)) {
            assert(!result.has_value());
            result = func(*reinterpret_cast<T*>(&storage));
        }
    }

    template <typename Func, std::size_t I>
    void try_apply(Func& func, bool& matched) {
        using T = std::tuple_element_t<I, std::tuple<Ts...>>;
        if (!matched && tag == Tag(I)) {
            matched = true;
            func(*reinterpret_cast<T*>(&storage));
        }
    }
};
