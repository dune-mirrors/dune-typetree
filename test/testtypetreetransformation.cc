#include "config.h"

#include "typetreetestswitch.hh"

#if TEST_TYPETREE_INVALID

int main()
{
  return 0;
}

#else

#include <dune/typetree/transformedtree.hh>

#include "typetreetestutility.hh"
#include "typetreetargetnodes.hh"

int main(int argc, char** argv)
{

  // basic tests

  // leaf node
  TreePrinter treePrinter;
  SimpleLeaf sl1;

  Dune::TypeTree::applyToTree(sl1,treePrinter);

  auto testTransformation = [](const auto& sourceNode, const auto& nodeBase) {
    using Node = std::decay_t<decltype(sourceNode)>;
    if constexpr(Node::isLeaf)
      return TargetLeaf{sourceNode,nodeBase};
    else if constexpr(Node::isPower && Node::hasStaticSize)
      return TargetPower{sourceNode,nodeBase};
    else if constexpr(Node::isPower)
      return TargetDynamicPower{sourceNode,nodeBase};
    else if constexpr(Node::isComposite)
      return TargetComposite{sourceNode,nodeBase};
    else
      return 0;
  };

  auto tl1 = transformedTree(sl1, testTransformation);

  typedef SimpleDynamicPower<SimpleLeaf> SDP;
  SDP sdp(sl1,sl1,sl1);

  typedef SimplePower<SimpleLeaf,3> SP1;
  SP1 sp1_1;
  sp1_1[0] = sl1;
  sp1_1[1] = sl1;
  sp1_1[2] = sl1;

  SimpleLeaf sl2;
  SP1 sp1_2(sl2,sl2,sl2);

  typedef SimpleComposite<SimpleLeafDerived,SP1,SimpleLeaf> SVC1;

  SVC1 svc1_1(SimpleLeafDerived(),sp1_2,sl1);

  Dune::TypeTree::applyToTree(sp1_1,TreePrinter());

  auto tsdp1_1 = transformedTree(sdp, testTransformation);
  auto tp1_1   = transformedTree(sp1_1, testTransformation);
  auto tvc1_1  = transformedTree(svc1_1, testTransformation);

  Dune::TypeTree::applyToTree(tvc1_1,TreePrinter());

  return 0;
}

#endif
