// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-GPL-2.0-only-with-PDELab-exception
#include <config.h>

#include <dune/common/test/testsuite.hh>

#include <dune/typetree/leafnode.hh>
#include <dune/typetree/powernode.hh>
#include <dune/typetree/compositenode.hh>
#include <dune/typetree/traversal.hh>



template<class P>
class SimpleLeafNode :
    public Dune::TypeTree::LeafNode
{
  using Base = Dune::TypeTree::LeafNode;
public:
  using Payload = P;

  SimpleLeafNode(const Payload& payload) :
    payload_(payload)
  {}

  SimpleLeafNode(Payload&& payload) :
    payload_(std::move(payload))
  {}

  const Payload& value() const
  { return payload_;}

  Payload& value()
  { return payload_;}

private:
  Payload payload_;
};



template<class P, class C, std::size_t n>
class SimplePowerNode:
    public Dune::TypeTree::PowerNode<C,n>
{
  using Base = Dune::TypeTree::PowerNode<C,n>;
public:
  using Payload = P;

  template<class... CC>
  SimplePowerNode(const Payload& payload, CC&&... cc) :
    Base(std::forward<CC>(cc)...),
    payload_(payload)
  {}

  template<class... CC>
  SimplePowerNode(Payload&& payload, CC&&... cc) :
    Base(std::forward<CC>(cc)...),
    payload_(std::move(payload))
  {}

  const Payload& value() const
  { return payload_;}

  Payload& value()
  { return payload_;}

private:
  Payload payload_;
};



template<class P, class... C>
class SimpleCompositeNode:
    public Dune::TypeTree::CompositeNode<C...>
{
  using Base = Dune::TypeTree::CompositeNode<C...>;
public:
  using Payload = P;

  template<class... CC>
  SimpleCompositeNode(const Payload& payload, CC&&... cc) :
    Base(std::forward<CC>(cc)...),
    payload_(payload)
  {}

  template<class... CC>
  SimpleCompositeNode(Payload&& payload, CC&&... cc) :
    Base(std::forward<CC>(cc)...),
    payload_(std::move(payload))
  {}

  const Payload& value() const
  { return payload_;}

  Payload& value()
  { return payload_;}

private:
  Payload payload_;
};



template<class P>
auto leafNode(P&& p)
{
  return SimpleLeafNode<std::decay_t<P>>(std::forward<P>(p));
}



template<class P, class... C>
auto compositeNode(P&& p, C&&... c)
{
  return SimpleCompositeNode<std::decay_t<P>, std::decay_t<C>...>(std::forward<P>(p), std::forward<C>(c)...);
}



template<class P, class C0, class... C>
auto powerNode(P&& p, C0&& c0, C&&... c)
{
  return SimplePowerNode<std::decay_t<P>, std::decay_t<C0>, sizeof...(C)+1>(std::forward<P>(p), std::forward<C0>(c0), std::forward<C>(c)...);
}


template<class PreOp = Dune::TypeTree::NoOp, class PostOp = Dune::TypeTree::NoOp, class LeafOp = Dune::TypeTree::NoOp>
struct GenericVisitor
    : Dune::TypeTree::TreeVisitor
    , Dune::TypeTree::DynamicTraversal
{
  GenericVisitor(PreOp const& preOp = {}, LeafOp const& leafOp = {}, PostOp const& postOp = {})
    : preOp_(preOp)
    , leafOp_(leafOp)
    , postOp_(postOp)
  {}

  template<class Node, class TreePath>
  void pre(Node&& node, TreePath tp) const { preOp_(node, tp); }

  template<class Node, class TreePath>
  void leaf(Node&& node, TreePath tp) const { leafOp_(node, tp); }

  template<class Node, class TreePath>
  void post(Node&& node, TreePath tp) const { postOp_(node, tp); }

private:
  PreOp preOp_;
  LeafOp leafOp_;
  PostOp postOp_;
};


int main()
{
  Dune::TestSuite test("tree traversal check");

  using Payload = std::size_t;

  auto tree = compositeNode(
                Payload(0),
                powerNode(
                  Payload(0),
                  leafNode(Payload(0)),
                  leafNode(Payload(0)),
                  leafNode(Payload(0))
                ),
                leafNode(Payload(0)));

  {
    std::size_t all = 0;
    forEachNode(tree, [&](auto&& node, auto&& path) {
      ++all;
    });
    test.check(all==6)
      << "Counting all nodes with forEachNode failed. Result is " << all << " but should be " << 6;
  }

  {
    std::size_t all = 0;
    auto countNode = [&](auto&& node, auto&& path) {
      ++all;
    };
    auto nop = [&](auto&& node, auto&& path) {};
    forEachNode(tree, countNode, countNode, nop);
    test.check(all==6)
      << "Counting all nodes with forEachNode failed. Result is " << all << " but should be " << 6;
  }

  {
    std::size_t all = 0;
    auto countNode = [&](auto&& node, auto&& path) {
      ++all;
    };
    auto nop = [&](auto&& node, auto&& path) {};
    forEachNode(tree, nop, countNode, countNode);
    test.check(all==6)
      << "Counting all nodes with forEachNode failed. Result is " << all << " but should be " << 6;
  }

  {
    std::size_t all = 0;
    auto countNode = [&](auto&& node, auto&& path) {
      ++all;
    };
    auto nop = [&](auto&& node, auto&& path) {};
    forEachNode(tree, nop, countNode, nop);
    test.check(all==4)
      << "Counting all nodes with forEachNode failed. Result is " << all << " but should be " << 4;
  }

  {
    std::size_t all = 0;
    auto countNode = [&](auto&& node, auto&& path) {
      ++all;
    };
    forEachNode(tree, countNode, countNode, countNode);
    test.check(all==8)
      << "Counting all nodes with forEachNode failed. Result is " << all << " but should be " << 8;
  }

  {
    std::size_t inner = 0;
    std::size_t leaf = 0;
    applyToTree(tree, GenericVisitor{
       [&](auto&& node, auto&& path) {
      ++inner;
    }, [&](auto&& node, auto&& path) {
      ++leaf;
    }});
    test.check(inner==2)
      << "Counting inner nodes with forEachNode failed. Result is " << inner << " but should be " << 2;
    test.check(leaf==4)
      << "Counting leaf nodes with forEachNode failed. Result is " << leaf << " but should be " << 4;
  }

  {
    std::size_t leaf = 0;
    forEachLeafNode(tree, [&](auto&& node, auto&& path) {
      ++leaf;
    });
    test.check(leaf==4)
      << "Counting leaf nodes with forEachLeafNode failed. Result is " << leaf << " but should be " << 4;
  }

  {
    auto countVisit = [] (auto&& node, auto&& path) {
      ++(node.value());
    };
    applyToTree(tree, GenericVisitor{countVisit, countVisit, countVisit});

    std::size_t visits=0;
    forEachNode(tree, [&](auto&& node, auto&& path) {
      visits += node.value();
    });

    test.check(visits==8)
      << "Counting all node visitations failed. Result is " << visits << " but should be " << 8;
  }

  return test.exit();
}
