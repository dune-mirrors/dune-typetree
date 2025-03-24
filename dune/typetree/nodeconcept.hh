// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-GPL-2.0-only-with-PDELab-exception

#ifndef DUNE_TYPETREE_NODE_CONCEPT_HH
#define DUNE_TYPETREE_NODE_CONCEPT_HH

#include <dune/common/typetraits.hh>

#include <concepts>
#include <type_traits>
#include <utility>


namespace Dune::TypeTree::Concept {

  //!@brief Model of a leaf tree node of a typetree
  template<class Node>
  concept LeafTreeNode = requires
  {
    std::remove_cvref_t<Node>::degree();
    requires IsCompileTimeConstant<decltype(std::remove_cvref_t<Node>::degree())>::value;
    requires (std::remove_cvref_t<Node>::degree() == 0);
  };

  //!@brief Model of an inner tree node of a typetree
  template<class Node>
  concept InnerTreeNode = requires(Node node)
  {
      { std::as_const(node).degree() } -> std::convertible_to<std::size_t>;
      node.child(index_constant<0>());
  };

  //!@brief Model of an inner node of a typetree with compile time known degree
  template<class Node>
  concept StaticDegreeInnerNode = InnerTreeNode<Node> && requires(Node node)
  {
    std::remove_cvref_t<Node>::degree();
    requires IsCompileTimeConstant<decltype(std::remove_cvref_t<Node>::degree())>::value;
    requires (std::remove_cvref_t<Node>::degree() != 0);
    node.child(index_constant<(std::remove_cvref_t<Node>::degree()-1)>());
  };

  //!@brief Model of an inner node of a typetree with uniform nodes
  template<class Node>
  concept UniformInnerTreeNode = InnerTreeNode<Node> && requires(Node node, std::size_t index)
  {
      node.child(index);
  };

  //!@brief Model of a node of a typetree
  template<class Node>
  concept TreeNode = requires
  {
    requires InnerTreeNode<Node> || LeafTreeNode<Node>;
  };

} // namespace Dune::TypeTree::Concept

#endif // DUNE_TYPETREE_NODE_CONCEPT_HH
