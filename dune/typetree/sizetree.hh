// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_TYPETREE_SIZETREE_HH
#define DUNE_TYPETREE_SIZETREE_HH

#include <type_traits>
#include <utility>

#include <dune/common/typetraits.hh>
#include <dune/typetree/transformedtree.hh>

namespace Dune {
namespace TypeTree {

//! Class that adds a size function to a Node base class
template<class Node>
struct SizeNodeMixin
    : public Node
{
  template<class N,
    std::enable_if_t<Dune::IsInteroperable<N,Node>::value, int> = 0>
  explicit SizeNodeMixin(N&& node)
    : Node{std::forward<N>(node)}
  {}

  auto size() const { return static_cast<Node const&>(*this).degree(); }
};

// deduction guide
template<class Node>
SizeNodeMixin(Node&& node)
  -> SizeNodeMixin<std::decay_t<Node>>;


//! Add a `size` method to all tree nodes
template<class Tree>
auto makeSizeTree(const Tree& tree)
{
  return transformedTree(tree, [](auto&& node)
  {
    return SizeNodeMixin{std::forward<decltype(node)>(node)};
  });
}

} // namespace TypeTree
} //namespace Dune

#endif // DUNE_TYPETREE_SIZETREE_HH
