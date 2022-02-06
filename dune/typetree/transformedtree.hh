// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_TYPETREE_TRANSFORMEDTREE_HH
#define DUNE_TYPETREE_TRANSFORMEDTREE_HH

#include <type_traits>
#include <utility>

#include <dune/common/indices.hh>
#include <dune/typetree/nodes.hh>

namespace Dune {
namespace TypeTree {

//! Transformation of the nodes in a type-tree
template<class SourceNode, class NodeMap>
auto transformedTree(SourceNode const& sourceNode, NodeMap nodeMap)
{
  if constexpr(SourceNode::isLeaf)
    return nodeMap(sourceNode,sourceNode);
  else {
    // not leaf
    auto transformedNodeBase = [&]{
      auto childNode = [&](auto i) { return transformedTree(sourceNode.child(i), nodeMap); };
      if constexpr(SourceNode::isTypeUniform) {
        using SubTree = decltype(childNode(index_constant<0>{}));
        if constexpr(SourceNode::isUniform) {
          if constexpr(SourceNode::hasStaticSize)
            return StaticUniformPowerNode{SourceNode::degree(), childNode(0)};
          else
            return DynamicUniformPowerNode{SourceNode::degree(), childNode(0)};
        }
        else {
          // not uniform
          if constexpr(SourceNode::hasStaticSize) {
            return Dune::unpackIntegerSequence(
              [childNode](auto... ii) { return StaticPowerNode{childNode(ii)...}; },
              std::make_index_sequence<std::size_t(SourceNode::degree())>{});
          }
          else {
            DynamicPowerNode<SubTree> container;
            container.reserve(sourceNode.degree());
            for (std::size_t i = 0; i < sourceNode.degree(); ++i)
              container.emplace_back(childNode(i));
            return container;
          }
        }
      }
      else {
        // not type-uniform
        return Dune::unpackIntegerSequence(
          [childNode](auto... ii) { return CompositeNode{childNode(ii)...}; },
          std::make_index_sequence<std::size_t(SourceNode::degree())>());
      }
    };

    return nodeMap(sourceNode, transformedNodeBase());
  }
}

} // namespace TypeTree
} // namespace Dune

#endif // DUNE_TYPETREE_TRANSFORMEDTREE_HH
