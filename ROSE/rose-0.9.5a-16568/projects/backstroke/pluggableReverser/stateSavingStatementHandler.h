#ifndef STATESAVINGSTATEMENTHANDLER_H
#define	STATESAVINGSTATEMENTHANDLER_H

#include "handlerTypes.h"

/** TODO:
	-Correctly handle standard smart pointer types, such as auto_ptr, shared_ptr.
 */
class StateSavingStatementHandler : public StatementReversalHandler
{
	// A collection to avoid this handler calls itself by calling evaluateStatement function for the same statement.
	std::set<SgStatement*> evaluating_stmts_;

	//! Check if this handler can handle the given statement.
	bool checkStatement(SgStatement* stmt) const;

	//! Get all modified variables from the given statement. Note the returned defs don't contain variables
	//! declared inside of this statement, and if one variable is a member of another one, we just include the latter one.
	std::vector<VariableRenaming::VarName> getAllDefsAtNode(SgNode* node);
	
	//! Push/pop the given variable. 
	void saveOneVariable(const VariableRenaming::VarName& varName, SgBasicBlock* forwardBody, SgBasicBlock* reverseBody,
		SgBasicBlock* commitBody, const ClassHierarchyWrapper& classHierarchy);

	//! Generates the code that pops a value from the stack and assigns it to the given variable.
	SgExpression* restoreOneVariable(const VariableRenaming::VarName& varName, SgType* pushedType);
	
public:
    StateSavingStatementHandler() { name_ = "State Saving Statement Handler"; }

    virtual StatementReversal generateReverseAST(SgStatement* stmt, const EvaluationResult&);
    virtual std::vector<EvaluationResult> evaluate(SgStatement* stmt, const VariableVersionTable& var_table);
};

#endif	/* STATESAVINGSTATEMENTHANDLER_H */

