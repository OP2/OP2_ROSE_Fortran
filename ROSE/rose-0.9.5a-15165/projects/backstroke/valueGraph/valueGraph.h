#ifndef BACKSTROKE_VALUE_GRAPH
#define BACKSTROKE_VALUE_GRAPH

#include "types.h"
#include "valueGraphNode.h"
#include "pathNumGenerator.h"
#include <ssa/staticSingleAssignment.h>
#include <boost/function.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>

namespace Backstroke
{

typedef boost::adjacency_list<boost::listS, boost::listS, boost::bidirectionalS,
		ValueGraphNode*, ValueGraphEdge*> ValueGraph;

class EventReverser
{
public:
	typedef boost::graph_traits<ValueGraph>::vertex_descriptor VGVertex;
	typedef boost::graph_traits<ValueGraph>::edge_descriptor   VGEdge;

private:
    //typedef std::pair<int, PathSet> PathSetWithIndex;
	typedef StaticSingleAssignment SSA;
	typedef SSA::VarName           VarName;
    typedef boost::filtered_graph<
        ValueGraph,
        boost::function<bool(const VGEdge&)>,
        boost::function<bool(const VGVertex&)> > SubValueGraph;

    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
		std::pair<PathSet, std::vector<VGEdge> >, PathSet> ReverseCFG;
    typedef boost::graph_traits<ReverseCFG>::vertex_descriptor RvsCFGVertex;
	typedef boost::graph_traits<ReverseCFG>::edge_descriptor   RvsCFGEdge;
    
    //! A functor to compare two route graph edges according to their indices from 
    //! the table passed in.
    struct RouteGraphEdgeComp
    {
        RouteGraphEdgeComp(const ValueGraph& routeG, const std::map<PathSet, int>& pathsIdx)
        : routeGraph(routeG), pathsIndexTable(pathsIdx) {}
        
        bool operator ()(const VGEdge& edge1, const VGEdge& edge2) const;
//        {
//            using namespace std;
//            cout << routeGraph[edge1]->paths << endl;
//            cout << routeGraph[edge2]->paths << endl;
//            
//            ROSE_ASSERT(pathsIndexTable.count(routeGraph[edge1]->paths));
//            ROSE_ASSERT(pathsIndexTable.count(routeGraph[edge2]->paths));
//            return pathsIndexTable.find(routeGraph[edge1]->paths)->second > 
//                   pathsIndexTable.find(routeGraph[edge2]->paths)->second;
//        }
        
        const ValueGraph& routeGraph;
        const std::map<PathSet, int>& pathsIndexTable;
    };

private:
    //! The event function definition.
    SgFunctionDefinition* funcDef_;

    //! The forward event function definition.
    SgFunctionDefinition* fwdFuncDef_;

    //! The reverse event function definition.
    SgFunctionDefinition* rvsFuncDef_;

    //! The commit event function definition.
    SgFunctionDefinition* cmtFuncDef_;

    //! The CFG of the event function.
    BackstrokeCFG* cfg_;
    
	//! The SSA form of the function definition.
	SSA* ssa_;

	//! The value graph object.
	ValueGraph valueGraph_;
    
	//! A special node in VG whose in edges are state saving edges.
	VGVertex root_;
    
    //! The graph representing the search result, which we call route.
    ValueGraph routeGraph_;
    
    //! The root node in routeGraph_ which is the same as in VG.
	VGVertex routeGraphRoot_;

    //! This object manages the path information of the function.
    PathNumManager* pathNumManager_;
	
	//! A map from SgNode to vertex of Value Graph.
	std::map<SgNode*, VGVertex> nodeVertexMap_;

	//! A map from variable with version to vertex of Value Graph.
	std::map<VersionedVariable, VGVertex> varVertexMap_;

//	//! A map from variable with version to its reaching def object.
//	//! This map is only for pseudo defs.
//	std::map<VersionedVariable, SSA::ReachingDefPtr> pseudoDefMap_;

	//! All values which need to be restored (state variables).
	std::vector<VGVertex> valuesToRestore_;

	//! All available values, including constant and state variables (last version).
	std::set<VGVertex> availableValues_;

    //!
    std::set<VGVertex> varsKilledAtEventEnd_;

	//! All state variables.
	std::set<VarName> stateVariables_;

	//! All edges in the VG which are used to reverse values.
	std::vector<VGEdge> dagEdges_;

    //! A table maps each path to all its nodes and edges in the VG. A path is expressed by
    //! a region index and path index.
    std::map<std::pair<int, int>, 
             std::pair<std::set<VGVertex>,
                       std::set<VGEdge> > > pathNodesAndEdges_;

    //! A table maps the route for each path to all its nodes and edges in the VG.
    std::map<std::pair<int, int>,
             std::pair<std::set<VGVertex>,
                       std::set<VGEdge> > > routeNodesAndEdges_;


public:
    //! The constructor.
    EventReverser(SgFunctionDefinition* funcDef);

    //! The destructor.
    ~EventReverser();

	//! Build the value graph for the given function.
	void buildValueGraph();

    //! Generate forward, reverse and commit functions according to the VG.
    void generateCode();

	//! Generate a dot file describing the value graph.
	void valueGraphToDot(const std::string& filename) const;
    
    //! Generate a dot file describing the search result.
    void routeGraphToDot(const std::string& filename) const;
    

//    void getPath(const SubValueGraph& g,
//                 const std::vector<VGVertex>& valuesToRestore);
	
private:

    //! Build the main part of the value graph.
    void buildBasicValueGraph();
    
    //! Build the route graph representing search result.
    void buildRouteGraph(const std::map<VGEdge, PathSet>& routes);
    
    //! Remove phi nodes from the route graph to facilitate code generation.
    void removePhiNodesFromRouteGraph();

    //! After the value graph is built, remove edges which don't help in the search.
    void removeUselessEdges();

    //! After the value graph is built, remove nodes which don't help in the search.
    void removeUselessNodes();

    //! Add path information to edges.
    void addPathsToEdges();

    //! Assign a global unique name for each value node in VG.
    void assignNameToNodes();

    //! Build the fwd, rvs and cmt functions.
	void buildFunctionBodies();

    //! Get all functions in place.
    void insertFunctions();
    
    //! Remove empty if statements.
    void removeEmptyIfStmt();

    /** Given a VG node with a def, returns all VG nodes whose defs kill the given def.
     *  @param killedNode The value graph node which must contains a value node or phi node.
	 *  @returns The value graph nodes which kill the def in the parameter.
	 */
    std::set<VGVertex> getKillers(VGVertex killedNode);
    
//    //! Add path information to out edges of phi nodes.
//    PathSetWithIndex addPathsForPhiNodes(VGVertex phiNode,
//                                         std::set<VGVertex>& processedPhiNodes);

    /** Create a value node from the given AST node.
	 *
	 *  @param lhsNode The AST node which contains a lvalue.
     *  @param rhsNode The AST node which contains a rvalue.
	 *  @returns The new vertex.
	 */
	VGVertex createValueNode(SgNode* lhsNode, SgNode* rhsNode);
    
    //! Create an operation node, plus two or three edges.
	VGVertex createOperatorNode(
            VariantT t,
            VGVertex result, VGVertex lhs, VGVertex rhs = nullVertex(),
            ValueGraphEdge* edgeToCopy = NULL);

    //! Add a reverse edge for every non-ordered edge, and add extra edges for
    //! + and - operations.
    void addExtraNodesAndEdges();

    //! Handle all final defs at the end of the event. This step is needed because
    //! we cannot get the phi node if it is not used. New phi node is added to VG,
    //! and all available variables are found.
    void processLastVersions();

    //! Returns if the given edge belongs to the given path.
    bool edgeBelongsToPath(const VGEdge& e, int dagIndex, int pathIndex) const;

    /** Get the final route in the given subgraph. The route connects each variable
     *  to store to the root node in the value graph.
	 *  @param path The path index.
	 *  @param subgraph A subgraph represents a path in the CFG.
     *  @param valuesToRestore All variables to restore in the subgraph.
     *  @returns reversalRoute The search result.
	 */
    std::set<VGEdge> getReversalRoute(int dagIndex, int pathIndex,
                                   const SubValueGraph& subgraph,
                                   const std::vector<VGVertex>& valuesToRestore);

	void writeValueGraphNode(std::ostream& out, const VGVertex& node) const
	{
		out << "[label=\"" << valueGraph_[node]->toString() << "\"]";
	}

	void writeValueGraphEdge(std::ostream& out, const VGEdge& edge) const
	{
		out << "[label=\"" << valueGraph_[edge]->toString() << "\"]";
	}

	/** Add a new vertex to the value graph.
	 *
	 *  @param newNode A value graph node which will be added.
	 *  @returns The new added vertex.
	 */
	VGVertex addValueGraphNode(ValueGraphNode* newNode);

   	/** Add a new edge to the value graph.
	 *
	 *  @param src The source vertex.
	 *  @param tar The target vertex.
	 *  @param edgeToCopy This edge will be copied to the new edge.
	 *  @returns The new added edge.
	 */
    VGEdge addValueGraphEdge(VGVertex src, VGVertex tar, ValueGraphEdge* edgeToCopy = NULL);

    /** Add a new edge coming from a phi node to the value graph.
	 *
	 *  @param src The source vertex.
	 *  @param tar The target vertex.
	 *  @param cfgEdges CFG edges from which the path information is calculated.
	 */
    void addValueGraphPhiEdge(VGVertex src, VGVertex tar,
        const std::set<ReachingDef::FilteredCfgEdge>& cfgEdges);

	/** Add a new ordered edge to the value graph.
	 *
	 *  @param src The source vertex.
	 *  @param tar The target vertex.
	 *  @param index The index of the edge (for example, for a binary operator,
	 * 0 means lhs operand, and 1 means rhs operand).
	 *  @returns The new added edge.
	 */
	VGEdge addValueGraphOrderedEdge(VGVertex src, VGVertex tar, int index);
    
    void addValueGraphStateSavingEdges(VGVertex src, SgNode* killer);

    /** Add new state saving edges to the value graph. The target is the root.
	 *
	 *  @param src The source vertex.
     *  @param pathNum The visible incomplete path number on this edge.
	 *  @returns The new added edges.
	 */
    std::vector<VGEdge> addValueGraphStateSavingEdges(VGVertex src);

	//! Add a phi node to the value graph.
	//VGVertex createPhiNode(VersionedVariable& var);
    VGVertex createPhiNode(VersionedVariable& var, SSA::ReachingDefPtr reachingDef);

	//! Connect each variable node to the root with cost.
	void addStateSavingEdges();

    //! Check if a variable is a state variable.
	bool isStateVariable(const VarName& name)
	{ return stateVariables_.count(name) > 0; }

	/** Given a SgNode, return its variable name and version.
	 * 
	 *  @param node A SgNode which should be a variable (either a var ref or a declaration).
	 *  @param isUse Inidicate if the variable is a use or a def.
	 */
	VersionedVariable getVersionedVariable(SgNode* node, bool isUse = true);

    //! For each path, find its corresponding subgraph.
    std::set<VGEdge> getRouteFromSubGraph(int dagIndex, int pathIndex);

    //! Get all nodes in the topological order in a subgraph.
    std::vector<VGVertex> getGraphNodesInTopologicalOrder(
            const SubValueGraph& subgraph) const;

    //! Get all operands of an operator node.
    std::pair<ValueNode*, ValueNode*>
    getOperands(VGVertex opNode) const;

    //! Generate the reverse function.
    void generateReverseFunction(
        SgScopeStatement* scope,
        const SubValueGraph& route);

    //! Build the reverse CFG for the given DAG.
    void buildReverseCFG(int dagIndex, 
                         ReverseCFG& rvsCFG);
    
    //! Add a node to reverse CFG. Called by buildReverseCFG().
    void addReverseCFGNode(
        const PathSet& paths, const VGEdge* edge, ReverseCFG& rvsCFG,
        std::map<PathSet, RvsCFGVertex>& rvsCFGBasicBlock,
        std::map<PathSet, PathSet>& parentTable);
    
    //! Given a DAG index, return all edges of its reversal in the proper order.
    //! This order is decided by topological order from both CFG and route graph.
    void getRouteGraphEdgesInProperOrder(int dagIndex, std::vector<VGEdge>& result);

    //! Generate code in a basic block of the reverse CFG.
    void generateCodeForBasicBlock(
            const std::vector<VGEdge>& edges,
            SgScopeStatement* scope);

    void generateCode(
            size_t dagIndex,
            const ReverseCFG& rvsCFG,
            SgBasicBlock* rvsFuncBody,
            const std::string& pathNumName);
    
	static VGVertex nullVertex()
	{ return boost::graph_traits<ValueGraph>::null_vertex(); }

    /**************************************************************************/
    // The following functions are for debuging.

    void printVarVertexMap()
    {
        std::map<VersionedVariable, VGVertex>::iterator it, itEnd;
        for (it = varVertexMap_.begin(), itEnd = varVertexMap_.end();
                it != itEnd; ++it)
        {
            std::cout << it->first.toString() << "\n";
        }
    }

    std::string edgeToString(const VGEdge& edge)
    {
        VGVertex src = boost::source(edge, valueGraph_);
        VGVertex tgt = boost::target(edge, valueGraph_);

        return valueGraph_[src]->toString() + " ==> " +
               valueGraph_[tgt]->toString();
    }
};

} // End of namespace Backstroke



#endif // BACKSTROKE_VALUE_GRAPH
