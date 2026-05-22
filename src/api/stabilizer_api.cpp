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

#include "api/stabilizer_api.h"

#include <stdexcept>
#include <utility>

#include "kernel/kernel.h"
#include "node/node_manager.h"
#include "parser/parser.h"
#ifdef SMTSTABILIZER_HAVE_BITWUZLA
#include "solver/bitwuzla/bitwuzla.h"
#endif

namespace stabilizer::api {

void SMTStabilizerOptions::set_solver(const std::string &solver) {
    if (solver != "bitwuzla") {
        throw std::invalid_argument("unsupported solver: " + solver);
    }
    d_solver = solver;
}

SMTStabilizer::SMTStabilizer(SMTStabilizerOptions options) : d_options(std::move(options)) {}

const SMTStabilizerOptions &SMTStabilizer::options() const noexcept { return d_options; }

void SMTStabilizer::set_options(const SMTStabilizerOptions &options) noexcept { d_options = options; }

void SMTStabilizer::configure_parser(stabilizer::parser::Parser &parser, const SMTStabilizerOptions &options) {
    auto global_options = parser.getOptions();
    global_options->setKeepLet(false);
    global_options->setExpandFunctions(false);
    global_options->setRewrite(options.get_rewrite());
}

std::string SMTStabilizer::run_pipeline(stabilizer::parser::Parser &parser, const SMTStabilizerOptions &options) {
    configure_parser(parser, options);

    stabilizer::node::NodeManager nm(parser);
    nm.simplify_assertions();

    stabilizer::kernel::Kernel kernel(nm);
    kernel.apply(nm);

    if (options.get_check_sat()) {
        if (options.get_solver() != "bitwuzla") {
            throw std::invalid_argument("unsupported solver: " + options.get_solver());
        }
#ifdef SMTSTABILIZER_HAVE_BITWUZLA
        stabilizer::solver::Bitwuzla solver;
        for (const auto &assertion : nm.assertions()) {
            solver.add_assertion(assertion);
        }
        return solver.check_sat();
#else
        throw std::runtime_error("Bitwuzla support is not compiled in");
#endif
    }

    return nm.to_string();
}

std::string SMTStabilizer::apply_file(const std::string &file_path) const {
    if (file_path.empty()) {
        throw std::invalid_argument("file_path must not be empty");
    }

    stabilizer::parser::Parser parser;
    parser.parse(file_path);
    return run_pipeline(parser, d_options);
}

std::string SMTStabilizer::apply_text(const std::string &smt2_text) const {
    if (smt2_text.empty()) {
        throw std::invalid_argument("smt2_text must not be empty");
    }

    stabilizer::parser::Parser parser;
    parser.parseStr(smt2_text);
    return run_pipeline(parser, d_options);
}

}  // namespace stabilizer::api