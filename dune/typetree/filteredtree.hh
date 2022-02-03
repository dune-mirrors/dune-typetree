// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_TYPETREE_FILTEREDTREE_HH
#define DUNE_TYPETREE_FILTEREDTREE_HH

#include <type_traits>
#include <utility>

#include <dune/common/indices.hh>
#include <dune/typetree/typetrees.hh>

namespace Dune {
namespace TypeTree {
namespace Detail {

  template<std::size_t... ii, std::size_t... jj>
  constexpr auto mergeSeq(std::index_sequence<ii...>, std::index_sequence<jj...>)
  {
    return std::index_sequence<ii..., jj...>{};
  }

  template<std::size_t i0, std::size_t... ii, bool b0, bool... bb,
    std::enable_if_t<(sizeof...(ii) == sizeof...(bb)), int> = 0>
  constexpr auto filteredSeq(std::index_sequence<i0,ii...>, std::integer_sequence<bool,b0,bb...>)
  {
    auto tail = filteredSeq(std::index_sequence<ii...>{}, std::integer_sequence<bool,bb...>{});
    if constexpr(b0)
      return mergeSeq(std::index_sequence<i0>{}, tail);
    else
      return tail;
  }

  template<std::size_t i0, std::size_t... ii, bool b0, bool... bb,
    std::enable_if_t<(sizeof...(ii) == sizeof...(bb)), int> = 0>
  constexpr auto filteredSeq(std::index_sequence<i0,ii...> seq, std::bool_constant<b0>, std::bool_constant<bb>...)
  {
    return filteredSeq(seq, std::integer_sequence<bool,b0,bb...>{});
  }

} // end namespace Detail


//! Filtering of nodes in a type-tree by a predicate `pred(Node, TreePath)`
template<class Tree, class Predicate, class Path = HybridTreePath<> >
auto filteredTree(Tree const& tree, Predicate pred, Path path = {})
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
