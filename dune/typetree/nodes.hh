// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_TYPETREE_NODES_HH
#define DUNE_TYPETREE_NODES_HH

#include <array>
#include <cassert>
#include <vector>

#include <dune/common/indices.hh>
#include <dune/common/tuplevector.hh>
#include <dune/common/typelist.hh>
#include <dune/common/typeutilities.hh>
#include <dune/common/typetraits.hh>

namespace Dune {
namespace TypeTree {

  enum class Properties : int
  {
    IsLeaf        = 1,
    IsUniform     = 2,
    IsTypeUniform = 4,
    HasStaticSize = 8
  };

  // composition of flags
  constexpr Properties operator|(Properties lhs, Properties rhs)
  {
    return static_cast<Properties>(static_cast<int>(lhs) | static_cast<int>(rhs));
  }

  // test for flags
  constexpr bool isSet(Properties lhs, Properties rhs)
  {
    return static_cast<Properties>(static_cast<int>(lhs) & static_cast<int>(rhs)) == rhs;
  }

  template<Properties props>
  struct TreeProperties
  {
    inline static constexpr bool isLeaf = isSet(props, Properties::IsLeaf);
    inline static constexpr bool isUniform = isSet(props, Properties::IsUniform);
    inline static constexpr bool isTypeUniform = isSet(props, Properties::IsTypeUniform);
    inline static constexpr bool hasStaticSize = isSet(props, Properties::HasStaticSize);

    // for backwards compatibility
    inline static constexpr bool isPower = !isLeaf && isTypeUniform && !isUniform;
    inline static constexpr bool isComposite = !isLeaf && !isTypeUniform && !isUniform && hasStaticSize;
  };


  //! A type-tree that cannot be represented with the other tree types
  struct UnknownTypeTree
  {};


  //! Leaf tree node with degree 0
  struct LeafNode
      : public TreeProperties<Properties::IsLeaf | Properties::HasStaticSize>
  {
    static constexpr index_constant<0> degree() { return {}; }
  };


  //! Non-uniform type-tree with all sub-trees of different type
  template<class... Childs>
  struct CompositeNode
      : private Dune::TupleVector<Childs...>
      , public TreeProperties<Properties::HasStaticSize>
  {
    using Super = Dune::TupleVector<Childs...>;

    //! The type of the childs tuple
    using ChildTypes = std::tuple<Childs...>;

    //! The type of the i'th child
    template <std::size_t i>
    using Child = std::tuple_element_t<i,ChildTypes>;

    //! Inherit the constructors from std::tuple
    using Super::Super;

    //! Return a reference to the i'th child of the tree
    template<std::size_t I0, class... II>
    auto& child(index_constant<I0> i, II... ii)
    {
      static_assert(I0 < sizeof...(Childs));
      if constexpr(sizeof...(II) > 0)
        return Super::operator[](i).child(ii...);
      else
        return Super::operator[](i);
    }

    //! Return a const reference to the i'th child of the tree
    template<std::size_t I0, class... II>
    const auto& child(index_constant<I0> i, II... ii) const
    {
      static_assert(I0 < sizeof...(Childs));
      if constexpr(sizeof...(II) > 0)
        return Super::operator[](i).child(ii...);
      else
        return Super::operator[](i);
    }

    //! Return the number of nodes
    static constexpr index_constant<(sizeof...(Childs))> degree() { return {}; }
  };

  // deduction guides
  template<class... Childs>
  CompositeNode(const Childs&...)
    -> CompositeNode<Childs...>;


  //! Non-uniform type-tree with all sub-trees of the same type and static size.
  template<class Child, std::size_t n>
  struct StaticPowerNode
      : public TreeProperties<Properties::IsTypeUniform | Properties::HasStaticSize>
      , public std::array<Child,n>
  {
    using Super = std::array<Child, n>;

    //! The type of the childs
    using ChildType = Child;

    //! Explicit definition of default constructor
    StaticPowerNode()
      : Super{}
    {}

    //! Forward all arguments to the array
    template<class... Childs,
      std::enable_if_t<(1+sizeof...(Childs) == n), int> = 0>
    explicit StaticPowerNode(const Child& child, Childs&&... childs)
      : Super{child, std::forward<Childs>(childs)...}
    {}

    //! Repeat the single argument to fill the array
    explicit StaticPowerNode(index_constant<n>, const Child& child)
      : Super{Dune::unpackIntegerSequence(
          [&](auto... ii) {
            return Super{(void(ii), child)...};
          }, std::make_index_sequence<n>{})
        }
    {}

    //! Return a reference to the i'th child of the tree
    template<class... II>
    auto& child(std::size_t i, II... ii)
    {
      assert(std::size_t(i) < n);
      if constexpr(sizeof...(II) > 0)
        return Super::operator[](i).child(ii...);
      else
        return Super::operator[](i);
    }

    //! Return a const reference to the i'th child of the tree
    template<class... II>
    const auto& child(std::size_t i, II... ii) const
    {
      assert(std::size_t(i) < n);
      if constexpr(sizeof...(II) > 0)
        return Super::operator[](i).child(ii...);
      else
        return Super::operator[](i);
    }

    //! Return the number of nodes
    static constexpr index_constant<n> degree() { return {}; }
  };

  // deduction guides
  template<std::size_t n, class Child>
  StaticPowerNode(index_constant<n>, const Child&)
    -> StaticPowerNode<Child, n>;

  template<class Child, class... Childs,
    std::enable_if_t<not Dune::IsIntegralConstant<Child>::value, int> = 0,
    std::enable_if_t<(std::is_convertible_v<Childs,Child> &&...), int> = 0>
  StaticPowerNode(const Child&, const Childs&...)
    -> StaticPowerNode<Child, 1+sizeof...(Childs)>;

  // alias for backwards compatibility
  template<class Child, std::size_t n>
  using PowerNode = StaticPowerNode<Child,n>;


  //! Non-uniform type-tree with all sub-trees of the same type and dynamic size.
  template<class Child>
  struct DynamicPowerNode
      : public TreeProperties<Properties::IsTypeUniform>
      , public std::vector<Child>
  {
    using Super = std::vector<Child>;

    //! The type of the childs
    using ChildType = Child;

    //! Inherit the constructors from std::vector
    using Super::Super;

    //! Return a reference to the i'th child of the tree
    template<class... II>
    auto& child(std::size_t i, II... ii)
    {
      assert(std::size_t(i) < degree());
      if constexpr(sizeof...(II) > 0)
        return Super::operator[](i).child(ii...);
      else
        return Super::operator[](i);
    }

    //! Return a const reference to the i'th child of the tree
    template<class... II>
    const auto& child(std::size_t i, II... ii) const
    {
      assert(std::size_t(i) < degree());
      if constexpr(sizeof...(II) > 0)
        return Super::operator[](i).child(ii...);
      else
        return Super::operator[](i);
    }

    //! Return the number of nodes
    std::size_t degree() const { return Super::size(); }
  };

  // deduction guides
  template<class Child>
  DynamicPowerNode(std::size_t, const Child&)
    -> DynamicPowerNode<Child>;


  //! Uniform type-tree with static size.
  template<class Child, std::size_t n>
  struct StaticUniformPowerNode
      : public TreeProperties<Properties::IsUniform | Properties::IsTypeUniform | Properties::HasStaticSize>
  {
    //! The type of the childs
    using ChildType = Child;

    //! Constructor that stores the `child`. Can be used for class-template-argument deduction.
    StaticUniformPowerNode(index_constant<n>, Child child)
      : child_{std::move(child)}
    {}

    //! Return a reference to the i'th child of the tree
    template<class I0, class... II>
    auto& child([[maybe_unused]] I0 i, II... ii)
    {
      assert(std::size_t(i) < n);
      if constexpr(sizeof...(II) > 0)
        return child_.child(ii...);
      else
        return child_;
    }

    //! Return a const reference to the i'th child of the tree
    template<class I0, class... II>
    const auto& child([[maybe_unused]] I0 i, II... ii) const
    {
      assert(std::size_t(i) < n);
      if constexpr(sizeof...(II) > 0)
        return child_.child(ii...);
      else
        return child_;
    }

    //! Return the number of nodes
    static constexpr index_constant<n> degree() { return {}; }

    Child child_;
  };


  //! Uniform type-tree with dynamic size.
  template<class Child>
  struct DynamicUniformPowerNode
      : public TreeProperties<Properties::IsUniform | Properties::IsTypeUniform>
  {
    //! The type of the childs
    using ChildType = Child;

    //! Constructor that stores the `degree` and the `child`.
    //! Can be used for class-template-argument deduction.
    DynamicUniformPowerNode(std::size_t degree, Child child)
      : degree_{degree}
      , child_{std::move(child)}
    {}

    //! Return a reference to the i'th child of the tree
    template<class I0, class... II>
    auto& child([[maybe_unused]] I0 i, II... ii)
    {
      assert(std::size_t(i) < degree_);
      if constexpr(sizeof...(II) > 0)
        return child_.child(ii...);
      else
        return child_;
    }

    //! Return a const reference to the i'th child of the tree
    template<class I0, class... II>
    const auto& child([[maybe_unused]] I0 i, II... ii) const
    {
      assert(std::size_t(i) < degree_);
      if constexpr(sizeof...(II) > 0)
        return child_.child(ii...);
      else
        return child_;
    }

    //! Return the number of nodes
    std::size_t degree() const { return degree_; }

    std::size_t degree_;
    Child child_;
  };

}} // end namespace Dune::TypeTree

#endif // DUNE_TYPETREE_NODES_HH
