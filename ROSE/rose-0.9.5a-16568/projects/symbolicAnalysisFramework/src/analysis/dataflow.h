#ifndef DATAFLOW_H
#define DATAFLOW_H

#include "analysisCommon.h"
#include "nodeState.h"
#include "functionState.h"
#include "analysis.h"
#include "lattice.h"

#include <boost/shared_ptr.hpp>
#include <vector>
#include <set>
#include <map>
#include <string>

// !!! NOTE: THE CURRENT INTER-/INTRA-PROCEDURAL ANALYSIS API EFFECTIVELY ASSUMES THAT EACH ANALYSIS WILL BE EXECUTED
// !!!       ONCE BECAUSE DURING A GIVEN ANALYSIS PASS THE INTRA- ANALYSIS MAY ACCUMULATE STATE AND THERE IS NO
// !!!       API FUNCTION THAT THE INTER- ANALYSIS CAN USE THE RE-INITIALIZE THE STATE OF THE INTRA- ANALYIS.

/*************************
 *** Dataflow Analyses ***
 *************************/
class InterProceduralDataflow;

class IntraProceduralDataflow : virtual public IntraProceduralAnalysis
{
        public:
        // generates the initial lattice state for the given dataflow node, in the given function, with the given NodeState
        //virtual std::vector<Lattice*> genInitState(const Function& func, const DataflowNode& n, const NodeState& state)=0;
        
        // !!! NOTE: THIS FUNCTION SHOULD BE AMENDED TO MAKE IT POSSIBLE TO SPECIFY THAT WE WANT THE INITIAL STATE
        // !!!       FOR ABOVE THIS NODE, BELOW THIS NODE OR A UNION OF BOTH. THIS IS IMPORTANT FOR LATTICES THAT
        // !!!       MAINTAIN DIFFERENT INFORMATION FOR THE DIFFERENT CASES, SUCH AS VarsExprsProductLattice, WHERE
        // !!!       DIFFERENT VARIABLES ARE LIVE BEFORE AND AFTER THE NODE. IN PARTICULAR, THIS WOULD BE USEFUL FOR
        // !!!       INTERPROCEDURAL ANALYSES WHERE THE SAME SgFunctionDefinition SgNode IS BOTH THE FIRST AND LAST
        // !!!       VirtualCFG NODE OF EACH FUNCTION WITH DIFFERENT INDEXES AND THE STATE BELOW IT CORRESPONDS TO THE
        // !!!       START OF THE FUNCTION AND THE STATE ABOVE IT CORRESPONDS TO THE END.
        virtual void genInitState(const Function& func, const DataflowNode& n, const NodeState& state,
                                  std::vector<Lattice*>& initLattices, std::vector<NodeFact*>& initFacts)=0;
        
                
        // Set of functions that have already been visited by this analysis, used
        // to make sure that the dataflow state of previously-visited functions is
        // not re-initialized when they are visited again.
        std::set<Function> visited;

        void setInterAnalysis(InterProceduralDataflow* interDataflowAnalysis)
        { this->interAnalysis = (InterProceduralAnalysis*)interDataflowAnalysis; }
        
        void setInterAnalysis(IntraProceduralDataflow* intraDFAnalysis)
        { this->interAnalysis = intraDFAnalysis->interAnalysis; }
        
        // Dataflow version of the function that intra-procedural analysis on the given function.
        // Takes in: 
        //    state - the function's NodeState
        //    analyzeDueToCallers - true if this function is analyzed due to changes in the the dataflow state from 
        //       its caller functions at their call sites to this function
        //    calleesUpdated - true if the function is analyzed due to changes of dataflow state of functions called
        //       by this function at their exit points (i.e. points where this state affects the caller)
        // Returns true if the function's NodeState gets modified as a result and false otherwise
        virtual bool runAnalysis(const Function& func, NodeState* state, bool analyzeDueToCallers, std::set<Function> calleesUpdated)=0;

        // Calls the full dataflow runAnalysis with dummy arguments to make it possible to use IntraProceduralDataflow
        // as if it were an IntraProceduralAnalysis     
        bool runAnalysis(const Function& func, NodeState* state)
        {
                // Each function is analyzed as if it were called directly by the language's runtime, ignoring 
                // the application's actual call graph
                bool analyzeDueToCallers = true; 
                
                // We ignore the application's call graph, so it doesn't matter whether this function calls other functions
                std::set<Function> calleesUpdated;
                
                return runAnalysis(func, state, analyzeDueToCallers, calleesUpdated);
        }
        
        InterProceduralDataflow* getInterAnalysis() const
        {
                return (InterProceduralDataflow*)interAnalysis;
        }
};

/// Apply an analysis A's transfer function at a particular AST node type
class IntraDFTransferVisitor : public ROSE_VisitorPatternDefaultBase
{
protected:
  // Common arguments to the underlying transfer function
  const Function &func;
  const DataflowNode &dfNode;
  NodeState &nodeState;
  const std::vector<Lattice*> &dfInfo;

public:

  IntraDFTransferVisitor(const Function &f, const DataflowNode &n, NodeState &s, const std::vector<Lattice*> &d)
    : func(f), dfNode(n), nodeState(s), dfInfo(d)
  { }

  virtual bool finish() = 0;
  virtual ~IntraDFTransferVisitor() { }
};

class IntraUnitDataflow : virtual public IntraProceduralDataflow
{
        public:
                
        // the transfer function that is applied to every node
        // n - the dataflow node that is being processed
        // state - the NodeState object that describes the state of the node, as established by earlier 
        //         analysis passes
        // dfInfo - the Lattices that this transfer function operates on. The function takes these lattices
        //          as input and overwrites them with the result of the transfer.
        // Returns true if any of the input lattices changed as a result of the transfer function and
        //    false otherwise.
        virtual bool transfer(const Function& func, const DataflowNode& n, NodeState& state, const std::vector<Lattice*>& dfInfo)=0;

  class DefaultTransfer : public IntraDFTransferVisitor
  {
    bool modified;
    IntraUnitDataflow *analysis;
  public:
    DefaultTransfer(const Function& func_, const DataflowNode& n_, NodeState& state_, const std::vector<Lattice*>& dfInfo_, IntraUnitDataflow *a)
      : IntraDFTransferVisitor(func_, n_, state_, dfInfo_), modified(false), analysis(a)
      { }

    
    void visit(SgNode *n) { modified = analysis->transfer(func, dfNode, nodeState, dfInfo); }
    bool finish() { return modified; }
  };
    
    virtual boost::shared_ptr<IntraDFTransferVisitor> getTransferVisitor(const Function& func, const DataflowNode& n,
                                                                  NodeState& state, const std::vector<Lattice*>& dfInfo)
  { return boost::shared_ptr<IntraDFTransferVisitor>(new DefaultTransfer(func, n, state, dfInfo, this)); }
};

class InterProceduralDataflow : virtual public InterProceduralAnalysis
{
        public:
        InterProceduralDataflow(IntraProceduralDataflow* intraDataflowAnalysis);
        
        // the transfer function that is applied to SgFunctionCallExp nodes to perform the appropriate state transfers
        // fw - =true if this is a forward analysis and =false if this is a backward analysis
        // n - the dataflow node that is being processed
        // state - the NodeState object that describes the dataflow state immediately before (if fw=true) or immediately after 
        //         (if fw=false) the SgFunctionCallExp node, as established by earlier analysis passes
        // dfInfo - the Lattices that this transfer function operates on. The function propagates them 
        //          to the calling function and overwrites them with the dataflow result of calling this function.
        // retState - Pointer reference to a Lattice* vector that will be assigned to point to the lattices of
        //          the function call's return value. The callee may not modify these lattices.
        // Returns true if any of the input lattices changed as a result of the transfer function and
        //    false otherwise.  
        virtual bool transfer(const Function& func, const DataflowNode& n, NodeState& state, 
                              const std::vector<Lattice*>& dfInfo, std::vector<Lattice*>** retState, bool fw)=0;
};

class InitDataflowState : public UnstructuredPassIntraAnalysis
{
        //std::vector<Lattice*> initState;
        IntraProceduralDataflow* dfAnalysis;
        
        public:
        InitDataflowState(IntraProceduralDataflow* dfAnalysis/*, std::vector<Lattice*> &initState*/)
        {
                this->dfAnalysis = dfAnalysis;
                //this->initState = initState;
        }
        
        void visit(const Function& func, const DataflowNode& n, NodeState& state);
};

// Analysis that finds the DataflowNodes that corresponds to calls to a given set of functions
class FindAllFunctionCalls : public UnstructuredPassIntraAnalysis
{
        // The set of functions that we wish to find the calls to
        const std::set<Function>& funcsToFind;
        
        // Maps each function in funcsToFind to a set of DataflowNodes that hold calls to this function
        std::map<Function, std::set<DataflowNode> > funcCalls;
        
        public:
        FindAllFunctionCalls(const std::set<Function>& funcsToFind): funcsToFind(funcsToFind)
        { }
        
        void visit(const Function& func, const DataflowNode& n, NodeState& state);
        
        // Returns a reference to funcCalls
        std::map<Function, std::set<DataflowNode> >& getFuncCalls() { return funcCalls; }
};

/* Base class of Uni-directional (Forward or Backward) Intra-Procedural Dataflow Analyses */
class IntraUniDirectionalDataflow : public IntraUnitDataflow
{
        public:

        // Runs the intra-procedural analysis on the given function and returns true if
        // the function's NodeState gets modified as a result and false otherwise
        // state - the function's NodeState
        bool runAnalysis(const Function& func, NodeState* state, bool analyzeDueToCallers, std::set<Function> calleesUpdated);
        
        protected:
        // propagates the dataflow info from the current node's NodeState (curNodeState) to the next node's 
        // NodeState (nextNodeState)
        bool propagateStateToNextNode(
             const std::vector<Lattice*>& curNodeState, DataflowNode curDFNode, int nodeIndex,
             const std::vector<Lattice*>& nextNodeState, DataflowNode nextDFNode);

        std::vector<DataflowNode> gatherDescendants(std::vector<DataflowEdge> edges,
                                                    DataflowNode (DataflowEdge::*edgeFn)() const);

        virtual NodeState*initializeFunctionNodeState(const Function &func, NodeState *fState) = 0;
        virtual VirtualCFG::dataflow*
          getInitialWorklist(const Function &func, bool firstVisit, bool analyzeDueToCallers, const set<Function> &calleesUpdated, NodeState *fState) = 0;
        virtual vector<Lattice*> getLatticeAnte(NodeState *state) = 0;
        virtual vector<Lattice*> getLatticePost(NodeState *state) = 0;

        // If we're currently at a function call, use the associated inter-procedural
        // analysis to determine the effect of this function call on the dataflow state.
        virtual void transferFunctionCall(const Function &func, const DataflowNode &n, NodeState *state) = 0;


        virtual vector<DataflowNode> getDescendants(const DataflowNode &n) = 0;
        virtual DataflowNode getUltimate(const Function &func) = 0;
};

/* Forward Intra-Procedural Dataflow Analysis */
class IntraFWDataflow  : public IntraUniDirectionalDataflow
{
        public:
        
        IntraFWDataflow()
        {}

        NodeState* initializeFunctionNodeState(const Function &func, NodeState *fState);
        VirtualCFG::dataflow*
          getInitialWorklist(const Function &func, bool firstVisit, bool analyzeDueToCallers, const set<Function> &calleesUpdated, NodeState *fState);
        vector<Lattice*> getLatticeAnte(NodeState *state);
        vector<Lattice*> getLatticePost(NodeState *state);
        void transferFunctionCall(const Function &func, const DataflowNode &n, NodeState *state);
        vector<DataflowNode> getDescendants(const DataflowNode &n);
        DataflowNode getUltimate(const Function &func);
};

/* Backward Intra-Procedural Dataflow Analysis */
class IntraBWDataflow  : public IntraUniDirectionalDataflow
{
        public:
        
        IntraBWDataflow()
        {}

        NodeState* initializeFunctionNodeState(const Function &func, NodeState *fState);
        VirtualCFG::dataflow*
          getInitialWorklist(const Function &func, bool firstVisit, bool analyzeDueToCallers, const set<Function> &calleesUpdated, NodeState *fState);
        virtual vector<Lattice*> getLatticeAnte(NodeState *state);
        virtual vector<Lattice*> getLatticePost(NodeState *state);
        void transferFunctionCall(const Function &func, const DataflowNode &n, NodeState *state);
        vector<DataflowNode> getDescendants(const DataflowNode &n);
        DataflowNode getUltimate(const Function &func);
};

/*// Dataflow class that maintains a Lattice for every currently live variable
class IntraFWPerVariableDataflow  : public IntraFWDataflow
{
        private:
        bool includeScalars;
        bool includeArrays;
        
                
        public:
        IntraFWPerVariableDataflow(bool includeScalars, bool includeArrays);
        
        // returns the set of global variables(scalars and/or arrays)
        varIDSet& getGlobalVars();
        
        // returns the set of variables(scalars and/or arrays) declared in this function
        varIDSet& getLocalVars(Function func);
        
        // returns the set of variables(scalars and/or arrays) referenced in this function
        varIDSet& getRefVars(Function func);
        
        // generates the initial variable-specific lattice state for a dataflow node
        virtual Lattice* genInitVarState(const Function& func, const DataflowNode& n, const NodeState& state)=0;
        
        // generates the initial non-variable-specific lattice state for a dataflow node
        virtual Lattice* genInitNonVarState(const Function& func, const DataflowNode& n, const NodeState& state)=0;
        
        // Generates a map of special constant variables (such as zeroVar) and the lattices that correspond to them
        // These lattices are assumed to be constants: it is assumed that they are never modified and it is legal to 
        //    maintain only one copy of each lattice may for the duration of the analysis.
        virtual std::map<varID, Lattice*> genConstVarLattices() const=0;
        
        private:
        // maps variables to the index of their respective Lattice objects in a given function
        std::map<Function, std::map<varID, int> > varLatticeIndex;
        // map of lattices that correspond to constant variables
        std::map<varID, Lattice*> constVarLattices;
        // =true if constVarLattices has been initialized and =false otherwise
        bool constVarLattices_init;
        
        public:
        // generates the initial lattice state for the given dataflow node, in the given function, with the given NodeState
        std::vector<Lattice*> genInitState(const Function& func, const DataflowNode& n, const NodeState& state);
        
        Lattice* getVarLattice(const Function& func, const varID& var, const std::vector<Lattice*>& dfInfo);
};*/

/******************************************************
 ***            printDataflowInfoPass               ***
 *** Prints out the dataflow information associated ***
 *** with a given analysis for every CFG node a     ***
 *** function.                                      ***
 ******************************************************/
class printDataflowInfoPass : public IntraFWDataflow
{
        Analysis* analysis;
        
        public:
        printDataflowInfoPass(Analysis *analysis)
        {
                this->analysis = analysis;
        }
        
        // generates the initial lattice state for the given dataflow node, in the given function, with the given NodeState
        //std::vector<Lattice*> genInitState(const Function& func, const DataflowNode& n, const NodeState& state);
        void genInitState(const Function& func, const DataflowNode& n, const NodeState& state,
                          std::vector<Lattice*>& initLattices, std::vector<NodeFact*>& initFacts);
        
        bool transfer(const Function& func, const DataflowNode& n, NodeState& state, const std::vector<Lattice*>& dfInfo);
};

/**********************************************************************
 ***                 UnstructuredPassInterDataflow                  ***
 *** The trivial inter-procedural dataflow where a intra-procedural ***
 *** dataflow analysis is executed once on every function in the    ***
 *** application, with no guarantees about how dataflow information ***
 *** is transmitted across function calls.                          ***
 **********************************************************************/
class UnstructuredPassInterDataflow : virtual public InterProceduralDataflow
{
        public:
        
        UnstructuredPassInterDataflow(IntraProceduralDataflow* intraDataflowAnalysis) 
                             : InterProceduralAnalysis((IntraProceduralAnalysis*)intraDataflowAnalysis), InterProceduralDataflow(intraDataflowAnalysis)
        {}
                
        // the transfer function that is applied to SgFunctionCallExp nodes to perform the appropriate state transfers
        // fw - =true if this is a forward analysis and =false if this is a backward analysis
        // n - the dataflow node that is being processed
        // state - the NodeState object that describes the dataflow state immediately before (if fw=true) or immediately after 
        //         (if fw=false) the SgFunctionCallExp node, as established by earlier analysis passes
        // dfInfo - the Lattices that this transfer function operates on. The function propagates them 
        //          to the calling function and overwrites them with the dataflow result of calling this function.
        // retState - Pointer reference to a Lattice* vector that will be assigned to point to the lattices of
        //          the function call's return value. The callee may not modify these lattices.
        // Returns true if any of the input lattices changed as a result of the transfer function and
        //    false otherwise.  
        bool transfer(const Function& func, const DataflowNode& n, NodeState& state, 
                      const std::vector<Lattice*>& dfInfo, std::vector<Lattice*>** retState, bool fw)
        { 
                return false;
        }
        
        void runAnalysis();
};

// Analysis that merges the dataflow states belonging to the given Analysis at all the return statements in the given function
// and produces a list of merged lattices (same number of lattices as were maintained by the nodes in the function).
// NOTE: The callers of this analysis are responsible for deallocating the lattices stored n mergedLatsRetStmt 
//       and mergedLatsRetVal at the end of the analysis pass.
class MergeAllReturnStates : public UnstructuredPassIntraAnalysis
{
        // Analysis whose states we'll be merging
        Analysis* analysis;
        
        // List of merged lattices of all the return statements and the returned values
        std::vector<Lattice*> mergedLatsRetStmt;
        std::vector<Lattice*> mergedLatsRetVal;
        
        public:
        //typedef enum ab {above=0, below=1};
        protected:
        //ab latSide;
        
        // After the analysis is complete, records true if the state of the mergedLattices changed
        // during the analysis and false otherwise
        bool modified;
        
        public:
        MergeAllReturnStates(Analysis* analysis/*, ab latSide*/): analysis(analysis)/*, latSide(latSide)*/
        { modified=false; }
        
        MergeAllReturnStates(Analysis* analysis, const std::vector<Lattice*>& mergedLatsRetStmt, const std::vector<Lattice*>& mergedLatsRetVal/*, ab latSide*/): 
                analysis(analysis), mergedLatsRetStmt(mergedLatsRetStmt), mergedLatsRetVal(mergedLatsRetVal)/*, latSide(latSide)*/
        { modified=false; }
        
        void visit(const Function& func, const DataflowNode& n, NodeState& state);
        
        // Merges the lattices in the given vector into mergedLat, which may be mergedLatsRetStmt or mergedLatsRetVal
        // Returns true of mergedLatsStmt changes as a result and false otherwise.
        static bool mergeLats(std::vector<Lattice*>& mergedLat, const std::vector<Lattice*>& lats);
        
        // Returns a reference to mergedLatsRetStmt
        std::vector<Lattice*>& getMergedLatsRetStmt() { return mergedLatsRetStmt; }
        
        // Returns a reference to mergedLatsRetVal
        std::vector<Lattice*>& getMergedLatsRetVal() { return mergedLatsRetVal; }
        
        // Returns the value of modified
        bool getModified() { return modified; }
        
        // Deallocates all the merged lattices
        ~MergeAllReturnStates();
};

// A NodeFact associated with a FunctionState that stores the merge of the lattices immediately 
// above all return statements in a given function. 
class DFStateAtReturns : public NodeFact
{
        // The dataflow state at the end of the function, merged over all the return statements
        // and the implicit return at the end of the function
        std::vector<Lattice*>& latsAtFuncReturn;
        // The dataflow state of the return value, merged over all the return statements
        std::vector<Lattice*>& latsRetVal;
        
        public:
        //DFStateAtReturns();
        
        DFStateAtReturns(std::vector<Lattice*>& latsAtFuncReturn, std::vector<Lattice*>& latsRetVal);
        
        // Returns a copy of this node fact
        NodeFact* copy() const;
        
        // Applies the MergeAllReturnStates analysis on the given function, incorporating the results into
        // the lattices held by this object.
        // Returns true of the lattices change as a result and false otherwise.
        bool mergeReturnStates(const Function& func, FunctionState* fState, IntraProceduralDataflow* intraAnalysis);
        
        // Returns a reference to latsAtFuncReturn
        std::vector<Lattice*>& getLatsAtFuncReturn() { return latsAtFuncReturn; }
        
        // Returns a reference to latsRetVal
        std::vector<Lattice*>& getLatsRetVal() { return latsRetVal; }
        
        std::string str(std::string indent);
};

class ContextInsensitiveInterProceduralDataflow : virtual public InterProceduralDataflow, public TraverseCallGraphDataflow
{
        // list of functions that still remain to be processed
        //list<Function> remaining;
        
        // The functions that still remain to be processed.
        
        // These functions need to be processed because they are called by functions that have been processed
        // or are called at startup such as main() and the constructors of static objects.
        std::set<Function> remainingDueToCallers;
        
        // Each function F in this map needs to be processed because it has called other functions and those functions 
        // have now been analyzed and the dataflow information at their exit points has changed since the last time 
        // F was analyzed. remainingDueToCalls maps each F to all such functions. As such, F needs to be re-analyzed, 
        // starting at the calls to these functions.
        std::map<Function, std::set<Function> > remainingDueToCalls;
        
        public:
        ContextInsensitiveInterProceduralDataflow(IntraProceduralDataflow* intraDataflowAnalysis, SgIncidenceDirectedGraph* graph) ;
        
        public:

        // the transfer function that is applied to SgFunctionCallExp nodes to perform the appropriate state transfers
        // fw - =true if this is a forward analysis and =false if this is a backward analysis
        // n - the dataflow node that is being processed
        // state - the NodeState object that describes the dataflow state immediately before (if fw=true) or immediately after 
        //         (if fw=false) the SgFunctionCallExp node, as established by earlier analysis passes
        // dfInfo - the Lattices that this transfer function operates on. The function propagates them 
        //          to the calling function and overwrites them with the dataflow result of calling this function.
        // retState - Pointer reference to a Lattice* vector that will be assigned to point to the lattices of
        //          the function call's return value. The callee may not modify these lattices.
        // Returns true if any of the input lattices changed as a result of the transfer function and
        //    false otherwise.  
        bool transfer(const Function& func, const DataflowNode& n, NodeState& state, 
                      const std::vector<Lattice*>& dfInfo, std::vector<Lattice*>** retState, bool fw);
        
        // Uses TraverseCallGraphDataflow to traverse the call graph.
        void runAnalysis();
        
        // Runs the intra-procedural analysis every time TraverseCallGraphDataflow passes a function.
        void visit(const CGFunction* func);
};

#endif
