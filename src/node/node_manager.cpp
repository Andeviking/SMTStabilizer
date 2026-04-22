#include "node_manager.h"

namespace stabilizer::node {
std::string NodeManager::to_string() {
    return m_parser.dumpSMT2();
}
}  // namespace stabilizer::node