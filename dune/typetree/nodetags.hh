// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_TYPETREE_NODETAGS_HH
#define DUNE_TYPETREE_NODETAGS_HH

#include <type_traits>

namespace Dune {
  namespace TypeTree {

    /** \addtogroup Nodes
     *  \ingroup TypeTree
     *  \{
     */

    //! Tag designating a leaf node.
    struct LeafNodeTag {};

    //! Tag designating a power node.
    struct PowerNodeTag {};

    //! Tag designating a power node with runtime degree.
    struct DynamicPowerNodeTag {};

    //! Tag designating a composite node.
    struct CompositeNodeTag {};

#ifndef DOXYGEN

    //! Special tag used as start value in algorithms.
    struct StartTag {};


    template<class N>
    struct NodeTagImpl
    {
      using type = std::conditional_t<
        N::isLeaf, LeafNodeTag, std::conditional_t<
        N::isPower && N::hasStaticSize, PowerNodeTag, std::conditional_t<
        N::isPower && !N::hasStaticSize, DynamicPowerNodeTag, std::conditional_t<
        N::isComposite, CompositeNodeTag, void>>>>;
    };

#endif // DOXYGEN

    //! \} group Nodes


  } // namespace TypeTree
} //namespace Dune

#endif // DUNE_TYPETREE_NODETAGS_HH
