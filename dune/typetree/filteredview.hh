// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_TYPETREE_FILTEREDTREE_HH
#define DUNE_TYPETREE_FILTEREDTREE_HH

#include <type_traits>
#include <utility>

#include <dune/common/indices.hh>
#include <dune/typetree/filteredtree.hh>
#include <dune/typetree/traversal.hh>

namespace Dune {
namespace TypeTree {
namespace Detail {

  template<bool b0, bool... bb, class Offset = index_constant<0>>
  constexpr auto mappedSeq(std::integer_sequence<bool,b0,bb...> mask, Offset offset = {})
  {
    auto next = index_constant<Offset::value+1>{};
    if constexpr(b0)
      return mergeSeq(offset, mappedSeq(std::integer_sequence<bool,bb...>{}, next));
    else
      return mappedSeq(mask, next);
  }

  template<std::size_t... ii>
  constexpr auto arraySeq(std::index_sequence<ii...> seq)
  {
    return Dune::unpackIndexSequence([&](auto... ii) {
      return std::array{std::size_t(ii)...};
    }, seq);
  }

} // end namespace Detail


template<class Node, class Predicate, class Path>
class FilteredNodeView
{
  template<class N>
  static auto makeIndexMapping(const N& node, Predicate pred, Path path)
  {
    if constexpr(Std::is_detected_v<Detail::DynamicTraversalConcept,N>) {
      if constexpr(N::hasStaticSize) {
        auto allIndices = std::make_index_sequence<std::size_t(N::degree())>{};
        auto filteredIndices = Dune::unpackIntegerSequence([&](auto... ii) {
          return Detail::filteredSeq(allIndices, pred(node[ii], push_back(path,ii))...);
        }, allIndices);
        return Detail::arraySeq(mappedIndices);
      }
      else {
        std::vector<std::size_t> filteredIndices;
        for (std::size_t i = 0; i < node.degree(); ++i)
          if (pred(node[ii], push_back(path,i)))
            filteredIndices.push_back(i);
        return filteredIndices;
      }
    }
    else {
      auto allIndices = std::make_index_sequence<std::size_t(N::degree())>{};
      auto filteredIndices = Dune::unpackIntegerSequence([&](auto... ii) {
        return Detail::filteredSeq(allIndices, pred(node[ii], push_back(path,ii))...);
      }, allIndices);
      return filteredIndices;
    }
  }

  using IndexMapping = decltype(makeIndexMapping(std::declval<Node>(), std::declval<Predicate>(), std::declval<Path>()));

public:
  FilteredNodeView(const Node& node, Predicate pred, Path path)
    : node_{node}
    , pred_{pred}
    , path_{path}
    , indexMapping_{makeIndexMapping(node,pred,path)}
  {}

  template<class Index>
  auto& child(const Index& i)
  {
    return node_.child(indexMapping_[i]);
  }

  template<class Index>
  const auto& child(const Index& i) const
  {
    return node_.child(indexMapping_[i]);
  }

  template<class N = Node,
    std::enabled_if_t<N::hasStaticSize, int> = 0>
  static constexpr auto degree()
  {
    return IndexMapping::size();
  }

  template<class N = Node,
    std::enabled_if_t<not N::hasStaticSize, int> = 0>
  auto degree() const
  {
    return indexMapping_.size();
  }

private:
  const Node& node_;
  Predicate pred_;
  Path path_;
  IndexMapping indexMapping_;
};

//! Filtering of nodes in a type-tree by a predicate `pred(Node, TreePath)`
template<class Tree, class Predicate, class Path = HybridTreePath<> >
auto filteredView(Tree const& tree, Predicate pred, Path path = {})
{
  if constexpr(Tree::isLeaf)
    return tree;
  else {
    // not leaf
    auto subTree = [&](auto i) { return filteredTree(tree[i], pred, push_back(path,i)); };
    if constexpr(Tree::isUniform) {
      // uniform nodes cannot be filtered
      if constexpr(Tree::isStatic)
        return StaticUniformPowerNode{Tree::degree(), subTree(0)};
      else
        return DynamicUniformPowerNode{Tree::degree(), subTree(0)};
    }
    else {
      // not uniform
      if constexpr(Tree::isStatic) {
        auto allIndices = std::make_index_sequence<std::size_t(Tree::degree())>{};
        auto filteredIndices = Dune::unpackIntegerSequence([&](auto... ii) {
          return Detail::filteredSeq(allIndices, pred(tree[ii], push_back(path,ii))...);
        }, allIndices);

        return Dune::unpackIntegerSequence([&](auto... ii) {
          if constexpr(Tree::isTypeUniform)
            return StaticPowerNode{subTree(ii)...};
          else
            return CompositeNode{subTree(ii)...};
        }, filteredIndices);
      }
      else {
        DynamicPowerNode<decltype(subTree(0))> container;
        for (std::size_t i = 0; i < tree.degree(); ++i)
          if (pred(tree[i], push_back(path,i)))
            container.emplace_back(subTree(i));
        return container;
      }
    }
  }
}

} // namespace TypeTree
} // namespace Dune

#endif // DUNE_TYPETREE_FILTEREDTREE_HH
