#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "bitwuzla/cpp/bitwuzla.h"
#include "node/node_manager.h"
#include "parser/dag.h"
#include "solver/smt_solver.h"

namespace stabilizer::solver {
constexpr std::array<bitwuzla::Kind, 40> GRAMMATICAL_CONSTRUCT_LIST_FULL = {
    bitwuzla::Kind::CONSTANT,
    bitwuzla::Kind::VALUE,
    bitwuzla::Kind::VARIABLE,
    bitwuzla::Kind::AND,
    bitwuzla::Kind::DISTINCT,
    bitwuzla::Kind::EQUAL,
    bitwuzla::Kind::IMPLIES,
    bitwuzla::Kind::NOT,
    bitwuzla::Kind::OR,
    bitwuzla::Kind::XOR,
    bitwuzla::Kind::ITE,
    bitwuzla::Kind::APPLY,
    bitwuzla::Kind::LAMBDA,
    bitwuzla::Kind::BV_ADD,
    bitwuzla::Kind::BV_AND,
    bitwuzla::Kind::BV_ASHR,
    bitwuzla::Kind::BV_COMP,
    bitwuzla::Kind::BV_CONCAT,
    bitwuzla::Kind::BV_MUL,
    bitwuzla::Kind::BV_NEG,
    bitwuzla::Kind::BV_NOT,
    bitwuzla::Kind::BV_OR,
    bitwuzla::Kind::BV_SDIV,
    bitwuzla::Kind::BV_SHL,
    bitwuzla::Kind::BV_SHR,
    bitwuzla::Kind::BV_SLE,
    bitwuzla::Kind::BV_SLT,
    bitwuzla::Kind::BV_SMOD,
    bitwuzla::Kind::BV_SREM,
    bitwuzla::Kind::BV_SUB,
    bitwuzla::Kind::BV_UDIV,
    bitwuzla::Kind::BV_ULE,
    bitwuzla::Kind::BV_ULT,
    bitwuzla::Kind::BV_UREM,
    bitwuzla::Kind::BV_XOR,
    bitwuzla::Kind::BV_EXTRACT,
    bitwuzla::Kind::BV_ROLI,
    bitwuzla::Kind::BV_RORI,
    bitwuzla::Kind::BV_SIGN_EXTEND,
    bitwuzla::Kind::BV_ZERO_EXTEND};
class Bitwuzla : public SmtSolver {
  public:
    Bitwuzla() : d_tm() {
        for (size_t i = 0; i < GRAMMATICAL_CONSTRUCT_LIST_FULL.size(); ++i) {
            d_features[GRAMMATICAL_CONSTRUCT_LIST_FULL.at(i)] = 0;
        }
    };
    // Bitwuzla(const bitwuzla::Options &options) : d_tm() {}

    ~Bitwuzla() override = default;

    void simplify() override;
    std::string check_sat() override;
    void add_assertion(const node::Node &assertion) override;
    bitwuzla::Options &options() { return d_options; }
    std::vector<float> features() const {
        std::vector<float> result(GRAMMATICAL_CONSTRUCT_LIST_FULL.size());
        for (size_t i = 0; i < d_features.size(); i++) {
            result[i] = d_features.at(GRAMMATICAL_CONSTRUCT_LIST_FULL.at(i));
        }
        return result;
    }

  private:
    bitwuzla::Term mk_term(const node::Node &node);
    bitwuzla::Sort mk_sort(const node::Sort &sort);
    std::vector<size_t> _features() const {
        std::vector<size_t> result(GRAMMATICAL_CONSTRUCT_LIST_FULL.size());
        for (size_t i = 0; i < d_features.size(); i++) {
            result[i] = d_features.at(GRAMMATICAL_CONSTRUCT_LIST_FULL.at(i));
        }
        return result;
    }

    bitwuzla::TermManager d_tm;
    std::vector<bitwuzla::Term> d_assertions;
    std::unordered_map<bitwuzla::Kind, size_t> d_features;
    bitwuzla::Options d_options;
    // bitwuzla::Bitwuzla d_solver;

    std::unordered_map<node::Node, bitwuzla::Term>
        d_cache;
};
}  // namespace stabilizer::solver