#include <memory>
#include <unordered_map>
#include <vector>
namespace stabilizer::node {
struct NodeInfo {};

class INode {
public:
protected:
private:
};

class Node {
public:
  Node(INode &node) : node(node) {}

private:
  INode &node;
};

class NodeManager {
public:
private:
  std::unordered_map<NodeInfo, size_t> d_hashtable;
  std::vector<std::unique_ptr<INode>> d_nodes;

  
};
} // namespace stabilizer::node