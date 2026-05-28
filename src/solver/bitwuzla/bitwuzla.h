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
class Bitwuzla : public SmtSolver {
  public:
    Bitwuzla() : d_tm() {};
    // Bitwuzla(const bitwuzla::Options &options) : d_tm() {}

    ~Bitwuzla() override = default;

    void simplify() override;
    std::string check_sat() override;
    void add_assertion(const node::Node &assertion) override;
    bitwuzla::Options &options() { return d_options; }
    std::vector<float> features() const {
        std::vector<float> result(d_features.size());
        for (size_t i = 0; i < d_features.size(); i++) {
            result[i] = d_features[i];
        }
        return result;
    }

  private:
    bitwuzla::Term mk_term(const node::Node &node);
    bitwuzla::Sort mk_sort(const node::Sort &sort);

    bitwuzla::TermManager d_tm;
    std::vector<bitwuzla::Term> d_assertions;
    std::array<size_t, static_cast<size_t>(bitwuzla::Kind::BV_ZERO_EXTEND) + 1> d_features = {};
    bitwuzla::Options d_options;
    // bitwuzla::Bitwuzla d_solver;

    std::unordered_map<node::Node, bitwuzla::Term>
        d_cache;
};
}  // namespace stabilizer::solver