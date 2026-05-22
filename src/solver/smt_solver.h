
#include <string>

#include "node/node_manager.h"
namespace stabilizer::solver {
/**
 * The SMT solver interface.
 * @warning This interface is experimental and may change in future versions.
 */
class SmtSolver {
  public:
    /** Constructor. */
    SmtSolver() {}
    /** Destructor. */
    virtual ~SmtSolver() {}

    /** Simplify the assertions. */
    virtual void simplify() {};

    /** Check the satisfiability of the assertions.
     * @return "sat", "unsat", or "unknown". */
    virtual std::string check_sat() = 0;

    /** Add an assertion. */
    virtual void add_assertion(const node::Node &assertion) = 0;
};
}  // namespace stabilizer::solver
