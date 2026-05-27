#include <memory>
#include <unordered_map>

#include "bitwuzla/cpp/bitwuzla.h"
#include "node/node_manager.h"
#include "parser/dag.h"
#include "solver/smt_solver.h"

namespace stabilizer::solver {
class Bitwuzla : public SmtSolver {
  public:
    Bitwuzla() : d_tm(), d_options(), d_solver(d_tm, d_options) {}
    Bitwuzla(const bitwuzla::Options &options) : d_tm(), d_options(options), d_solver(d_tm, d_options) {}

    ~Bitwuzla() override = default;

    void simplify() override;
    std::string check_sat() override;
    void add_assertion(const node::Node &assertion) override;

  private:
    bitwuzla::Term mk_term(const node::Node &node);
    bitwuzla::Sort mk_sort(const node::Sort &sort);

    bitwuzla::TermManager d_tm;
    bitwuzla::Options d_options;
    bitwuzla::Bitwuzla d_solver;

    std::unordered_map<node::Node, bitwuzla::Term> d_cache;
};
}  // namespace stabilizer::solver