#include "bitwuzla.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "bitwuzla/cpp/bitwuzla.h"
#include "parser/kind.h"
#include "parser/sort.h"

namespace stabilizer::solver {
// bool Bitwuzla::xgboost_predict(const std::string &model_path) const {
// #ifdef SMTSTABILIZER_HAVE_XGBOOST
//     std::vector<float> features;
//     features.reserve(d_features.size());
//     for (const auto &feature : d_features) {
//         features.push_back(static_cast<float>(feature));
//     }

//     DMatrixHandle dmat = nullptr;
//     BoosterHandle booster = nullptr;
//     const float missing = std::numeric_limits<float>::quiet_NaN();

//     auto cleanup = [&]() {
//         if (booster != nullptr) {
//             XGBoosterFree(booster);
//             booster = nullptr;
//         }
//         if (dmat != nullptr) {
//             XGDMatrixFree(dmat);
//             dmat = nullptr;
//         }
//     };

//     auto ensure_ok = [&](int code, const std::string &message) {
//         if (code != 0) {
//             std::string error = message;
//             error += ": ";
//             error += XGBGetLastError();
//             cleanup();
//             throw std::runtime_error(error);
//         }
//     };

//     ensure_ok(XGDMatrixCreateFromMat(features.data(), 1, static_cast<bst_ulong>(features.size()), missing, &dmat),
//               "Failed to create XGBoost DMatrix");

//     const DMatrixHandle dmats[] = {dmat};
//     ensure_ok(XGBoosterCreate(dmats, 1, &booster), "Failed to create XGBoost booster");
//     ensure_ok(XGBoosterLoadModel(booster, model_path.c_str()), "Failed to load XGBoost model");

//     const char *config = R"({"type":0,"training":false,"iteration_begin":0,"iteration_end":0,"strict_shape":false})";
//     const bst_ulong *out_shape = nullptr;
//     bst_ulong out_dim = 0;
//     const float *out_result = nullptr;
//     ensure_ok(XGBoosterPredictFromDMatrix(booster, dmat, config, &out_shape, &out_dim, &out_result),
//               "Failed to run XGBoost prediction");

//     if (out_result == nullptr || out_dim == 0) {
//         cleanup();
//         throw std::runtime_error("XGBoost returned an empty prediction result");
//     }

//     float prediction = out_result[0];
//     cleanup();
//     return prediction >= 0.5;
// #else
//     (void)model_path;
//     throw std::runtime_error("XGBoost support is not compiled in");
// #endif
// }

void Bitwuzla::simplify() {
    // d_solver.simplify();
}

void Bitwuzla::add_assertion(const node::Node &assertion) {
    std::vector<node::Node> visit{assertion};
    while (!visit.empty()) {
        auto node = visit.back();
        auto [it, success] = d_cache.emplace(node, bitwuzla::Term());
        if (success) {
            for (const auto &child : node->getChildren()) {
                visit.push_back(child);
            }
            continue;
        }
        else if (it->second.is_null()) {
            it->second = mk_term(node);
        }
        visit.pop_back();
    }
    d_assertions.emplace_back(d_cache.at(assertion));
    d_features.at(static_cast<size_t>(d_assertions.back().kind()))++;
    // d_solver.assert_formula(d_cache.at(assertion));
}

bitwuzla::Sort Bitwuzla::mk_sort(const node::Sort &sort) {
    using namespace stabilizer::parser;
    switch (sort->kind) {
        case SORT_KIND::SK_BOOL:
            return d_tm.mk_bool_sort();
        case SORT_KIND::SK_BV:
            return d_tm.mk_bv_sort(sort->getBitWidth());
        case SORT_KIND::SK_INTOREAL:
            return d_tm.mk_bv_sort(64);  // treat int-or-real as 64-bit bit-vector for simplicity
        default:
            throw std::runtime_error("Unsupported sort kind in Bitwuzla translation: " + sort->toString());
    }
}

bitwuzla::Term Bitwuzla::mk_term(const node::Node &node) {
    using namespace stabilizer::parser;
    auto children = node->getChildren();
    std::vector<bitwuzla::Term> args(children.size());
    for (size_t i = 0; i < children.size(); i++) {
        args[i] = d_cache.at(children[i]);
        d_features.at(static_cast<size_t>(args[i].kind()))++;
    }

    auto sort = mk_sort(node->getSort());
    auto name = node->getName();

    switch (node->getKind()) {
        case NODE_KIND::NT_CONST:
            if (name.starts_with("#b")) {
                name.erase(0, 2);
                return d_tm.mk_bv_value(sort, name);
            }
            else {
                return d_tm.mk_bv_value(sort, name, 10);
            }
        case NODE_KIND::NT_VAR:
            return d_tm.mk_const(sort);
        case NODE_KIND::NT_CONST_TRUE:
            return d_tm.mk_true();
        case NODE_KIND::NT_CONST_FALSE:
            return d_tm.mk_false();
        case NODE_KIND::NT_AND:
            return d_tm.mk_term(bitwuzla::Kind::AND, args);
        case NODE_KIND::NT_OR:
            return d_tm.mk_term(bitwuzla::Kind::OR, args);
        case NODE_KIND::NT_NOT:
            return d_tm.mk_term(bitwuzla::Kind::NOT, args);
        case NODE_KIND::NT_IMPLIES:
            return d_tm.mk_term(bitwuzla::Kind::IMPLIES, args);
        case NODE_KIND::NT_XOR:
            return d_tm.mk_term(bitwuzla::Kind::XOR, args);
        case NODE_KIND::NT_EQ:
            return d_tm.mk_term(bitwuzla::Kind::EQUAL, args);
        case NODE_KIND::NT_DISTINCT_BOOL:
        case NODE_KIND::NT_DISTINCT:
            return d_tm.mk_term(bitwuzla::Kind::DISTINCT, args);
        case NODE_KIND::NT_ITE:
            return d_tm.mk_term(bitwuzla::Kind::ITE, args);
        case NODE_KIND::NT_BV_NOT:
            return d_tm.mk_term(bitwuzla::Kind::BV_NOT, args);
        case NODE_KIND::NT_BV_NEG:
            return d_tm.mk_term(bitwuzla::Kind::BV_NEG, args);
        case NODE_KIND::NT_BV_AND:
            return d_tm.mk_term(bitwuzla::Kind::BV_AND, args);
        case NODE_KIND::NT_BV_OR:
            return d_tm.mk_term(bitwuzla::Kind::BV_OR, args);
        case NODE_KIND::NT_BV_XOR:
            return d_tm.mk_term(bitwuzla::Kind::BV_XOR, args);
        case NODE_KIND::NT_BV_NAND:
            return d_tm.mk_term(bitwuzla::Kind::BV_NAND, args);
        case NODE_KIND::NT_BV_NOR:
            return d_tm.mk_term(bitwuzla::Kind::BV_NOR, args);
        case NODE_KIND::NT_BV_XNOR:
            return d_tm.mk_term(bitwuzla::Kind::BV_XNOR, args);
        case NODE_KIND::NT_BV_COMP:
            return d_tm.mk_term(bitwuzla::Kind::BV_COMP, args);
        case NODE_KIND::NT_BV_ADD:
            return d_tm.mk_term(bitwuzla::Kind::BV_ADD, args);
        case NODE_KIND::NT_BV_SUB:
            return d_tm.mk_term(bitwuzla::Kind::BV_SUB, args);
        case NODE_KIND::NT_BV_MUL:
            return d_tm.mk_term(bitwuzla::Kind::BV_MUL, args);
        case NODE_KIND::NT_BV_UDIV:
            return d_tm.mk_term(bitwuzla::Kind::BV_UDIV, args);
        case NODE_KIND::NT_BV_SDIV:
            return d_tm.mk_term(bitwuzla::Kind::BV_SDIV, args);
        case NODE_KIND::NT_BV_UREM:
            return d_tm.mk_term(bitwuzla::Kind::BV_UREM, args);
        case NODE_KIND::NT_BV_SREM:
            return d_tm.mk_term(bitwuzla::Kind::BV_SREM, args);
        case NODE_KIND::NT_BV_SMOD:
            return d_tm.mk_term(bitwuzla::Kind::BV_SMOD, args);
        case NODE_KIND::NT_BV_SHL:
            return d_tm.mk_term(bitwuzla::Kind::BV_SHL, args);
        case NODE_KIND::NT_BV_LSHR:
            return d_tm.mk_term(bitwuzla::Kind::BV_SHR, args);
        case NODE_KIND::NT_BV_ASHR:
            return d_tm.mk_term(bitwuzla::Kind::BV_ASHR, args);
        case NODE_KIND::NT_BV_ULT:
            return d_tm.mk_term(bitwuzla::Kind::BV_ULT, args);
        case NODE_KIND::NT_BV_ULE:
            return d_tm.mk_term(bitwuzla::Kind::BV_ULE, args);
        case NODE_KIND::NT_BV_UGT:
            return d_tm.mk_term(bitwuzla::Kind::BV_UGT, args);
        case NODE_KIND::NT_BV_UGE:
            return d_tm.mk_term(bitwuzla::Kind::BV_UGE, args);
        case NODE_KIND::NT_BV_SLT:
            return d_tm.mk_term(bitwuzla::Kind::BV_SLT, args);
        case NODE_KIND::NT_BV_SLE:
            return d_tm.mk_term(bitwuzla::Kind::BV_SLE, args);
        case NODE_KIND::NT_BV_SGT:
            return d_tm.mk_term(bitwuzla::Kind::BV_SGT, args);
        case NODE_KIND::NT_BV_SGE:
            return d_tm.mk_term(bitwuzla::Kind::BV_SGE, args);
        case NODE_KIND::NT_BV_CONCAT:
            return d_tm.mk_term(bitwuzla::Kind::BV_CONCAT, args);
        case NODE_KIND::NT_BV_EXTRACT:
            return d_tm.mk_term(bitwuzla::Kind::BV_EXTRACT, {args[0]}, {std::stoull(args[1].value<std::string>(10)), std::stoull(args[2].value<std::string>(10))});
        case NODE_KIND::NT_BV_REPEAT:
            return d_tm.mk_term(bitwuzla::Kind::BV_REPEAT, {args[0]}, {std::stoull(args[1].value<std::string>(10))});
        case NODE_KIND::NT_BV_ZERO_EXT:
            return d_tm.mk_term(bitwuzla::Kind::BV_ZERO_EXTEND, {args[0]}, {std::stoull(args[1].value<std::string>(10))});
        case NODE_KIND::NT_BV_SIGN_EXT:
            return d_tm.mk_term(bitwuzla::Kind::BV_SIGN_EXTEND, {args[0]}, {std::stoull(args[1].value<std::string>(10))});
        case NODE_KIND::NT_BV_ROTATE_LEFT:
            return d_tm.mk_term(bitwuzla::Kind::BV_ROLI, {args[0]}, {std::stoull(args[1].value<std::string>(10))});
        case NODE_KIND::NT_BV_ROTATE_RIGHT:
            return d_tm.mk_term(bitwuzla::Kind::BV_RORI, {args[0]}, {std::stoull(args[1].value<std::string>(10))});
        case NODE_KIND::NT_FUNC_APPLY:
            return d_tm.mk_term(bitwuzla::Kind::APPLY, args);
        case NODE_KIND::NT_FUNC_DEF:
            args.emplace_back(args.front());
            args.erase(args.begin());
            if (args.size() == 1) {
                return args[0];
            }
            return d_tm.mk_term(bitwuzla::Kind::LAMBDA, args);
        case NODE_KIND::NT_FUNC_PARAM:
            return d_tm.mk_var(sort, name);
        default:
            throw std::runtime_error("Unsupported node kind in Bitwuzla translation: " + kindToString(node->getKind()));
    }
}

std::string Bitwuzla::check_sat() {
// for (const auto &c : d_features) {
//     std::cout << c << ',';
// }
// std::cout << std::endl;
// exit(0);
#ifdef SMTSTABILIZER_HAVE_XGBOOST

#endif
    bitwuzla::Bitwuzla solver(d_tm, d_options);
    for (const auto &assertion : d_assertions) {
        solver.assert_formula(assertion);
    }

    auto result = solver.check_sat();
    if (result == bitwuzla::Result::SAT) {
        return "sat";
    }
    else if (result == bitwuzla::Result::UNSAT) {
        return "unsat";
    }
    else if (result == bitwuzla::Result::UNKNOWN) {
        return "unknown";
    }
    else {
        throw std::runtime_error("Unexpected result from Bitwuzla: " + std::to_string(static_cast<size_t>(result)));
    }
}
}  // namespace stabilizer::solver