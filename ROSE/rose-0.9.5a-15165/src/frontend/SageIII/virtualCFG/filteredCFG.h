#ifndef FILTEREDCFGVIEW_H
#define FILTEREDCFGVIEW_H

//#include "rose.h"
#include "virtualCFG.h"
#include <string>
#include <vector>

namespace VirtualCFG
{
    // Function call node and edge filters

    template < typename FilterFunction > class FilteredCFGEdge;

    template < typename FilterFunction > class FilteredCFGNode
    {
        CFGNode n;
        FilterFunction filter;

      public:
        FilteredCFGNode(CFGNode n):n(n)
        {
        }

        /**  Need a default constructor to use with boost_graph. Underlying node will be invalid. */
        FilteredCFGNode() : n()
        {
        }

        CFGNode toNode() const
        {
            return n;
        }
        std::string toString() const
        {
                                                std::string nToString= n.toString();
                                                if (nToString.find('>',0)>0)
                                                nToString.insert(nToString.find('>',0)+1,"\n");
            return nToString;
        }
        std::string toStringForDebugging() const
        {
            return n.toStringForDebugging();
        }
        std::string id() const
        {
            return n.id();
        }
        SgNode *getNode() const
        {
            return n.getNode();
        }
        unsigned int getIndex() const
        {
            return n.getIndex();
        }
        std::vector < FilteredCFGEdge < FilterFunction > >outEdges() const;
        std::vector < FilteredCFGEdge < FilterFunction > >inEdges() const;
        bool isInteresting() const
        {
            return true;
        }
        bool operator==(const FilteredCFGNode & o)const
        {
            return n == o.n;
        }
        bool operator!=(const FilteredCFGNode & o)const
        {
            return !(*this == o);
        }
        bool operator<(const FilteredCFGNode & o)const
        {
            return n < o.n;
        }
    };

    template < typename FilterFunction > class FilteredCFGEdge
    {
        CFGPath p;
        FilterFunction filter;

      public:
      FilteredCFGEdge(CFGPath p):p(p)
        {
        }

        /**  Need a default constructor to use with boost_graph. Underlying path will be invalid. */
        FilteredCFGEdge()
        {
        }

        std::string toString()const
        {
            return p.toString();
        }
        std::string toStringForDebugging() const
        {
            return p.toStringForDebugging();
        }
        std::string id() const
        {
            return p.id();
        }
        FilteredCFGNode < FilterFunction > source() const
        {
            return FilteredCFGNode < FilterFunction > (p.source());
        }
        FilteredCFGNode < FilterFunction > target() const
        {
            return FilteredCFGNode < FilterFunction > (p.target());
        }
        EdgeConditionKind condition() const
        {
            return p.condition();
        }
        SgExpression *caseLabel() const
        {
            return p.caseLabel();
        }
        std::vector < SgInitializedName * >scopesBeingExited() const
        {
            return p.scopesBeingExited();
        }
        std::vector < SgInitializedName * >scopesBeingEntered() const
        {
            return p.scopesBeingEntered();
        }
        bool operator==(const FilteredCFGEdge < FilterFunction > &o)const
        {
            return p == o.p;
        }
        bool operator!=(const FilteredCFGEdge < FilterFunction > &o)const
        {
            return p != o.p;
        }
        bool operator<(const FilteredCFGEdge < FilterFunction > &o)const
        {
            return p < o.p;
        }
        
        //! The underlying path in the full CFG represented by this edge in the filtered CFG.
        const CFGPath& getPath() const
        {
                return p;
        }
    };

    template < typename FilterFunction > std::ostream & cfgToDot(std::ostream & o,
                                                                 std::string graphName,
                                                                 FilteredCFGNode <
                                                                 FilterFunction > start);
}

#include "filteredCFGImpl.h"
#endif // XCFGVIEW_H
