#pragma once
#include <cstddef>
#include <vector>

// #include "node/node.h"
#include "node/node_manager.h"
namespace stabilizer::kernel {
class Kernel {
  public:
    Kernel() = delete;
    Kernel(node::NodeManager &nm);  //[TODO] convert declare-fun to declare-const

    void apply(node::NodeManager &nm);

  private:
    std::vector<std::vector<size_t>> d_graph;
    std::vector<std::vector<std::pair<size_t, size_t>>> d_reversed_graph;
    std::vector<size_t> d_hash_table;
    std::vector<size_t> d_context_hash;
    std::vector<node::Node> d_nodes;  // reversed topological order
    std::vector<size_t> d_processing;

    std::vector<size_t> d_symbols;
    std::vector<size_t> d_unique_symbols;
    // std::vector<size_t> d_propagate_num;

    std::vector<uint8_t> d_visited;
    std::vector<uint8_t> d_is_commutative;
    std::vector<uint8_t> d_is_symbol;

    // std::unordered_map<size_t, size_t> d_spe_hash_count;

    size_t d_symbol_num;

    bool is_commutative(const size_t &i, const bool &from_cache = true);
    // void context_propagate();
    void context_propagate(const std::vector<size_t> &processing, const std::vector<size_t> &symbols);
    void specific_propagate();
    void rebuild_graph();  // graph symbols
    void sort_propagate(std::unordered_map<std::string, node::Sort> &sort_key_map, std::vector<std::vector<parser::Parser::DTTypeDecl>> &datatype_blocks);
    // void propagate();
    void propagate(const std::vector<size_t> &processing);
};
}  // namespace stabilizer::kernel

// #pragma once
// #include <cstddef>
// #include <vector>

// #include "node/node_manager.h"
// #include "util/node_helper.h"
// namespace stabilizer::kernel {
// class Kernel {
//   public:
//     Kernel(const node::NodeManager &nm);
//     ~Kernel();

//     bool done();

//     void apply();

//   private:
//     std::vector<std::vector<size_t>> d_graph;
//     std::vector<std::vector<std::pair<size_t, size_t>>> d_reversed_graph;
//     std::vector<size_t> d_hash_table;
//     std::vector<size_t> d_context_hash;
//     std::vector<size_t> d_processing;

//     std::vector<size_t> d_symbols;
//     std::vector<size_t> d_unique_symbols;
//     // std::vector<size_t> d_propagate_num;

//     std::vector<bool> d_visited;
//     std::vector<bool> d_is_commutative;
//     std::vector<bool> d_is_symbol;

//     size_t d_symbol_num;

//     bool is_commutative(const size_t &i, const bool &from_cache = true);
//     void context_propagate(const std::vector<size_t> &processing, const std::vector<size_t> &symbols);
//     void specific_propagate();
//     void rebuild_graph();  // graph symbols
//     void propagate(const std::vector<size_t> &processing);
// };
// }  // namespace stabilizer::kernel