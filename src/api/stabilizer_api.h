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

#pragma once

#include <string>

namespace stabilizer::parser {
class Parser;
}

namespace stabilizer::api {

/**
 * @brief Public options for the SMTStabilizer API.
 *
 * This type mirrors the runtime options exposed by the command-line layer.
 * Its defaults match the CLI behavior: rewrite is enabled, check-sat is off,
 * and the solver mode defaults to bitwuzla.
 */
class SMTStabilizerOptions {
  public:
    SMTStabilizerOptions() = default;

    /** Enable or disable parser-side rewrite normalization. */
    void set_rewrite(bool rewrite) noexcept { d_rewrite = rewrite; }
    /** Return whether parser-side rewrite normalization is enabled. */
    bool get_rewrite() const noexcept { return d_rewrite; }

    /** Enable or disable satisfiability checking after stabilization. */
    void set_check_sat(bool check_sat) noexcept { d_check_sat = check_sat; }
    /** Return whether satisfiability checking is enabled. */
    bool get_check_sat() const noexcept { return d_check_sat; }

    /** Select the solver backend used when satisfiability checking is enabled. */
    void set_solver(const std::string &solver);
    /** Return the configured solver backend. */
    const std::string &get_solver() const noexcept { return d_solver; }

  private:
    bool d_rewrite = true;
    bool d_check_sat = false;
    std::string d_solver = "bitwuzla";
};

/**
 * @brief High-level facade for stabilizing SMT-LIB2 inputs.
 *
 * The facade accepts either a file path or an SMT-LIB2 script string, runs the
 * existing parser/node/kernel pipeline, and returns the normalized SMT2 text.
 *
 * API-to-kernel boundary:
 * - API methods validate input and instantiate parser state.
 * - run_pipeline creates node::NodeManager, disables parser-side keep-let and
 *   function expansion, applies the rewrite toggle, simplifies assertions, and
 *   then transfers control to kernel::Kernel::apply.
 * - If check-sat is enabled, the stabilized assertions are forwarded to the
 *   selected solver backend and the solver status string is returned.
 * - Kernel remains an internal implementation detail; the API exposes the
 *   runtime options used by the CLI and stable return types.
 *
 * Instances are cheap to copy and can be configured independently.
 */
class SMTStabilizer {
  public:
    explicit SMTStabilizer(SMTStabilizerOptions options = {});

    /** Return the current API options. */
    const SMTStabilizerOptions &options() const noexcept;
    /** Replace the current API options. */
    void set_options(const SMTStabilizerOptions &options) noexcept;

    /** Return whether satisfiability checking is enabled. */
    bool get_check_sat() const noexcept { return d_options.get_check_sat(); }
    /** Enable or disable satisfiability checking. */
    void set_check_sat(bool check_sat) noexcept { d_options.set_check_sat(check_sat); }

    /** Return the configured solver backend. */
    const std::string &get_solver() const noexcept { return d_options.get_solver(); }
    /** Select the solver backend used when satisfiability checking is enabled. */
    void set_solver(const std::string &solver) { d_options.set_solver(solver); }

    /**
     * @brief Apply the full pipeline to an SMT-LIB2 file.
     * @param file_path Path to an SMT-LIB2 input file.
     * @return Stabilized SMT2 text, or a solver status string when
     * check-sat is enabled.
     * @throws std::invalid_argument if the path is empty.
     * @throws parser::Parser or kernel-related exceptions if parsing or
     * normalization fails.
     */
    std::string apply_file(const std::string &file_path) const;

    /**
     * @brief Apply the full pipeline to an SMT-LIB2 script provided as text.
     * @param smt2_text Full SMT-LIB2 input text.
     * @return Stabilized SMT2 text, or a solver status string when
     * check-sat is enabled.
     * @throws std::invalid_argument if the text is empty.
     * @throws parser::Parser or kernel-related exceptions if parsing or
     * normalization fails.
     */
    std::string apply_text(const std::string &smt2_text) const;

  private:
    SMTStabilizerOptions d_options;

    /** Configure parser-global options before pipeline execution. */
    static void configure_parser(stabilizer::parser::Parser &parser, const SMTStabilizerOptions &options);

    /**
     * Run parser -> node manager -> kernel pipeline and serialize final output.
     */
    static std::string run_pipeline(stabilizer::parser::Parser &parser, const SMTStabilizerOptions &options);
};

}  // namespace stabilizer::api