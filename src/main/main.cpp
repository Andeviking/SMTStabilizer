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

#include <ctime>
#include <ios>
#include <iostream>

#include "kernel/kernel.h"
#include "node/node_manager.h"
#include "option/option.h"
#ifdef SMTSTABILIZER_HAVE_BITWUZLA
#include "solver/bitwuzla/bitwuzla.h"
#endif

int main(int argc, char *argv[]) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    stabilizer::parser::Parser p;
    p.getOptions()->setKeepLet(false);
    p.getOptions()->setExpandFunctions(false);

    stabilizer::option::Options options;
    std::string file = options.parse_from_argv(argc, argv);

    p.getOptions()->setRewrite(options.get<bool>(stabilizer::option::Option::REWRITE));

    if (file != "<stdin>")
        p.parse(file);
    else
        p.parse("");

    stabilizer::node::NodeManager nm(p);

    nm.simplify_assertions();

    stabilizer::kernel::Kernel kernel(nm);
    kernel.apply(nm);

    if (options.get<bool>(stabilizer::option::Option::CHECK_SAT)) {
        if (options.get<std::string>(stabilizer::option::Option::SOLVER) == "bitwuzla") {
#ifdef SMTSTABILIZER_HAVE_BITWUZLA
            stabilizer::solver::Bitwuzla solver;

            for (const auto &assertion : nm.assertions()) {
                solver.add_assertion(assertion);
            }

            std::cout << solver.check_sat() << std::endl;
#else
            throw std::runtime_error("Bitwuzla support is not compiled in. Please ensure that the bundled Bitwuzla library is present and properly linked.");
#endif
        }
        else {
            throw std::runtime_error("Unsupported solver: " + options.get<std::string>(stabilizer::option::Option::SOLVER));
        }
    }
    else {
        std::cout << nm.to_string() << std::endl;
    }

    return 0;
}
