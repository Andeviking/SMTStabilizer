// Copyright (c) 2026 XiangZhang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include "api/stabilizer_api.h"
#include "api/stabilizer_c_api.h"

namespace {

using stabilizer::api::SMTStabilizer;
using stabilizer::api::SMTStabilizerOptions;

std::string read_file(const std::filesystem::path &path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("unable to open test file: " + path.string());
    }
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

void expect(bool condition, const std::string &message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

std::filesystem::path test_root() {
    return std::filesystem::path(TEST_SOURCE_DIR);
}

void test_option_roundtrip() {
    SMTStabilizerOptions options;
    expect(options.get_rewrite(), "rewrite should default to true");
    expect(options.get_context_propagation(), "context propagation should default to true");
    expect(options.get_subgraph_pruning(), "subgraph pruning should default to true");

    options.set_rewrite(false);
    options.set_context_propagation(false);
    options.set_subgraph_pruning(false);

    expect(!options.get_rewrite(), "rewrite getter/setter mismatch");
    expect(!options.get_context_propagation(), "context propagation getter/setter mismatch");
    expect(!options.get_subgraph_pruning(), "subgraph pruning getter/setter mismatch");
}

void test_apply_file_and_text_match() {
    const auto input_path = test_root() / "inputs" / "rewrite_input.smt2";
    const auto input = read_file(input_path);

    SMTStabilizer stabilizer;
    const std::string file_output = stabilizer.apply_file(input_path.string());
    const std::string text_output = stabilizer.apply_text(input);

    expect(!file_output.empty(), "file output should not be empty");
    expect(file_output == text_output, "apply_file and apply_text should produce identical output");
    expect(file_output.find("(check-sat)") != std::string::npos, "output should contain check-sat");
    expect(file_output.find("(exit)") != std::string::npos, "output should contain exit");
}

void test_rewrite_toggle_smoke() {
    const auto input_path = test_root() / "inputs" / "rewrite_input.smt2";
    const auto input = read_file(input_path);

    SMTStabilizerOptions rewrite_on;
    rewrite_on.set_rewrite(true);
    SMTStabilizer with_rewrite(rewrite_on);
    const std::string rewritten = with_rewrite.apply_text(input);

    SMTStabilizerOptions rewrite_off;
    rewrite_off.set_rewrite(false);
    SMTStabilizer without_rewrite(rewrite_off);
    const std::string raw = without_rewrite.apply_text(input);

    expect(!rewritten.empty(), "rewrite-on output should not be empty");
    expect(!raw.empty(), "rewrite-off output should not be empty");
    expect(rewritten.find("(check-sat)") != std::string::npos, "rewrite-on output should contain check-sat");
    expect(raw.find("(check-sat)") != std::string::npos, "rewrite-off output should contain check-sat");
}

void test_flag_matrix_smoke() {
    const auto input_path = test_root() / "inputs" / "symmetric_input.smt2";
    const auto input = read_file(input_path);

    for (int mask = 0; mask < 8; ++mask) {
        SMTStabilizerOptions options;
        options.set_rewrite((mask & 1) != 0);
        options.set_context_propagation((mask & 2) != 0);
        options.set_subgraph_pruning((mask & 4) != 0);

        SMTStabilizer stabilizer(options);
        const std::string output = stabilizer.apply_text(input);
        expect(!output.empty(), "smoke output should not be empty");
        expect(output.find("(set-logic") != std::string::npos, "smoke output should contain set-logic");
        expect(output.find("(assert") != std::string::npos, "smoke output should contain assertions");
        expect(output.find("(check-sat)") != std::string::npos, "smoke output should contain check-sat");
    }
}

void test_cpp_error_paths() {
    SMTStabilizer stabilizer;
    bool caught_empty = false;
    try {
        (void)stabilizer.apply_text("");
    }
    catch (const std::invalid_argument &) {
        caught_empty = true;
    }
    expect(caught_empty, "empty text should throw invalid_argument");

    bool caught_bad_file = false;
    try {
        (void)stabilizer.apply_file("/definitely/not/a/real/file.smt2");
    }
    catch (const std::runtime_error &) {
        caught_bad_file = true;
    }
    expect(caught_bad_file, "bad file path should throw runtime_error");
}

void test_c_api() {
    auto *options = stabilizer_options_create();
    expect(options != nullptr, "stabilizer_options_create returned null");
    stabilizer_options_set_rewrite(options, true);
    stabilizer_options_set_context_propagation(options, false);
    stabilizer_options_set_subgraph_pruning(options, true);

    expect(stabilizer_options_get_rewrite(options), "C option rewrite mismatch");
    expect(!stabilizer_options_get_context_propagation(options), "C option context propagation mismatch");
    expect(stabilizer_options_get_subgraph_pruning(options), "C option subgraph pruning mismatch");

    auto *handle = stabilizer_create(options);
    expect(handle != nullptr, "stabilizer_create returned null");

    expect(stabilizer_get_rewrite(handle), "C handle rewrite mismatch");
    expect(!stabilizer_get_context_propagation(handle), "C handle context propagation mismatch");
    expect(stabilizer_get_subgraph_pruning(handle), "C handle subgraph pruning mismatch");

    const auto input_path = test_root() / "inputs" / "rewrite_input.smt2";
    char *output = nullptr;
    const stabilizer_status status = stabilizer_apply_file(handle, input_path.string().c_str(), &output);
    expect(status == STABILIZER_STATUS_OK, "C apply_file should succeed");
    expect(output != nullptr, "C apply_file should produce output");
    expect(std::string(output).find("(check-sat)") != std::string::npos, "C output should contain check-sat");
    stabilizer_free_string(output);

    output = nullptr;
    const stabilizer_status bad_status = stabilizer_apply_text(handle, "", &output);
    expect(bad_status == STABILIZER_STATUS_INVALID_ARGUMENT, "empty C text should return invalid argument");
    expect(output == nullptr, "failed C apply should not produce output");
    expect(std::string(stabilizer_last_error(handle)).size() > 0, "last error should be populated");

    stabilizer_destroy(handle);
    stabilizer_options_destroy(options);
}

}  // namespace

int main() {
    try {
        test_option_roundtrip();
        test_apply_file_and_text_match();
        test_rewrite_toggle_smoke();
        test_flag_matrix_smoke();
        test_cpp_error_paths();
        test_c_api();
    }
    catch (const std::exception &e) {
        std::cerr << "API test failure: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "API tests passed" << std::endl;
    return EXIT_SUCCESS;
}