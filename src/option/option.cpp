#include "option.h"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>

namespace stabilizer::option {

const std::array<OptionInfo, static_cast<size_t>(Option::NUM_OPTIONS)>
    Options::d_option_info = []() {
        std::array<OptionInfo, static_cast<size_t>(Option::NUM_OPTIONS)> arr{{
            OptionInfo{Option::HELP, "-h", "--help", "Print this help message", BoolInfo{false}},
            OptionInfo{Option::CHECK_SAT, "", "--check-sat", "Check satisfiability of the formula", BoolInfo{false}},
            OptionInfo{Option::REWRITE, "", "--rewrite", "Apply rewrite rules to the formula", BoolInfo{true}},
            OptionInfo{Option::SOLVER, "", "--solver", "Select the underlying SMT solver (e.g., 'bitwuzla')", ModeInfo{"bitwuzla"}},
            OptionInfo{Option::BZLA_ABSTRACTION, "", "--bzla-abstraction", "Apply Bitwuzla abstraction", BoolInfo{true}},
            OptionInfo{Option::BZLA_SUBST, "", "--bzla-subst", "Apply Bitwuzla substitution", BoolInfo{true}},
            OptionInfo{Option::BZLA_NORMALIZE, "", "--bzla-normalize", "Apply Bitwuzla normalization", BoolInfo{true}},
            OptionInfo{Option::BZLA_REWRITE_LEVEL, "", "--bzla-rewrite-level", "Set Bitwuzla rewrite level (0-2)", NumericInfo{2, 0, 2}},
            OptionInfo{Option::BZLA_SAT_SOLVER, "", "--bzla-sat-solver", "Select Bitwuzla SAT solver (e.g., 'cadical')", ModeInfo{"cadical", "kissat"}},
        }};
        std::sort(arr.begin(), arr.end(), [](const OptionInfo &a, const OptionInfo &b) {
            return a.option < b.option;
        });
        return arr;
    }();

const std::unordered_map<std::string, Option> Options::d_name2option = []() {
    std::unordered_map<std::string, Option> umap;
    for (size_t i = 0; i < static_cast<size_t>(Option::NUM_OPTIONS); ++i) {
        const OptionInfo &info = d_option_info[i];
        assert(i == static_cast<size_t>(info.option));
        if (!info.short_name.empty()) {
            umap[info.short_name] = static_cast<Option>(i);
        }
        if (!info.long_name.empty()) {
            umap[info.long_name] = static_cast<Option>(i);
        }
    }
    return umap;
}();

Options::Options() {
    for (size_t i = 0; i < static_cast<size_t>(Option::NUM_OPTIONS); ++i) {
        const OptionInfo &info = d_option_info.at(i);
        if (std::holds_alternative<BoolInfo>(info.info)) {
            d_values.at(i) = std::get<BoolInfo>(info.info).default_value;
        }
        else if (std::holds_alternative<NumericInfo>(info.info)) {
            d_values.at(i) = std::get<NumericInfo>(info.info).default_value;
        }
        else if (std::holds_alternative<ModeInfo>(info.info)) {
            const ModeInfo &mode_info = std::get<ModeInfo>(info.info);
            assert(!mode_info.modes.empty());
            d_values.at(i) = mode_info.modes.front();
        }
        else {
            throw std::logic_error("Unhandled option type");
        }
    }
}

Options &Options::operator=(const Options &other) {
    if (this != &other) {
        d_values = other.d_values;
    }
    return *this;
}

template <>
const bool &Options::get<bool>(Option option) const {
    size_t idx = static_cast<size_t>(option);
    const OptionInfo &info = d_option_info.at(idx);
    if (!std::holds_alternative<BoolInfo>(info.info)) {
        throw std::invalid_argument("Attempt to set non-boolean option: " + info.long_name);
    }
    return std::get<bool>(d_values.at(static_cast<size_t>(option)));
}

template <>
const uint64_t &Options::get<uint64_t>(Option option) const {
    size_t idx = static_cast<size_t>(option);
    const OptionInfo &info = d_option_info.at(idx);
    if (!std::holds_alternative<NumericInfo>(info.info)) {
        throw std::invalid_argument("Attempt to set non-boolean option: " + info.long_name);
    }
    return std::get<uint64_t>(d_values.at(static_cast<size_t>(option)));
}

template <>
const std::string &Options::get<std::string>(Option option) const {
    size_t idx = static_cast<size_t>(option);
    const OptionInfo &info = d_option_info.at(idx);
    if (!std::holds_alternative<ModeInfo>(info.info)) {
        throw std::invalid_argument("Attempt to set non-boolean option: " + info.long_name);
    }
    return std::get<std::string>(d_values.at(static_cast<size_t>(option)));
}

template <>
void Options::set<bool>(Option option, const bool &value) {
    size_t idx = static_cast<size_t>(option);
    const OptionInfo &info = d_option_info.at(idx);
    if (!std::holds_alternative<BoolInfo>(info.info)) {
        throw std::invalid_argument("Attempt to set non-boolean option: " + info.long_name);
    }
    d_values.at(idx) = value;
}

template <>
void Options::set<uint64_t>(Option option, const uint64_t &value) {
    size_t idx = static_cast<size_t>(option);
    const OptionInfo &info = d_option_info.at(idx);
    if (!std::holds_alternative<NumericInfo>(info.info)) {
        throw std::invalid_argument("Attempt to set non-numeric option: " + info.long_name);
    }
    const NumericInfo &num_info = std::get<NumericInfo>(info.info);
    if (value < num_info.min_value || value > num_info.max_value) {
        throw std::out_of_range("Value out of range for option " + info.long_name);
    }
    d_values.at(idx) = value;
}

template <>
void Options::set<std::string>(Option option, const std::string &value) {
    size_t idx = static_cast<size_t>(option);
    const OptionInfo &info = d_option_info.at(idx);
    if (!std::holds_alternative<ModeInfo>(info.info)) {
        throw std::invalid_argument("Attempt to set non-mode option: " + info.long_name);
    }
    const ModeInfo &mode_info = std::get<ModeInfo>(info.info);
    if (std::find(mode_info.modes.begin(), mode_info.modes.end(), value) == mode_info.modes.end()) {
        throw std::invalid_argument("Invalid mode value for option " + info.long_name + ": " + value);
    }
    d_values.at(idx) = value;
}

void Options::set_from_string(const std::string &name, const std::string &value) {
    auto it = d_name2option.find(name);
    if (it == d_name2option.end()) {
        throw std::invalid_argument("Unknown option name: " + name);
    }
    set_from_string(it->second, value);
}

void Options::set_from_string(const Option &option, const std::string &value) {
    if (is_boolean(option)) {
        if (value == "true" || value == "1") {
            set<bool>(option, true);
        }
        else if (value == "false" || value == "0") {
            set<bool>(option, false);
        }
        else {
            throw std::invalid_argument("Invalid boolean value for option " + to_string(option) +
                                        ": " + value);
        }
    }
    else if (is_numeric(option)) {
        try {
            uint64_t num_value = std::stoull(value);
            set<uint64_t>(option, num_value);  // range checked in set<uint64_t>
        }
        catch (const std::exception &e) {
            throw std::invalid_argument("Invalid numeric value for option " + to_string(option) +
                                        ": " + value);
        }
    }
    else if (is_mode(option)) {
        set<std::string>(option, value);  // membership checked in set<std::string>
    }
    else {
        throw std::logic_error("Unhandled option type for option " + to_string(option));
    }
}

std::string Options::parse_from_argv(int32_t argc, char *argv[]) {
    std::string option, file = "<stdin>";
    bool flag = false;

    auto is_bool_literal = [](const std::string &value) {
        return value == "true" || value == "false" || value == "1" || value == "0";
    };

    for (int32_t i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (!flag) {  // option
            if (arg.front() != '-') {
                // if (i != argc - 1) {
                //     throw std::invalid_argument("Input file must be the last argument, but got: " + arg);
                // }
                file = arg;
                continue;
            }
            auto pos = arg.find('=');
            if (pos != std::string::npos) {  // --option=value
                option = arg.substr(0, pos);
                std::string value = arg.substr(pos + 1);
                set_from_string(option, value);
                continue;
            }
            else {  // --option
                option = arg;
                flag = true;
                continue;
            }
        }
        else {  // value
            Option opt = str2option(option);
            if (arg.front() == '-') {
                if (is_boolean(opt)) {
                    // [TODO] handle --no-option
                    set_from_string(option, (option.size() > 5 && option.substr(0, 5) == "--no-") ? "false" : "true");
                    option = arg;
                    auto pos = arg.find('=');
                    if (pos != std::string::npos) {  // --option=value
                        option = arg.substr(0, pos);
                        std::string value = arg.substr(pos + 1);
                        set_from_string(option, value);
                        flag = false;
                        continue;
                    }
                    else {  // --option
                        flag = true;
                        continue;
                    }
                }
                else {
                    throw std::invalid_argument("Expected value for option " + option +
                                                ", but got option " + arg);
                }
            }
            if (is_boolean(opt) && !is_bool_literal(arg)) {
                // For trailing positional input, treat boolean option as implicit true.
                set_from_string(option, (option.size() > 5 && option.substr(0, 5) == "--no-") ? "false" : "true");
                if (i != argc - 1) {
                    throw std::invalid_argument("Input file must be the last argument, but got: " + arg);
                }
                file = arg;
                flag = false;
                continue;
            }
            set_from_string(option, arg);
            flag = false;
            continue;
        }
    }

    if (flag) {
        if (is_boolean(str2option(option))) {
            // [TODO] handle --no-option
            set_from_string(option, (option.size() > 5 && option.substr(0, 5) == "--no-") ? "false" : "true");
        }
        else {
            throw std::invalid_argument("Expected value for option " + option +
                                        ", but got nothing");
        }
    }

    return file;
}

void Options::print_help(std::ostream &out) const {
    std::ios_base::fmtflags original = out.flags();

    int name_width = 19;
    // const int type_width = 10;
    const int default_width = 12;
    const int description_width = 45;
    const std::string indent = "  ";
    // const int header_width = name_width + type_width + default_width + description_width;

    size_t max_short_name_len = 0;
    for (size_t i = 0; i < static_cast<size_t>(Option::NUM_OPTIONS); ++i) {
        max_short_name_len = std::max(max_short_name_len, d_option_info.at(i).short_name.size());
    }

    size_t max_name_len = std::string("<input>").size();
    for (size_t i = 0; i < static_cast<size_t>(Option::NUM_OPTIONS); ++i) {
        const OptionInfo &info = d_option_info.at(i);
        size_t cur_name_len = 0;
        if (!info.long_name.empty()) {
            cur_name_len = max_short_name_len + 2 + info.long_name.size();
            if (info.short_name.empty()) {
                cur_name_len = max_short_name_len + 2 + info.long_name.size();
            }
        }
        else {
            cur_name_len = info.short_name.size();
        }
        max_name_len = std::max(max_name_len, cur_name_len);
    }

    name_width = std::max(name_width, static_cast<int>(max_name_len + 1));
    const int header_width = name_width + default_width + description_width;

    auto wrap_text = [&](const std::string &text) {
        std::vector<std::string> lines;
        std::istringstream text_stream(text);
        std::string raw_line;
        bool has_any_content = false;

        while (std::getline(text_stream, raw_line)) {
            has_any_content = true;
            if (raw_line.empty()) {
                lines.emplace_back("");
                continue;
            }

            std::istringstream words(raw_line);
            std::string word;
            std::string line;
            while (words >> word) {
                if (line.empty()) {
                    line = word;
                }
                else if (static_cast<int>(line.size()) + 1 + static_cast<int>(word.size()) <= description_width) {
                    line += " " + word;
                }
                else {
                    lines.emplace_back(line);
                    line = word;
                }
            }
            if (!line.empty()) {
                lines.emplace_back(line);
            }
        }

        if (!has_any_content) {
            lines.emplace_back("");
        }

        return lines;
    };

    auto print_line = [&](const std::string &names, const std::string &type_str, const std::string &default_str, const std::string &desc) {
        const auto desc_lines = wrap_text(desc);
        bool first = true;
        for (const auto &desc_line : desc_lines) {
            out << indent << std::left << std::setw(name_width) << (first ? names : "");
            // out << std::left << std::setw(type_width) << (first ? type_str : "");
            out << std::left << std::setw(default_width) << (first ? default_str : "");
            out << std::left << std::setw(description_width) << desc_line << "\n";
            first = false;
        }
    };

    out << "Usage:\n"
        << indent << "XBV [<options>] [<input>]\n\n"
        << "Options:\n";

    out << indent << std::left << std::setw(name_width) << "option"
        // << std::left << std::setw(type_width) << "type"
        << std::left << std::setw(default_width) << "default"
        << std::left << std::setw(description_width) << "description"
        << "\n";
    out << indent << std::string(header_width, '-') << "\n";

    print_line("<input>", "  /", "[<stdin>]", "<stdin> to read from standard input or a file path as the last argument");

    for (size_t i = 0; i < static_cast<size_t>(Option::NUM_OPTIONS); ++i) {
        const OptionInfo &info = d_option_info.at(i);

        std::ostringstream names;
        if (!info.long_name.empty()) {
            if (info.short_name.empty()) {
                names << std::string(max_short_name_len + 2, ' ') << info.long_name;
            }
            else {
                names << std::left << std::setw(static_cast<int>(max_short_name_len)) << info.short_name;
                names << ", " << info.long_name;
            }
        }
        else {
            names << info.short_name;
        }

        std::string type_str;
        std::string default_str;
        std::string desc = info.description;

        if (std::holds_alternative<BoolInfo>(info.info)) {
            const BoolInfo &bool_info = std::get<BoolInfo>(info.info);
            type_str = "boolean";
            default_str = "[" + std::string(bool_info.default_value ? "true" : "false") + "]";
        }
        else if (std::holds_alternative<NumericInfo>(info.info)) {
            const NumericInfo &num_info = std::get<NumericInfo>(info.info);
            type_str = "numeric";
            default_str = "[" + std::to_string(num_info.default_value) + "]";
            desc += " (range [" + std::to_string(num_info.min_value) + ", " +
                    std::to_string(num_info.max_value) + "])";
        }
        else if (std::holds_alternative<ModeInfo>(info.info)) {
            const ModeInfo &mode_info = std::get<ModeInfo>(info.info);
            type_str = "mode";
            default_str = "[" + mode_info.modes.front() + "]";
            std::ostringstream detail_stream;
            detail_stream << "{";
            for (size_t j = 0; j < mode_info.modes.size(); ++j) {
                if (j != 0) {
                    detail_stream << ", ";
                }
                detail_stream << mode_info.modes[j];
            }
            detail_stream << "}";
            desc += " (" + detail_stream.str() + ")";
        }

        print_line(names.str(), type_str, default_str, desc);
    }

    out.flags(original);
}
}  // namespace stabilizer::option
