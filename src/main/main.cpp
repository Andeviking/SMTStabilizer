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

#include <omp.h>

#include <ctime>
#include <filesystem>
#include <ios>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "kernel/kernel.h"
#include "node/node_manager.h"
#include "option/option.h"
#include "solver/bitwuzla/bitwuzla.h"

#ifdef SMTSTABILIZER_HAVE_XGBOOST
#include <xgboost/c_api.h>
#endif
#ifdef SMTSTABILIZER_HAVE_BITWUZLA
#include "bitwuzla/cpp/bitwuzla.h"

#endif

#ifdef SMTSTABILIZER_HAVE_XGBOOST
static void xgboost_smoke_test() {
    int major = 0;
    int minor = 0;
    int patch = 0;
    XGBoostVersion(&major, &minor, &patch);

    std::cerr << "[xgboost] detected version " << major << "." << minor << "." << patch << std::endl;
}

std::filesystem::path get_model_dir() {
#if defined(__linux__)
    std::error_code ec;
    const auto exe_path = std::filesystem::read_symlink("/proc/self/exe", ec);
    if (!ec && !exe_path.empty()) {
        return exe_path.parent_path() / "model";
    }
#endif
    return std::filesystem::current_path() / "model";
}

bool predict(const std::string &model_path, DMatrixHandle dmat) {
    BoosterHandle booster = nullptr;
    const float missing = std::numeric_limits<float>::quiet_NaN();

    auto cleanup = [&]() {
        if (booster != nullptr) {
            XGBoosterFree(booster);
            booster = nullptr;
        }
    };

    auto ensure_ok = [&](int code, const std::string &message) {
        if (code != 0) {
            std::string error = message;
            error += ": ";
            error += XGBGetLastError();
            cleanup();
            throw std::runtime_error(error);
        }
    };

    const DMatrixHandle dmats[] = {dmat};
    ensure_ok(XGBoosterCreate(dmats, 1, &booster), "Failed to create XGBoost booster");
    ensure_ok(XGBoosterLoadModel(booster, model_path.c_str()), "Failed to load XGBoost model");

    const char *config = R"({"type":0,"training":false,"iteration_begin":0,"iteration_end":0,"strict_shape":false})";
    const bst_ulong *out_shape = nullptr;
    bst_ulong out_dim = 0;
    const float *out_result = nullptr;
    ensure_ok(XGBoosterPredictFromDMatrix(booster, dmat, config, &out_shape, &out_dim, &out_result),
              "Failed to run XGBoost prediction");

    if (out_result == nullptr || out_dim == 0) {
        cleanup();
        throw std::runtime_error("XGBoost returned an empty prediction result");
    }

    float prediction = out_result[0];
    cleanup();
    return prediction >= 0.5;
}

#endif

int main(int argc, char *argv[]) {
    omp_set_num_threads(1);

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
#ifdef SMTSTABILIZER_HAVE_XGBOOST
            const float missing = std::numeric_limits<float>::quiet_NaN();
            DMatrixHandle dmat = nullptr;
            auto features = solver.features();
            XGDMatrixCreateFromMat(features.data(), 1, static_cast<bst_ulong>(features.size()), missing, &dmat);

            // xgboost_smoke_test();
            auto &options = solver.options();
            const auto model_dir = get_model_dir();
            std::string t_solver = predict((model_dir / "router" / "router.ubj").string(), dmat) ? "kissat" : "cadical";
            options.set(bitwuzla::Option::SAT_SOLVER, t_solver);
            if (t_solver == "kissat") {
                options.set(bitwuzla::Option::ABSTRACTION, false);
            }
            else
                options.set(bitwuzla::Option::ABSTRACTION, predict((model_dir / t_solver / (t_solver + "_abstraction.ubj")).string(), dmat));
            options.set(bitwuzla::Option::PP_VARIABLE_SUBST, predict((model_dir / t_solver / (t_solver + "_variable-substitution.ubj")).string(), dmat));
            bool normalize = predict((model_dir / t_solver / (t_solver + "_normalize.ubj")).string(), dmat);
            options.set(bitwuzla::Option::PP_NORMALIZE, normalize);
            if (!normalize)
                options.set(bitwuzla::Option::REWRITE_LEVEL, 1 + predict((model_dir / t_solver / (t_solver + "_rewrite-level.ubj")).string(), dmat));
            if (dmat != nullptr) {
                XGDMatrixFree(dmat);
                dmat = nullptr;
            }
#endif
            // auto &bzla_options = solver.options();
            // bzla_options.set(bitwuzla::Option::ABSTRACTION, options.get<bool>(stabilizer::option::Option::BZLA_ABSTRACTION));
            // bzla_options.set(bitwuzla::Option::PP_VARIABLE_SUBST, options.get<bool>(stabilizer::option::Option::BZLA_SUBST));
            // bzla_options.set(bitwuzla::Option::PP_NORMALIZE, options.get<bool>(stabilizer::option::Option::BZLA_NORMALIZE));
            // bzla_options.set(bitwuzla::Option::REWRITE_LEVEL, options.get<uint64_t>(stabilizer::option::Option::BZLA_REWRITE_LEVEL));
            // bzla_options.set(bitwuzla::Option::SAT_SOLVER, options.get<std::string>(stabilizer::option::Option::BZLA_SAT_SOLVER));

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
