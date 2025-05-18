#ifndef SHADER_PARAM_H
#define SHADER_PARAM_H

#include <imgui.h>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

// Base template: false
template <typename T, typename Variant>
struct is_in_variant : std::false_type {};

// Specialization for std::variant
template <typename T, typename... Ts>
struct is_in_variant<T, std::variant<Ts...>> : std::disjunction<std::is_same<T, Ts>...> {};

// Concept using "requires"
template <typename T, typename Variant>
concept InVariant = is_in_variant<T, Variant>::value;


namespace ShaderParam {

enum class Widget {
    Slider,
    Field,
    Checkbox,
    Dropdown,
};

template <Widget W>
struct Feature;

template <Widget W>
struct Float;

template <Widget W>
struct Integer;

template <Widget W>
struct Choice;

template <Widget W>
struct Feature;


using ParamVariant = std::variant<
    Float<Widget::Slider>,
    Float<Widget::Field>,

    Integer<Widget::Slider>,
    Integer<Widget::Field>,

    Feature<Widget::Checkbox>,

    Choice<Widget::Dropdown>>;


struct Param {
    virtual void display() = 0;
    virtual ~Param() = default;

  protected:
    Param() = default;

  private:
    std::string name;
};

template <Widget W>
struct Float : public Param {
    static_assert(is_in_variant<Float<W>, ParamVariant>::value);

    Float(const std::string& name, const float& min, const float& max, const float& initial_value)
        : name(name), state(initial_value), min(min), max(max) {}

    void display() override;

  private:
    std::string name;
    float state;

    float min;
    float max;
};

template <Widget W>
struct Integer : public Param {
    static_assert(is_in_variant<Integer<W>, ParamVariant>::value);

    Integer(const std::string& name, const int& min, const int& max, const int& initial_value)
        : name(name), state(initial_value), min(min), max(max) {}

    void display() override;

  private:
    std::string name;
    int state;

    int min;
    int max;
};

template <Widget W>
struct Feature : public Param {
    static_assert(is_in_variant<Feature<W>, ParamVariant>::value);

    Feature(const std::string& name, const bool& initial_state) : name(name), state(initial_state) {}

    void display() override;

    template <typename P, typename... Args>
    void add_child(Args&&... args) {
        childs.push_back(std::make_unique<P>(args...));
    }

  private:
    std::string name;
    bool state;

    std::vector<std::unique_ptr<Param>> childs;
};

template <Widget W>
struct Choice : public Param {
    static_assert(is_in_variant<Choice<W>, ParamVariant>::value);

    Choice(const std::string& name, const size_t& initial_state_idx, const std::vector<std::string>& values)
        : name(name), state(initial_state_idx), values(values), childs(values.size()) {}

    void display() override;

    template <typename P, typename... Args>
    void add_child(size_t index, Args&&... args) {
        if (index >= values.size()) {
            throw std::runtime_error("Child index too high."); // TODO: replace by a warning and return 
        }
        childs.at(index).push_back(std::make_unique<P>(std::forward(args)...));
    }

    template <typename P, typename... Args>
    void add_child(std::string value, Args&&... args) {
        auto it = std::find(values.begin(), values.end(), value);
        if (it != values.end()) {
            childs.at(it - values.begin()).push_back(std::make_unique<P>(std::forward(args)...));
        } else {
            throw std::runtime_error("Item no found."); // TODO: replace by a warning and return 
        }
        
    }

  private:
    std::string name;
    size_t state;

    std::vector<std::string> values;
    std::vector<std::vector<std::unique_ptr<Param>>> childs;
};

template <typename P, typename... Args>
void add_child();

void window(std::vector<std::unique_ptr<Param>>& params);

}  // namespace ShaderParam

#endif
