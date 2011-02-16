
/*
 * Written by Carlo Bertolli and Adam Betts
 *
 * This class stores all OP2 declared varibles in the context of a OP2 program
 * 
 *
 * This class extends AstSimpleProcessing since that class provides
 * utilities to traverse the Abstract Syntax Tree (AST).
 *
 */


#ifndef OP_DECLARED_VARIABLES_H
#define OP_DECLARED_VARIABLES_H

#include <rose.h>

#include "OP2CommonDefinitions.h"



class OpDeclaredVariables: public AstSimpleProcessing
{

private:

	/*
	 * The project which the source-to-source translator operates upon
	 */
	SgProject * project;	
	
	/*
	 * Actual arguments passed to every declaration of op_set in the program
	 */
	
	std::map < SgName, SgExpressionPtrList > declaredOpSets;
	
	/*
	 * Actual arguments passed to every declaration of op_map in the program
	 */
	
	std::map < SgName, SgExpressionPtrList > declaredOpMaps;
	/*
	 * Actual arguments passed to every declaration of op_dat in the program
	 */

	std::map < SgName, SgExpressionPtrList > declaredOpDats;
	
public:
	
	/*
	 * Empty constructor
	 */
	OpDeclaredVariables ( SgProject * _project ): project ( _project ) {}

	/*
	 * Empty destructor
	 */
	~OpDeclaredVariables () {}
	
	/*
	 * Modifiers (read-only, for robustness)
	 */
	std::map < SgName, SgExpressionPtrList >  getDeclaredOpSets () { return declaredOpSets; }
	
	std::map < SgName, SgExpressionPtrList >  getDeclaredOpMaps () { return declaredOpMaps; }

	std::map < SgName, SgExpressionPtrList >  getDeclaredOpDats () { return declaredOpDats; }

	SgExpressionPtrList getDeclaredOpSetArgs ( SgName * opSetName ) { return declaredOpSets[*opSetName]; }

	SgExpressionPtrList getDeclaredOpMapArgs ( SgName * opMapName ) { return declaredOpMaps[*opMapName]; }
	
	SgExpressionPtrList getDeclaredOpDatArgs ( SgName * opDatName ) { return declaredOpDats[*opDatName]; }
	
	/*
	 * Visit the program and fill up the private variables above
	 */
	void visit ( SgNode * currentNode );
	
};

#endif