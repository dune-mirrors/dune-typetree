// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_TYPETREE_TRANSFORMTREE_HH
#define DUNE_TYPETREE_TRANSFORMTREE_HH

#include <type_traits>
#include <utility>

#include <dune/common/typetraits.hh>
#include <dune/typetree/transformedtree.hh>

namespace Dune {
namespace TypeTree {

//! Class that adds a data member and corresponding access methods to a Node base class
template<class Node, class Data>
struct DataNodeMixin
    : public Node
{
  template<class N, class D,
    std::enable_if_t<Dune::IsInteroperable<N,Node>::value, int> = 0,
    std::enable_if_t<Dune::IsInteroperable<D,Data>::value, int> = 0>
  DataNodeMixin(N&& node, D&& data)
    : Node{std::forward<N>(node)}
    , data_{std::forward<D>(data)}
  {}

  //! Access the tree childs using a tree-path (const access)
  template<class... T, class N = Node,
    std::enable_if_t<not N::isLeaf, int> = 0>
  const auto& operator[](const TypeTree::HybridTreePath<T...>& path) const
  {
    return TypeTree::child(static_cast<Node const&>(*this), path).data();
  }

  //! Access the tree childs using a tree-path (mutable access)
  template<class... T, class N = Node,
    std::enable_if_t<not N::isLeaf, int> = 0>
  auto& operator[](const TypeTree::HybridTreePath<T...>& path)
  {
    return TypeTree::child(static_cast<Node&>(*this), path).data();
  }

  const Data& data() const { return data_; }
  Data& data() { return data_; }

private:
  Data data_;
};

// deduction guide
template<class Node, class Data>
DataNodeMixin(Node&& node, Data&& data)
  -> DataNodeMixin<std::decay_t<Node>,std::decay_t<Data>>;


//! Add a `data` member to all tree nodes
template<class Data, class Tree,
  std::enable_if_t<std::is_default_constructible_v<Data>, int> = 0>
auto makeDataTree(const Tree& tree)
{
  return transformedTree(tree, [](auto&& /*source*/, auto&& node)
  {
    return DataNodeMixin{std::forward<decltype(node)>(node), Data{}};
  });
}

//! Add a `data` member to all tree nodes, constructed using the function `DataFactory(Node)`
template<class Tree, class DataFactory>
auto makeDataTree(const Tree& tree, const DataFactory& dataFactory)
{
  return transformedTree(tree, [dataFactory](auto const& node)
  {
    return DataNodeMixin{node, dataFactory(node)};
  });
}

} // namespace TypeTree
} //namespace Dune

#endif // DUNE_TYPETREE_TRANSFORMTREE_HH
