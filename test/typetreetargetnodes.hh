#pragma once

#include <dune/typetree/nodes.hh>

#include "typetreetestutility.hh"

struct TargetLeaf
  : public Dune::TypeTree::LeafNode
  , public Counter
{
  using Source = SimpleLeaf;
  using Base = Dune::TypeTree::LeafNode;

  TargetLeaf(const Source& source, const Base& /*base*/)
    : Counter{source}
  {}

  const char* name() const
  {
    return "TargetLeaf";
  }
};

template<class Source, class Base>
struct TargetPower
  : public Base
  , public Counter
{
  TargetPower(const Source& source, const Base& base)
    : Base{base}, Counter{source}
  {}

  const char* name() const
  {
    return "TargetPower";
  }
};


template<class Source, class Base>
struct TargetDynamicPower
  : public Base
  , public Counter
{
  TargetDynamicPower(const Source& source, const Base& base)
    : Base{base}, Counter{source}
  {}

  const char* name() const
  {
    return "TargetDynamicPower";
  }
};


template<class Source, class Base>
struct TargetComposite
  : public Base
  , public Counter
{
  TargetComposite(const Source& source, const Base& base)
    : Base{base}, Counter{source}
  {}

  const char* name() const
  {
    return "TargetComposite";
  }
};