// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-GPL-2.0-only-with-PDELab-exception
#include "config.h"

#include <dune/common/classname.hh>
#include <dune/common/test/testsuite.hh>

#include <dune/typetree/proxynode.hh>

#include "typetreetestutility.hh"

template<typename Node>
class SimpleProxy
  : public Dune::TypeTree::ProxyNode<Node>
{

  typedef Dune::TypeTree::ProxyNode<Node> BaseT;

public:

  static const char* name()
  {
    return "SimpleProxy";
  }

  int id() const
  {
    return this->proxiedNode().id();
  }

  SimpleProxy(Node& node)
    : BaseT(node)
  {}

};

template<typename Node>
void testProxyNode(Node& node)
{
  Dune::TestSuite suite{"Check ProxyNode on " + Dune::className<Node>()};
  namespace Info = Dune::TypeTree::Experimental::Info;
  typedef SimpleProxy<Node> ProxyNode;
  ProxyNode proxyNode(node);
  Dune::TypeTree::applyToTree(proxyNode,TreePrinter());
  static_assert(decltype(Info::depth(node)){} == decltype(Info::depth(proxyNode)){}, "Proxy node has wrong depth");
  suite.check(Info::nodeCount(node) == Info::nodeCount(proxyNode)) << "Proxy node has wrong node count";
  suite.check(Info::leafCount(node) == Info::leafCount(proxyNode)) << "Proxy node has wrong leaf count";
}


int main(int argc, char** argv)
{

  // basic tests

  // leaf node
  SimpleLeaf sl1;

  typedef SimplePower<SimpleLeaf,3> SP1;
  SP1 sp1_1;
  sp1_1.setChild(0,sl1);
  sp1_1.setChild(1,sl1);
  sp1_1.setChild(2,sl1);

  typedef SimpleDynamicPower<SimpleLeaf> SDP1;
  SDP1 sdp1_1{sl1,sl1,sl1};

  SimpleLeaf sl2;
  SP1 sp1_2(sl2,false);

  Dune::TypeTree::applyToTree(sp1_1,TreePrinter());

  typedef SimpleComposite<SimpleLeaf,SP1,SimpleLeaf> SC1;
  SC1 sc1_1(sl1,sp1_2,sl2);

  typedef SimpleComposite<SimpleLeaf,SimpleLeaf,SimpleLeaf> SC2;
  SC2 sc2(sl1,sl1,sl1);

  testProxyNode(sl1);
  testProxyNode(sp1_1);
  testProxyNode(sdp1_1);
  testProxyNode(sc1_1);

  testProxyNode<const SimpleLeaf>(sl1);
  testProxyNode<const SP1>(sp1_1);
  testProxyNode<const SDP1>(sdp1_1);
  testProxyNode<const SC1>(sc1_1);

  typedef SimpleComposite<SimpleLeaf,SP1,SimpleLeaf,SC1> SVC1;
  SVC1 svc1_1(sl1,sp1_1,sl2,sc1_1);

  SP1 sp1_3(SimpleLeaf(),SimpleLeaf(),sl1);

  SVC1 svc1_2(SimpleLeaf(),SP1(sp1_2),sl2,const_cast<const SC1&>(sc1_1));

  typedef SimpleComposite<SimpleLeaf,SVC1,SimpleLeaf,SP1,SC1> SVC2;
  SVC2 svc2_1(sl1,svc1_2,sl2,sp1_3,sc1_1);

  testProxyNode(svc2_1);
  testProxyNode<const SVC2>(svc2_1);

  return 0;
}
