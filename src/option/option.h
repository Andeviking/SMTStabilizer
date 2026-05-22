#pragma once

#include <sys/types.h>

#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace stabilizer::option {
enum class Option {
    HELP,
    CHECK_SAT,
    REWRITE,
    SOLVER,
    NUM_OPTIONS
};

struct BoolInfo {
    bool default_value;
};

struct NumericInfo {
    uint64_t default_value;
    uint64_t min_value;
    uint64_t max_value;
};

struct ModeInfo {  // first is default
    std::vector<std::string> modes;
    template <typename... Ms>
    ModeInfo(Ms &&...ms)
        : modes{std::string(std::forward<Ms>(ms))...} {
    }
};

struct OptionInfo {
    Option option;
    std::string short_name;
    std::string long_name;
    std::string description;

    std::variant<BoolInfo, NumericInfo, ModeInfo> info;
};

class Options {
  public:
    Options();
    Options(const Options &other) : d_values(other.d_values) {};
    Options &operator=(const Options &other);
    ~Options() = default;

    // bool parse_from_string(const std::string& str);

    template <typename T>
    const T &get(Option option) const;

    template <typename T>
    void set(Option option, const T &value);

    void set_from_string(const std::string &name, const std::string &value);
    void set_from_string(const Option &option, const std::string &value);

    std::string parse_from_argv(int32_t argc, char *argv[]);

    bool is_boolean(Option option) const {
        const OptionInfo &info = d_option_info.at(static_cast<size_t>(option));
        return std::holds_alternative<BoolInfo>(info.info);
    }

    bool is_numeric(Option option) const {
        const OptionInfo &info = d_option_info.at(static_cast<size_t>(option));
        return std::holds_alternative<NumericInfo>(info.info);
    }

    bool is_mode(Option option) const {
        const OptionInfo &info = d_option_info.at(static_cast<size_t>(option));
        return std::holds_alternative<ModeInfo>(info.info);
    }

    std::string to_string(Option option, bool long_name = true) const {
        const OptionInfo &info = d_option_info.at(static_cast<size_t>(option));
        return long_name ? info.long_name : info.short_name;
    }

    Option str2option(const std::string &name) const {
        auto it = d_name2option.find(name);
        if (it == d_name2option.end()) {
            throw std::invalid_argument("Unknown option name: " + name);
        }
        return it->second;
    }

    void print_help(std::ostream &out) const;

  private:
    const static std::array<OptionInfo, static_cast<size_t>(Option::NUM_OPTIONS)> d_option_info;
    const static std::unordered_map<std::string, Option> d_name2option;

    std::array<std::variant<bool, uint64_t, std::string>, static_cast<size_t>(Option::NUM_OPTIONS)> d_values;
};

template <>
const bool &Options::get(Option option) const;
template <>
const uint64_t &Options::get(Option option) const;
template <>
const std::string &Options::get(Option option) const;

template <>
void Options::set(Option option, const bool &value);
template <>
void Options::set(Option option, const uint64_t &value);
template <>
void Options::set(Option option, const std::string &value);
}  // namespace stabilizer::option
