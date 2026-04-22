#pragma once
#include <vector>

#include "node/node_manager.h"
namespace stabilizer::util {

size_t hash_node(const node::Node& node);

void hash_combine(size_t& seed, const size_t& v);
void hash_children(size_t& seed, const std::vector<size_t>& children, const std::vector<size_t>& hash_table);
void hash_communative_children(size_t& seed, const std::vector<size_t>& children, const std::vector<size_t>& hash_table);
void hash_parents(size_t& seed, const std::vector<std::pair<size_t, size_t>>& parents, const std::vector<size_t>& hash_table);
}  // namespace stabilizer::util