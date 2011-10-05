#ifndef BACKSTROKE_EVENTPROCESSOR_H
#define	BACKSTROKE_EVENTPROCESSOR_H

#include <rose.h>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include "variableVersionTable.h"
#include "handlerTypes.h"
#include "utilities/types.h"
#include "ssa/staticSingleAssignment.h"


class VariableRenaming;

class IVariableFilter
{
public:
	/** Given a variable declaration, return true if it's a variable of interest, false if the variable
	 * should be ignored for purposes of reverse computation. Examples of variables to be ignored are
	 * management variables such as an event scheduling queue, the standard output stream, etc. */
	virtual bool isVariableInteresting(const VariableRenaming::VarName& var) const = 0;
};

/** Class that handles reversing an event. All expression handlers and statement handlers are registered with this
 * class. */
class EventProcessor
{
	//! This is the current event function to handle.
	SgFunctionDeclaration* event_;

	//! All expression handlers which are added by the user.
	std::vector<ExpressionReversalHandler*> exp_handlers_;

	//! All statement handlers which are added by the user.
	std::vector<StatementReversalHandler*> stmt_handlers_;

	/** Handlers which can restore a variable value without state saving. */
	std::vector<VariableValueRestorer*> variableValueRestorers;

	//! All declarations of stacks which store values of different types.
	std::map<std::string, SgVariableDeclaration*> stack_decls_;

	//! The variable renaming analysis object.
	VariableRenaming* var_renaming_;

	//! Interprocedural SSA analysis
	StaticSingleAssignment* interproceduralSsa_;

	//! This set is used to prevent infinite recursion when calling restoreVariable.
	std::set<std::pair<VariableRenaming::VarName, VariableRenaming::NumNodeRenameEntry> > activeValueRestorations;

	//! Objects that determines which variables don't ever need to be reversed. E.g. management objects
	IVariableFilter* variableFilter_;

	//! Make those two classes the friends to let them use some private methods.
	friend class ReversalHandlerBase;

private:

	//! Given an expression, return all evaluation results using all expression handlers.
	std::vector<EvaluationResult> evaluateExpression(SgExpression* exp, const VariableVersionTable& var_table, bool is_value_used);

	//! Given a statement, return all evaluation results using all statement handlers.
	std::vector<EvaluationResult> evaluateStatement(SgStatement* stmt, const VariableVersionTable& var_table);

	//! Given a set of results, if two results of them have the same variable table, we remove
	//! the one which has the higher cost.
	std::vector<EvaluationResult> filterResults(const std::vector<EvaluationResult>& results);

	//! The following methods are for expression and statement handlers for store and restore.
	SgVarRefExp* getStackVar(SgType* type);
	SgExpression* pushVal(SgExpression* exp, SgType* type);

	//! Generate an expression which pops the top of a stack and returns the value
	SgExpression* popVal(SgType* type);

	//! Generate an expression which pops the bottom of a stack and discards the value.
	//! This is used for fossil collection
	SgExpression* popVal_front(SgType* type);

	SgExpression* cloneValueExp(SgExpression* value, SgType* type);

	SgExpression* assignPointerExp(SgExpression* lhs, SgExpression* rhs, SgType* lhsType, SgType* rhsType);
	
public:

	EventProcessor(IVariableFilter* varFilter = NULL);

	//! Add an expression handler to the pool of expression handlers.
	void addExpressionHandler(ExpressionReversalHandler* exp_handler);

	//! Add a statement handler to the pool of statement handlers
	void addStatementHandler(StatementReversalHandler* stmt_handler);

	//! Add a value extractor to the pool of variable value restorers
	void addVariableValueRestorer(VariableValueRestorer* restorer);

	//! Returns three functions: the instrumented forward event, the reverse event, and the commit method
	std::vector<EventReversalResult> processEvent();

	//! Returns three functions: the instrumented forward event, the reverse event, and the commit method
	std::vector<EventReversalResult> processEvent(SgFunctionDeclaration* event);

	//! Return if the given variable is a state variable (currently we assume all variables except
	//! those defined inside the event function are state varibles).
	bool isStateVariable(SgExpression* exp);
	bool isStateVariable(const VariableRenaming::VarName& var);

	//! Check if every state variable in the given variable version table has the initial version
	//! which should be 1.
	bool checkForInitialVersions(const VariableVersionTable& var_table);

	//! Get all declarations of stacks which store values of different types.
	std::vector<SgVariableDeclaration*> getStackDeclarationsForLastEvent() const;

	void setVariableRenaming(VariableRenaming* var_renaming)
	{ var_renaming_ = var_renaming; }

	VariableRenaming* getVariableRenaming() const
	{ return var_renaming_;	}

	void setInterproceduralSsa(StaticSingleAssignment* ssa)
	{ interproceduralSsa_ = ssa; }

	const StaticSingleAssignment* getInterproceduralSsa() const
	{ return interproceduralSsa_; }

	/**
	* Given a variable and a version, returns an expression evaluating to the value of the variable
	* at the given version.
	*
	* @param variable name of the variable to be restored
	* @param availableVariables variables whos values are currently available
	* @param definition the version of the variable which should be restored
	* @return NULL on failure; else an expression that evaluates to the value of the variable
	*/
	SgExpression* restoreVariable(VariableRenaming::VarName variable, const VariableVersionTable& availableVariables,
			VariableRenaming::NumNodeRenameEntry definitions);

	//! Restores the value of an expression given a set of currently available variables. For example, if the
	//! expression is (a + b), the values of a and b will be extracted from the currently available variables, and then
	//! the expression val(a) + val(b) will be returned.
	//!
	//! @returns expression evaluating to the same value as the original, or NULL on failure
	SgExpression* restoreExpressionValue(SgExpression* expression, const VariableVersionTable& availableVariables);
};


#endif	/* BACKSTROKE_EVENTPROCESSOR_H */

