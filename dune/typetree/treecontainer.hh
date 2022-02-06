// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_TYPETREE_TREECONTAINER_HH
#define DUNE_TYPETREE_TREECONTAINER_HH

#include <type_traits>
#include <utility>
#include <functional>

#include <dune/common/indices.hh>
#include <dune/typetree/childextraction.hh>
#include <dune/typetree/treepath.hh>
#include <dune/typetree/transformedtree.hh>
#include <dune/typetree/nodes.hh>

namespace Dune {
namespace TypeTree {
namespace Detail {

/**
  * \brief A simple lambda for creating default constructible values from a node
  *
  * This simply returns LeafToValue<Node>{} for a given Node. It's needed
  * because using a lambda expression in a using declaration is not allowed
  * because it's an unevaluated context.
  */
template<template<class Node> class LeafToValue>
struct LeafToDefaultConstructibleValue
{
  template<class Node>
  auto operator()(const Node& node) const
  {
    return LeafToValue<Node>{};
  }
};

} // namespace Detail


//! Wrap a tree and transform it into a vector backend
template<class Node>
class ContainerNodeMixin
    : public Node
{
public:
  template<class N,
    std::enable_if_t<Dune::IsInteroperable<N,Node>::value, int> = 0>
  explicit ContainerNodeMixin(N&& node)
    : Node{std::forward<N>(node)}
  {}

  template<class N = Node,
    std::enable_if_t<std::is_default_constructible_v<N>, int> = 0>
  explicit ContainerNodeMixin()
    : Node{}
  {}

  template<class T,
    std::enable_if_t<not std::is_same<T,Node>::value, int> = 0>
  explicit ContainerNodeMixin(const T& tree)
    : ContainerNodeMixin{}
  {
    resize(tree);
  }

  //! Access the tree childs using a tree-path (const access)
  template<class... T>
  const auto& operator[](const TypeTree::HybridTreePath<T...>& path) const
  {
    return TypeTree::child(static_cast<Node const&>(*this), path);
  }

  //! Access the tree childs using a tree-path (mutable access)
  template<class... T>
  auto& operator[](const TypeTree::HybridTreePath<T...>& path)
  {
    return TypeTree::child(static_cast<Node&>(*this), path);
  }

  //! Resize the node or check for correct size
  void resize(std::size_t size)
  {
    if constexpr(Node::hasStaticSize)
      assert(Node::degree() == size);
    else
      Node::resize(size);
  }

  template<class Tree,
    decltype((std::declval<Tree>().degree(), int{})) = 0>
  void resize(const Tree& tree)
  {
    resize(tree.degree());
  }
};

// deduction guide
template<class Node>
ContainerNodeMixin(Node&& node)
  -> ContainerNodeMixin<std::decay_t<Node>>;


template<class Value>
class LeafNodeContainer
{
public:
  template<class V,
    std::enable_if_t<Dune::IsInteroperable<V,Value>::value, int> = 0>
  explicit LeafNodeContainer(V&& value)
    : value_{std::forward<V>(value)}
  {}

  template<class V = Value,
    std::enable_if_t<std::is_default_constructible_v<V>, int> = 0>
  explicit LeafNodeContainer()
    : value_{}
  {}

  template<class Tree,
    std::enable_if_t<Tree::isLeaf, int> = 0>
  explicit LeafNodeContainer(const Tree&)
    : LeafNodeContainer{}
  {}

  //! Access the tree childs using a tree-path (const access)
  const auto& operator[](const TypeTree::HybridTreePath<>& path) const
  {
    return value_;
  }

  //! Access the tree childs using a tree-path (mutable access)
  auto& operator[](const TypeTree::HybridTreePath<>& path)
  {
    return value_;
  }

  template<class... Args>
  void resize(Args&&...) {}

private:
  Value value_;
};

// deduction guide
template<class Value>
LeafNodeContainer(Value&& value)
  -> LeafNodeContainer<std::decay_t<Value>>;


/** \addtogroup TypeTree
 *  \{
 */

/**
 * \brief Create container having the same structure as the given tree
 *
 * This class allows to create a nested hybrid container having the same structure
 * as a given type tree. Power nodes are represented as std::array's while composite
 * nodes are represented as Dune::TupleVector's. The stored values for the leaf nodes
 * are creating using a given predicate. For convenience the created container is
 * not returned directly. Instead, the returned object stores the container and
 * provides operator[] access using a HybridTreePath.
 *
 * \param tree The tree which should be mapper to a container
 * \param leafToValue A mapping `Node -> Value` used to generate the stored values
 *                    for the leaves
 *
 * \returns A container matching the tree structure
 */
template <class Tree, class LeafToValue>
auto makeTreeContainer(Tree const& tree, LeafToValue leafToValue)
{
  if constexpr(Tree::isLeaf) {
    using Value = std::decay_t<decltype(leafToValue(tree))>;
    return LeafNodeContainer<Value>{leafToValue(tree)};
  } else
    return transformedTree(tree, [leafToValue](auto&& /*sourceNode*/, auto&& targetNode) {
      using Node = std::decay_t<decltype(targetNode)>;
      if constexpr(Node::isLeaf)
        return leafToValue(std::forward<decltype(targetNode)>(targetNode));
      else
        return ContainerNodeMixin<Node>{std::forward<decltype(targetNode)>(targetNode)};
    });
}

/**
 * \brief Create container havin the same structure as the given tree
 *
 * This class allows to create a nested hybrid container having the same structure
 * as a given type tree. Power nodes are represented as std::array's while composite
 * nodes are represented as Dune::TupleVector's. The stored values for the leaf nodes
 * are of the given type Value. For convenience the created container is
 * not returned directly. Instead, the returned object stores the container and
 * provides operator[] access using a HybridTreePath.
 *
 * \tparam Value Type of the values to be stored for the leafs. Should be default constructible.
 * \param leafToValue A predicate used to generate the stored values for the leaves
 *
 * \returns A container matching the tree structure
 */
template<class Value, class Tree,
  std::enable_if_t<std::is_default_constructible_v<Value>, int> = 0>
auto makeTreeContainer(const Tree& tree)
{
  return makeTreeContainer(tree, [](const auto&) {return Value{};});
}

/**
 * \brief Alias to container type generated by makeTreeContainer for given tree type and uniform value type
 */
template<class Value, class Tree>
using UniformTreeContainer = decltype(makeTreeContainer<Value>(std::declval<const Tree&>()));

/**
 * \brief Alias to container type generated by makeTreeContainer for give tree type and when using LeafToValue to create values
 */
template<template<class Node> class LeafToValue, class Tree>
using TreeContainer = decltype(makeTreeContainer(std::declval<const Tree&>(), std::declval<Detail::LeafToDefaultConstructibleValue<LeafToValue>>()));

//! \} group TypeTree

} // namespace TypeTree
} // namespace Dune

#endif // DUNE_TYPETREE_TREECONTAINER_HH
