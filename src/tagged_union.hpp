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
    decltype(auto) apply_impl(Func& func, std::index_sequence<Is...>) {
        // using ReturnType = std::invoke_result_t<Func, std::tuple_element_t<0, std::tuple<Ts...>>&>;
        using _ReturnType = std::invoke_result_t<Func, std::tuple_element_t<0, std::tuple<Ts...>>&>;

        static_assert(
            (std::is_same_v<_ReturnType, std::invoke_result_t<Func, std::tuple_element_t<Is, std::tuple<Ts...>>&>> && ...
            ),
            "All types of this union does not return the same type for the applied function."
        );

        using ReturnType = std::conditional_t<
            std::is_reference_v<_ReturnType>,
            std::reference_wrapper<std::remove_reference_t<_ReturnType>>,
            _ReturnType>;


        bool matched = false;
        if constexpr (!std::is_void_v<ReturnType>) {
            std::optional<ReturnType> result = std::nullopt;
            (try_apply<Func, ReturnType, Is>(func, matched, result), ...);
            assert(result.has_value());
            ReturnType realsult = std::move(result.value());
            if constexpr (std::is_reference_v<_ReturnType>) {
                return realsult.get();
            } else {
                return realsult;
            }
        } else {
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
