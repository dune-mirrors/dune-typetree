// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_TYPETREE_TRANSFORMEDTREE_HH
#define DUNE_TYPETREE_TRANSFORMEDTREE_HH

#include <type_traits>
#include <utility>

#include <dune/common/indices.hh>
#include <dune/typetree/typetrees.hh>

namespace Dune {
namespace TypeTree {

//! Transformation of the nodes in a type-tree
template<class Tree, class MapNode>
auto transformedTree(Tree const& tree, MapNode mapNode)
{
  if constexpr(Tree::isLeaf)
    return mapNode(tree);
  else {
    // not leaf
    auto transformedNode = [&]{
      auto subTree = [&](auto i) { return transformedTree(tree[i], mapNode); };
      if constexpr(Tree::isTypeUniform) {
        using SubTree = decltype(subTree(index_constant<0>{}));
        if constexpr(Tree::isUniform) {
          if constexpr(Tree::isStatic)
            return StaticUniformPowerNode{Tree::degree(), subTree(0)};
          else
            return DynamicUniformPowerNode{Tree::degree(), subTree(0)};
        }
        else {
          // not uniform
          if constexpr(Tree::isStatic) {
            return Dune::unpackIntegerSequence(
              [subTree](auto... ii) { return StaticPowerNode{subTree(ii)...}; },
              std::make_index_sequence<std::size_t(Tree::degree())>{});
          }
          else {
            DynamicPowerNode<SubTree> container;
            container.reserve(tree.degree());
            for (std::size_t i = 0; i < tree.degree(); ++i)
              container.emplace_back(subTree(i));
            return container;
          }
        }
      }
      else {
        // not type-uniform
        return Dune::unpackIntegerSequence(
          [subTree](auto... ii) { return CompositeNode{subTree(ii)...}; },
          std::make_index_sequence<std::size_t(Tree::degree())>());
      }
    };

    return mapNode(transformedNode());
  }
}

} // namespace TypeTree
} // namespace Dune

#endif // DUNE_TYPETREE_TRANSFORMEDTREE_HH
