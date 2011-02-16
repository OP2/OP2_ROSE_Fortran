
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

#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>


#include "OpDeclaredVariables.h"

namespace
{
  const std::string OP_DECL_SET_PREFIX = "op_decl_set";
  const std::string OP_DECL_MAP_PREFIX = "op_decl_map";
  const std::string OP_DECL_DAT_PREFIX = "op_decl_dat";
}



/*
 * ====================================================================================================
 * Constants with internal linkage
 * ====================================================================================================
 */



//!! WE NEED TO OBTAIN THE ARGUMENTS TO THE CALLS, NOT THE VARIABLE DECLARATIONS
//!! THEREFORE, WE NEED STL MAPS ASSOCIATING A SgVarRefExp TO A SgExpressionPtrList&


/*
 * Visit the program and fill up the private variables above
 */
void
OpDeclaredVariables::visit ( SgNode * currentNode )
{
	using std::cout;
	using std::endl;
	using std::string;
	using std::map;
	using std::pair;
	
//	using SageInterface::isSgFunctionCallExp;
	
	switch ( currentNode->variantT() ) {
	
		case V_SgFunctionCallExp:
		{
			
			cout << "Found a function call" << endl;
			
			/*
			 * Function call found in the AST
			 */
			SgFunctionCallExp* functionCallExp = isSgFunctionCallExp ( currentNode );
			ROSE_ASSERT (functionCallExp != NULL);
			
			const string calleeName =
													functionCallExp->getAssociatedFunctionDeclaration ()->get_name ().getString ();
			
			/*
			 * Check if it is a op_decl_set
			 */
			if (boost::starts_with (calleeName, OP_DECL_SET_PREFIX ))
			{
				
				/*
				 * Retrieve the arguments of the call
				 */
				SgExpressionPtrList& args =
				functionCallExp->get_args ()->get_expressions ();
				
				/*
				 * The parser has already checked that the number of arguments passed to op_decl_set is the correct one
				 */
					
				/*
				 * Add a new map associating the op_set variable reference with the arguments
				 */
				SgVarRefExp * setRef = isSgVarRefExp ( args[OP_DECL_SET_VAR_REF] );
				
				declaredOpSets.insert ( pair < SgName, SgExpressionPtrList > ( setRef->get_symbol()->get_name(),
																																			 args ) );
				
				cout << "Found a op_decl_set with name " << setRef->get_symbol()->get_name().getString() << endl;

			}
			
			/*
			 * Check if it is a op_decl_map
			 */
			if (boost::starts_with (calleeName, OP_DECL_MAP_PREFIX ))
			{
				
				/*
				 * Retrieve the arguments of the call
				 */
				SgExpressionPtrList& args =
				functionCallExp->get_args ()->get_expressions ();
			
				/*
				 * Add a new map associating the op_map variable reference with the arguments
				 */
				SgVarRefExp * mapRef = isSgVarRefExp ( args[OP_DECL_MAP_VAR_REF] );
				
				declaredOpMaps.insert ( pair < SgName, SgExpressionPtrList > ( mapRef->get_symbol()->get_name(),
																																			args ) );
				
				cout << "Found a op_decl_map with name " << mapRef->get_symbol()->get_name().getString() << endl;
			}
			
			/*
			 * Check if it is a op_decl_dat
			 */
			if (boost::starts_with (calleeName, OP_DECL_DAT_PREFIX ))
			{
				
				/*
				 * Retrieve the arguments of the call
				 */
				SgExpressionPtrList& args =
				functionCallExp->get_args ()->get_expressions ();
				
				
				/*
				 * Add a new map associating the op_dat variable reference with the arguments
				 */
				SgVarRefExp * datRef = isSgVarRefExp ( args[OP_DECL_DAT_VAR_REF] );
				
				declaredOpDats.insert ( pair < SgName, SgExpressionPtrList > ( datRef->get_symbol()->get_name(),
																																			args ) );
								
			}

			break;
			
		}
		default: {}// do not worry
	}
}
