#ifndef ROSE_BinaryAnalysis_FunctionCall_H
#define ROSE_BinaryAnalysis_FunctionCall_H

#include "BinaryControlFlow.h"

class SgAsmFunctionDeclaration;

namespace BinaryAnalysis {

    /** Binary function call analysis.
     *
     *  This class serves mostly to organize the functions that operate on function calls, but also provides a container for
     *  various settings that influence the function call analyses, such as vertex and edge filters.
     *
     *  Function call graphs can be computed over any subtree of the AST, although one usually does so over an entire binary
     *  interpretation (SgAsmInterpretation).  The vertex and edge filters can restrict which functions and call edges are
     *  considered by the various methods of this class. */
    class FunctionCall {
    public:

        FunctionCall()
            : vertex_filter(NULL), edge_filter(NULL)
            {}

        /** The default function call graph type.
         *
         *  A function call graph is simply a Boost graph whose vertex descriptors are integers and whose vertices point to
         *  SgAsmFunctionDeclaration nodes in the AST (via the boost::vertex_name property).  The graph edges represent
         *  function calls from one SgAsmFunctionDeclaration to another.  Since this graph is a Boost graph, it is endowed with
         *  all the features of a Boost graph and can be the operand of the various Boost graph algorithms.  See build_cg() for
         *  specifics about what is included in such a graph.
         *
         *  Another way to represent function calls is to adapt a global control flow graph
         *  (BinaryAnalysis::ControlFlowGraph) to include only the edges (and their incident vertices) that flow from one
         *  function to another.  The advantage of using a control flow graph to represent function call information is that
         *  each call site will be included in the function call graph due to the fact that the control flow graph vertices are
         *  blocks (SgAsmBlock) rather than functions (SgAsmFunctionDeclaration).
         *
         *  It is common to need a type for the vertices and edges.  Boost graphs store this information in graph_traits and
         *  users should use that to obtain those types.  Doing so will, in the long run, make your code more extensible since
         *  the only datatype you're depending on is the graph itself--change the graph type and the vertex and edge types will
         *  automatically adjust.  See Boost Graph Library documentation for all the available types.  The most common are:
         *
         *  @code
         *  typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
         *  typedef boost::graph_traits<Graph>::edge_descriptor Edge;
         *  @endcode
         */
        typedef boost::adjacency_list<boost::listS,                                 /* out-edges of each vertex in std::list */
                                      boost::vecS,                                  /* store vertices in std::vector */
                                      boost::directedS,                             /* call graph is directed */
                                      boost::property<boost::vertex_name_t, SgAsmFunctionDeclaration*>
                                     > Graph;


        /**********************************************************************************************************************
         *                                      Filters
         **********************************************************************************************************************/
    public:

        /** Filter for vertices.
         *
         *  This class can be specialized in order to filter out functions (SgAsmFunctionDeclaration) that satisfy an arbitrary
         *  condition.  See set_vertex_filter() for details. */
        class VertexFilter {
        public:
            virtual ~VertexFilter() {}
            virtual bool operator()(FunctionCall*, SgAsmFunctionDeclaration*) = 0;
        };

        /** Filter for edges.
         *
         *  This class can be specialized in order to filter out certain edges that would otherwise make it into the function
         *  call graph.  See set_edge_filter() for details. */
        class EdgeFilter {
        public:
            virtual ~EdgeFilter() {}
            virtual bool operator()(FunctionCall*, SgAsmFunctionDeclaration *source, SgAsmFunctionDeclaration *target) = 0;
        };

        /** Manipulate the vertex filter.
         *
         *  When building a function call graph, the vertex filter is invoked on each function which is about to be added as
         *  a vertex.  If the filter returns false then that function is not added to the graph.  A null filter accepts all
         *  vertices.
         *
         *  @{ */
        void set_vertex_filter(VertexFilter *filter) { vertex_filter = filter; }
        VertexFilter *get_vertex_filter() const { return vertex_filter; }
        /** @}*/

        /** Manipulate the edge filter.
         *
         *  When building a function call graph, the edge filter is invoked for each edge which is about to be added to the
         *  graph. If the filter returns false then that edge is not added to the graph.  A null filter accepts all edges.
         *
         *  @{ */
        void set_edge_filter(EdgeFilter *filter) { edge_filter = filter; }
        EdgeFilter *get_edge_filter() const { return edge_filter; }
        /** @}*/

        /** Determines if a vertex is filtered out.
         *
         *  Returns true if the vertex would be filtered out by being rejected by the current vertex filter.
         *
         *  @{ */
        bool is_vertex_filtered(SgAsmFunctionDeclaration *func, VertexFilter *filter) {
            return filter && !(*filter)(this, func);
        }
        bool is_vertex_filtered(SgAsmFunctionDeclaration *func) {
            return is_vertex_filtered(func, vertex_filter);
        }
        /** @} */

        /** Determines if an edge is filtered out.
         *
         *  Returns true if the edge would be filtered out by being rejected by the current edge filter.
         *
         *  @{ */
        bool is_edge_filtered(SgAsmFunctionDeclaration *src, SgAsmFunctionDeclaration *dst, EdgeFilter *filter) {
            return filter && !(*filter)(this, src, dst);
        }
        bool is_edge_filtered(SgAsmFunctionDeclaration *src, SgAsmFunctionDeclaration *dst) {
            return is_edge_filtered(src, dst, edge_filter);
        }
        /** @} */

    protected:
        VertexFilter *vertex_filter;
        EdgeFilter *edge_filter;

        /**********************************************************************************************************************
         *                                      Methods that modify the AST
         **********************************************************************************************************************/
    public:

        /** Cache vertex descriptors in AST.
         *
         *  The vertices of a function call graph are of type Vertex, and point at the functions (SgAsmFunctionDeclaration) of
         *  the AST. Although most graph algorithms will only need to map Vertex to SgAsmFunctionDeclaration, the inverse
         *  mapping is also sometimes useful.  That mapping can be stored into an std::map via graph traversal, or stored in
         *  the AST itself attached to each SgAsmFunctionDeclaration.  Using an std::map requires an O(log N) lookup each time
         *  we need to get the vertex descriptor for a function, while storing the vertex descriptor in the AST requires O(1)
         *  lookup time.
         *
         *  The vertex descriptors are available via SgAsmFunctionDeclaration::get_cached_vertex().  Other graph types (e.g.,
         *  dominance graphs) might also use the same cache line.  The cached vertex is stored as a size_t, which is the same
         *  underlying type for function call graph vertices.
         *
         *  The current vertex filter determines which function nodes are modified. */
        template<class FunctionCallGraph>
        void cache_vertex_descriptors(const FunctionCallGraph&);

        /**********************************************************************************************************************
         *                                      Graph construction methods
         **********************************************************************************************************************/
    public:

        /** Build a function call graph from a control flow graph.
         *
         *  Given a control flow graph (CFG) spanning multiple functions, create a function call graph (CG) by collapsing
         *  vertices in the CFG that belong to a common function.  Any resulting self-loop edges will be removed unless the
         *  target of the corresponding edge in the CFG was the function entry block (i.e., intra-function CFG edges whose
         *  target is the function's entry block are assumed to be recursive calls, while all other intra-function CFG edges
         *  are omitted from the CG).
         *
         *  The current vertex and edge filters are used to restrict which functions and calls make it into the graph.
         *
         *  @{ */
        template<class FunctionCallGraph, class ControlFlowGraph>
        FunctionCallGraph build_cg_from_cfg(const ControlFlowGraph&);

        template<class ControlFlowGraph, class FunctionCallGraph>
        void build_cg_from_cfg(const ControlFlowGraph &cfg, FunctionCallGraph &cg/*out*/);
        /** @} */

        /** Build a function call graph from an AST.
         *
         *  Given an AST, traverse the AST beginning at @p root and build a function call graph (CG).  The function call graph
         *  will contain only SgAsmFunctionDeclaration vertices that are in the specified subtree and which are not filtered
         *  out by the current vertex filter.  Edges also must pass the edge filter to be included in the graph.
         *
         *  The following two methods of constructing a CG should result in identical graphs (although vertex and edge order
         *  may be different):
         *
         *  @code
         *  using namespace BinaryAnalysis;
         *  typedef FunctionCall::Graph CG;
         *  typedef ControlFlow::Graph  CFG;
         *  SgAsmNode *node = ...;
         *
         *  // Method 1
         *  CG cg1 = FunctionCall().build_cg_from_ast(node);
         *
         *  // Method 2
         *  CFG cfg = ControlFlow().build_cfg_from_ast<CFG>(node);
         *  CG cg2 = FunctionCall().build_cg_from_cfg<CG>(cfg);
         *  @endcode
         *
         *  In general, building the function call graph directly from the AST will be faster than first building the control
         *  flow graph.
         *
         *  @{ */
        template<class FunctionCallGraph>
        FunctionCallGraph build_cg_from_ast(SgNode *root);

        template<class FunctionCallGraph>
        void build_cg_from_ast(SgNode *root, FunctionCallGraph &cg/*out*/);
        /** @} */

        /** Copies a graph while filtering.
         *
         *  Copies a graph while applying the current source and destination vertex and edge filters.  If all vertices are
         *  selected by the vertex filter, then the desintation graph's vertex descriptors will correspond to the same
         *  vertices in the source graph (i.e., vertex V in the source will be the same basic block as vertex V in the
         *  destination).
         *
         *  If an edge is unfiltered but one of its vertices is filtered, then the edge will not be included in the result.
         *
         *  @{ */
        template<class FunctionCallGraph>
        FunctionCallGraph copy(const FunctionCallGraph &src);
        template<class FunctionCallGraph>
        void copy(const FunctionCallGraph &src, FunctionCallGraph &dst/*out*/);
        /** @} */

    };
}



/******************************************************************************************************************************
 *                                      Function template definitions
 ******************************************************************************************************************************/

template<class FunctionCallGraph>
void
BinaryAnalysis::FunctionCall::cache_vertex_descriptors(const FunctionCallGraph &cg)
{
    typename boost::graph_traits<FunctionCallGraph>::vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end)=vertices(cg); vi!=vi_end; ++vi) {
        SgAsmFunctionDeclaration *func = get(boost::vertex_name, cg, *vi);
        if (func && !is_vertex_filtered(func))
            func->set_cached_vertex(*vi);
    }
}

template<class ControlFlowGraph, class FunctionCallGraph>
void
BinaryAnalysis::FunctionCall::build_cg_from_cfg(const ControlFlowGraph &cfg, FunctionCallGraph &cg/*out*/)
{
    typedef typename boost::graph_traits<FunctionCallGraph>::vertex_descriptor CG_Vertex;
    typedef typename boost::graph_traits<ControlFlowGraph>::vertex_descriptor CFG_Vertex;
    typedef std::map<SgAsmFunctionDeclaration*, CG_Vertex> FunctionVertexMap;

    cg.clear();

    /* Add CG vertices by collapsing CFG nodes that belong to a common function. */
    FunctionVertexMap fv_map;
    typename boost::graph_traits<ControlFlowGraph>::vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end)=vertices(cfg); vi!=vi_end; ++vi) {
        SgAsmBlock *block = get(boost::vertex_name, cfg, *vi);
        SgAsmFunctionDeclaration *func = block->get_enclosing_function();
        if (!is_vertex_filtered(func)) {
            typename FunctionVertexMap::iterator fi=fv_map.find(func);
            if (func && fi==fv_map.end()) {
                CG_Vertex v = add_vertex(cg);
                put(boost::vertex_name, cg, v, func);
                fv_map[func] = v;
            }
        }
    }

    /* Add edges whose target is a function entry block. */
    typename boost::graph_traits<ControlFlowGraph>::edge_iterator ei, ei_end;
    for (boost::tie(ei, ei_end)=edges(cfg); ei!=ei_end; ++ei) {
        CFG_Vertex cfg_a = source(*ei, cfg);
        CFG_Vertex cfg_b = target(*ei, cfg);
        SgAsmBlock *block_a = get(boost::vertex_name, cfg, cfg_a);
        SgAsmBlock *block_b = get(boost::vertex_name, cfg, cfg_b);
        SgAsmFunctionDeclaration *func_a = block_a->get_enclosing_function();
        SgAsmFunctionDeclaration *func_b = block_b->get_enclosing_function();
        if (func_a && func_b && block_b==func_b->get_entry_block() && !is_edge_filtered(func_a, func_b)) {
            typename FunctionVertexMap::iterator fi_a = fv_map.find(func_a);
            if (fi_a!=fv_map.end()) {
                typename FunctionVertexMap::iterator fi_b = fv_map.find(func_b);
                if (fi_b!=fv_map.end())
                    add_edge(fi_a->second, fi_b->second, cg);
            }
        }
    }
}

template<class FunctionCallGraph, class ControlFlowGraph>
FunctionCallGraph
BinaryAnalysis::FunctionCall::build_cg_from_cfg(const ControlFlowGraph &cfg)
{
    FunctionCallGraph cg;
    build_cg_from_cfg(cfg, cg);
    return cg;
}

template<class FunctionCallGraph>
void
BinaryAnalysis::FunctionCall::build_cg_from_ast(SgNode *root, FunctionCallGraph &cg/*out*/)
{
    typedef typename boost::graph_traits<FunctionCallGraph>::vertex_descriptor Vertex;
    typedef std::map<SgAsmFunctionDeclaration*, Vertex> FunctionVertexMap;
    FunctionVertexMap fv_map;

    cg.clear();

    /* Visiter that adds a vertex for each unique function. */
    struct VertexAdder: public AstSimpleProcessing {
        FunctionCall *analyzer;
        FunctionCallGraph &cg;
        FunctionVertexMap &fv_map;
        VertexAdder(FunctionCall *analyzer, FunctionCallGraph &cg, FunctionVertexMap &fv_map)
            : analyzer(analyzer), cg(cg), fv_map(fv_map)
            {}
        void visit(SgNode *node) {
            SgAsmFunctionDeclaration *func = isSgAsmFunctionDeclaration(node);
            if (func && !analyzer->is_vertex_filtered(func)) {
                Vertex vertex = add_vertex(cg);
                fv_map[func] = vertex;
                put(boost::vertex_name, cg, vertex, func);
            }
        }
    };

    /* Visitor that adds edges for each vertex.  Traversal should be over one function at a time. */
    struct EdgeAdder: public AstSimpleProcessing {
        FunctionCall *analyzer;
        FunctionCallGraph &cg;
        FunctionVertexMap &fv_map;
        Vertex source_vertex;
        EdgeAdder(FunctionCall *analyzer, FunctionCallGraph &cg, FunctionVertexMap &fv_map, Vertex source_vertex)
            : analyzer(analyzer), cg(cg), fv_map(fv_map), source_vertex(source_vertex)
            {}
        SgAsmFunctionDeclaration *function_of(SgAsmBlock *block) {
            return block ? block->get_enclosing_function() : NULL;
        }
        void visit(SgNode *node) {
            SgAsmBlock *block_a = isSgAsmBlock(node); /* the calling block */
            SgAsmFunctionDeclaration *func_a = function_of(block_a); /* the calling function */
            if (!func_a)
                return;
            const SgAsmTargetPtrList &succs = block_a->get_successors();
            for (SgAsmTargetPtrList::const_iterator si=succs.begin(); si!=succs.end(); ++si) {
                SgAsmBlock *block_b = (*si)->get_block(); /* the called block */
                SgAsmFunctionDeclaration *func_b = function_of(block_b); /* the called function */
                if (func_b && block_b==func_b->get_entry_block() && !analyzer->is_edge_filtered(func_a, func_b)) {
                    typename FunctionVertexMap::iterator fi_b = fv_map.find(func_b); /* find vertex for called function */
                    if (fi_b!=fv_map.end())
                        add_edge(source_vertex, fi_b->second, cg);
                }
            }
        }
    };

    VertexAdder(this, cg, fv_map).traverse(root, preorder);
    typename boost::graph_traits<FunctionCallGraph>::vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end)=vertices(cg); vi!=vi_end; ++vi) {
        SgAsmFunctionDeclaration *source_func = get(boost::vertex_name, cg, *vi);
        EdgeAdder(this, cg, fv_map, *vi).traverse(source_func, preorder);
    }
}

template<class FunctionCallGraph>
FunctionCallGraph
BinaryAnalysis::FunctionCall::build_cg_from_ast(SgNode *root)
{
    FunctionCallGraph cg;
    build_cg_from_ast(root, cg);
    return cg;
}

template<class FunctionCallGraph>
void
BinaryAnalysis::FunctionCall::copy(const FunctionCallGraph &src, FunctionCallGraph &dst)
{
    typedef typename boost::graph_traits<FunctionCallGraph>::vertex_descriptor Vertex;
    Vertex NO_VERTEX = typename boost::graph_traits<FunctionCallGraph>::null_vertex();

    dst.clear();
    std::vector<Vertex> src_to_dst(num_vertices(src), NO_VERTEX);

    typename boost::graph_traits<FunctionCallGraph>::vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end)=vertices(src); vi!=vi_end; ++vi) {
        SgAsmFunctionDeclaration *func = get(boost::vertex_name, src, *vi);
        if (!is_vertex_filtered(func)) {
            src_to_dst[*vi] = add_vertex(dst);
            put(boost::vertex_name, dst, src_to_dst[*vi], func);
        }
    }

    typename boost::graph_traits<FunctionCallGraph>::edge_iterator ei, ei_end;
    for (boost::tie(ei, ei_end)=edges(src); ei!=ei_end; ++ei) {
        if (NO_VERTEX!=src_to_dst[source(*ei, src)] && NO_VERTEX!=src_to_dst[target(*ei, src)]) {
            SgAsmFunctionDeclaration *func1 = get(boost::vertex_name, src, source(*ei, src));
            SgAsmFunctionDeclaration *func2 = get(boost::vertex_name, src, target(*ei, src));
            if (!is_edge_filtered(func1, func2))
                add_edge(src_to_dst[source(*ei, src)], src_to_dst[target(*ei, src)], dst);
        }
    }
}

template<class FunctionCallGraph>
FunctionCallGraph
BinaryAnalysis::FunctionCall::copy(const FunctionCallGraph &src)
{
    FunctionCallGraph dst;
    copy(src, dst);
    return dst;
}



#endif
