// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-GPL-2.0-only-with-PDELab-exception

#ifndef DUNE_TYPETREE_COMPOSITENODE_HH
#define DUNE_TYPETREE_COMPOSITENODE_HH

#include <tuple>
#include <memory>
#include <type_traits>

#include <dune/typetree/nodetags.hh>
#include <dune/typetree/childextraction.hh>
#include <dune/typetree/typetraits.hh>

namespace Dune {
  namespace TypeTree {

    /** \addtogroup Nodes
     *  \ingroup TypeTree
     *  \{
     */

    //! Base class for composite nodes based on variadic templates.
    template<typename... Children>
    class CompositeNode
    {

    public:

      //! The type tag that describes a CompositeNode.
      typedef CompositeNodeTag NodeTag;

      //! The type used for storing the children.
      typedef std::tuple<std::shared_ptr<Children>... > NodeStorage;

      //! A tuple storing the types of all children.
      typedef std::tuple<Children...> ChildTypes;

      //! Mark this class as non leaf in the \ref TypeTree.
      static const bool isLeaf = false;

      //! Mark this class as a non power in the \ref TypeTree.
      static const bool isPower = false;

      //! Mark this class as a composite in the \ref TypeTree.
      static const bool isComposite = true;

      //! The number of children.
      [[deprecated("Will be removed after release 2.9. Use degree()")]]
      static const std::size_t CHILDREN = sizeof...(Children);

      static constexpr auto degree ()
      {
        return std::integral_constant<std::size_t,sizeof...(Children)>{};
      }

      //! Access to the type and storage type of the i-th child.
      template<std::size_t k>
      struct Child {

        static_assert((k < degree()), "child index out of range");

        //! The type of the child.
        typedef typename std::tuple_element<k,ChildTypes>::type Type;

        //! The type of the child.
        typedef typename std::tuple_element<k,ChildTypes>::type type;
      };

      //! @name Child Access
      //! @{

      //! Returns the k-th child.
      /**
       * \returns a reference to the k-th child.
       */
      template<std::size_t k>
      typename Child<k>::Type& child (index_constant<k> = {})
      {
        return *std::get<k>(_children);
      }

      //! Returns the k-th child (const version).
      /**
       * \returns a const reference to the k-th child.
       */
      template<std::size_t k>
      const typename Child<k>::Type& child (index_constant<k> = {}) const
      {
        return *std::get<k>(_children);
      }

      //! Returns the storage of the k-th child.
      /**
       * \returns a copy of the object storing the k-th child.
       */
      template<std::size_t k>
      std::shared_ptr<typename Child<k>::Type> childStorage (index_constant<k> = {})
      {
        return std::get<k>(_children);
      }

      //! Returns the storage of the k-th child (const version).
      /**
       * \returns a copy of the object storing the k-th child.
       */
      template<std::size_t k>
      std::shared_ptr<const typename Child<k>::Type> childStorage (index_constant<k> = {}) const
      {
        return std::get<k>(_children);
      }

      //! Sets the k-th child to the passed-in value.
      template<std::size_t k>
      void setChild (typename Child<k>::Type& child, index_constant<k> = {})
      {
        std::get<k>(_children) = stackobject_to_shared_ptr(child);
      }

      //! Store the passed value in k-th child.
      template<std::size_t k>
      void setChild (typename Child<k>::Type&& child, index_constant<k> = {})
      {
        std::get<k>(_children) = convert_arg(std::move(child));
      }

      //! Sets the storage of the k-th child to the passed-in value.
      template<std::size_t k>
      void setChild (std::shared_ptr<typename Child<k>::Type> child, index_constant<k> = {})
      {
        std::get<k>(_children) = std::move(child);
      }

      const NodeStorage& nodeStorage () const
      {
        return _children;
      }

      //! @}

      //! @name Nested Child Access
      //! @{

      // The following two methods require a little bit of SFINAE trickery to work correctly:
      // We have to make sure that they don't shadow the methods for direct child access because
      // those get called by the generic child() machinery. If that machinery picks up the methods
      // defined below, we have an infinite recursion.
      // So the methods make sure that either
      //
      // * there are more than one argument. In that case, we got multiple indices and can forward
      //   to the general machine.
      //
      // * the first argument is not a valid flat index, i.e. either a std::size_t or an index_constant.
      //   The argument thus has to be some kind of TreePath instance that we can also pass to the
      //   generic machine.
      //
      // The above SFINAE logic works, but there is still a problem with the return type deduction.
      // We have to do a lazy lookup of the return type after SFINAE has succeeded, otherwise the return
      // type deduction will trigger the infinite recursion.

      //! Returns the child given by the list of indices.
      /**
       * This method simply forwards to the freestanding function child(). See that
       * function for further information.
       */
#ifdef DOXYGEN
      template<typename... Indices>
      ImplementationDefined& child (Indices... indices)
#else
      template<typename I0, typename... I,
        std::enable_if_t<(sizeof...(I) > 0) || IsTreePath<I0>::value, int > = 0>
      decltype(auto) child (I0 i0, I... i)
#endif
      {
        static_assert(sizeof...(I) > 0 || impl::_non_empty_tree_path(I0{}),
          "You cannot use the member function child() with an empty TreePath, use the freestanding version child(node,treePath) instead."
          );
        return Dune::TypeTree::child(*this,i0,i...);
      }

      //! Returns the child given by the list of indices.
      /**
       * This method simply forwards to the freestanding function child(). See that
       * function for further information.
       */
#ifdef DOXYGEN
      template<typename... Indices>
      const ImplementationDefined& child (Indices... indices)
#else
      template<typename I0, typename... I,
        std::enable_if_t<(sizeof...(I) > 0) || IsTreePath<I0>::value, int > = 0>
      decltype(auto) child (I0 i0, I... i) const
#endif
      {
        static_assert(sizeof...(I) > 0 || impl::_non_empty_tree_path(I0{}),
          "You cannot use the member function child() with an empty TreePath, use the freestanding version child(node,treePath) instead."
          );
        return Dune::TypeTree::child(*this,i0,i...);
      }

      //! @}

    protected:

      //! @name Constructors
      //! @{

      //! Default constructor.
      /**
       * This constructor requires the storage type to be default
       * constructible.
       * \warning If the storage type is a pointer, the resulting object
       * will not be usable before its children are set using any of the
       * setChild(...) methods!
       */
      CompositeNode ()
      {}

      //! Initialize all children with the passed-in objects.
      template<typename... Args, typename = typename std::enable_if<(sizeof...(Args) == degree())>::type>
      CompositeNode (Args&&... args)
        : _children(convert_arg(std::forward<Args>(args))...)
      {}

      //! Initialize the CompositeNode with copies of the passed in Storage objects.
      CompositeNode (std::shared_ptr<Children>... children)
        : _children(std::move(children)...)
      {}

      //! Initialize the CompositeNode with a copy of the passed-in storage type.
      CompositeNode (const NodeStorage& children)
        : _children(children)
      {}

      //! @}

    private:
      NodeStorage _children;
    };

    //! \} group Nodes

  } // namespace TypeTree
} //namespace Dune

#endif // DUNE_TYPETREE_COMPOSITENODE_HH
