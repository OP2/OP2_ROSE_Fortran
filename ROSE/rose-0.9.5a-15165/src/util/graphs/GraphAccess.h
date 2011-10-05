
/* Qing Yi: This file defines a graph access interface that is used
  in several graph analysis/processing algorithms defined in this directory
*/
#ifndef GRAPH_ACCESS_H
#define GRAPH_ACCESS_H

#include <IteratorTmpl.h>

class GraphAccess {
 public:
  typedef enum {EdgeOut = 1, EdgeIn = 2, BiEdge = 3} EdgeDirection;
  static EdgeDirection Reverse(EdgeDirection d)
    {  switch (d) {
       case EdgeOut: return EdgeIn;
       case EdgeIn: return EdgeOut;
       case BiEdge: return BiEdge;
      }
      assert(false);
    }
};

// The graph access template
template <class NodeImpl, class EdgeImpl>
class GraphAccessTemplate : public GraphAccess
{
 public:
  typedef NodeImpl Node;
  typedef EdgeImpl Edge;
  typedef IteratorWrap<Edge*,IteratorImpl<Edge*> > EdgeIterator;
  typedef IteratorWrap<Node*,IteratorImpl<Node*> > NodeIterator;

  virtual ~GraphAccessTemplate() {}
  virtual NodeIterator GetNodeIterator() const = 0; // iterator to all nodes
  virtual EdgeIterator   // the iterator to all edges incident to node n
  GetNodeEdgeIterator(const Node* n, EdgeDirection dir) const=0;
  virtual Node*  // the source or sink of edge e
  GetEdgeEndPoint( const Edge* e, EdgeDirection dir) const = 0;
  virtual bool ContainNode(const Node* n) const = 0; // whether graph contains n
  virtual bool ContainEdge(const Edge* e) const = 0; // whether graph contains e
  virtual std::string nodeToString(Node* n) const { return ""; }
  virtual std::string edgeToString(Edge* n) const { return ""; }
};

// this template class builds a bridge to GraphAccess interface 
// from a concrete graph implementaion. 
template <class NodeBase, class EdgeBase, class GraphImpl>
class GraphAccessWrapTemplate : public GraphAccessTemplate<NodeBase,EdgeBase>
{
 protected:
  GraphImpl* impl;
 public:
  typedef NodeBase Node;
  typedef EdgeBase Edge;
  typedef typename GraphAccessTemplate<Node,Edge>::NodeIterator NodeIterator;
  typedef typename GraphAccessTemplate<Node,Edge>::EdgeIterator EdgeIterator;


  GraphAccessWrapTemplate(GraphImpl* _impl) : impl(_impl) {}
  ~GraphAccessWrapTemplate() {}

  NodeIterator GetNodeIterator() const 
    { return new IteratorImplTemplate<Node*,typename GraphImpl::NodeIterator>
            (impl->GetNodeIterator()); } 
  EdgeIterator 
  GetNodeEdgeIterator(const Node* n, GraphAccess::EdgeDirection dir) const
   { return new IteratorImplTemplate<Edge*,typename GraphImpl::EdgeIterator>
             (impl->GetNodeEdgeIterator(static_cast<const typename GraphImpl::Node*>(n),dir)); }
  Node* GetEdgeEndPoint( const Edge* e, GraphAccess::EdgeDirection dir) const
    { return  impl->GetEdgeEndPoint(static_cast<const typename GraphImpl::Edge*>(e), dir); }
  bool ContainNode(const Node* n) const 
    { return (n == 0)? false : 
           impl->ContainNode(static_cast<const typename GraphImpl::Node*>(n)); }
  bool ContainEdge(const Edge* e) const 
    { return (e == 0)? false : 
       impl->ContainEdge(static_cast<const typename GraphImpl::Edge*>(e)); }
};

// the graph access interface used in various graph processing algorithms
typedef GraphAccessTemplate<void, void> GraphAccessInterface;

#endif
