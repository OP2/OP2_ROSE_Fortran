
#include <iostream>
#include <sstream>

#include "rose_op2build.h"

///////////////////////////// OPBuild : Implementation /////////////////////////////////////////////////////////////////////

/*
 * Empty default constructor
 */
OPBuild::OPBuild() 
{  
}

/* 
 * 	Build needs to know which ROSE project it is working on
 */
void OPBuild::setProject(SgProject *p)
{
  project = p;
}

/*
 *	Build needs to know about all the kernels
 */
void OPBuild::setParLoop(OPParLoop* pl)
{
	this->pl = pl;
}

/*
 *	Get type from the varrefexp
 */
SgType* OPBuild::getTypeFromExpression(SgExpression* e)
{
	SgVarRefExp* ref = NULL;
	if(isSgAddressOfOp(e))
	{
		return isSgPointerType(isSgAddressOfOp(e)->get_type())->get_base_type();
	}
	return NULL;
}

/*
 *  Replace the op_par_loop with respective kernel function
 */
void OPBuild::generateBuildFile()
{
  // We build a new file for the CUDA kernel and its stub function
  string file_name = "op_par.cu";
  cout << "Generating OP Build File" << endl;
  
  SgSourceFile *file = isSgSourceFile(buildFile("blank.cpp", file_name, NULL));
  ROSE_ASSERT(file!=NULL);
  SgGlobal *globalScope = file->get_globalScope();
  
	// Add important files
	cout << "Adding includes and imports" << endl;
  addTextForUnparser(globalScope, "#include \"op_lib.cu\"\n", AstUnparseAttribute::e_before);
	addTextForUnparser(globalScope, "#import \"op_datatypes.cpp\"\n", AstUnparseAttribute::e_before);
  
	// Add shared const variables
	cout << "Adding shared const variables" << endl;
	for(int i = 0; i < sharedConstVariables.size() ; ++i)
  {
		SgVariableDeclaration* stmt2 = isSgVariableDeclaration(sharedConstVariables[i]);
		SgVariableDeclaration *varDec = buildVariableDeclaration(
																			stmt2->get_definition()->get_vardefn()->get_name(), 
																			stmt2->get_definition()->get_vardefn()->get_type(), 
																			NULL, 
																			globalScope);
		addTextForUnparser(varDec, "\n__constant__", AstUnparseAttribute::e_before);
		appendStatement(varDec,globalScope);
  }

	// Add generated kernel files
  map<string, SgFunctionDeclaration*>::iterator p;
  for(p=pl->cudaFunctionDeclarations.begin(); p != pl->cudaFunctionDeclarations.end() ; ++p)
  {
    addTextForUnparser(globalScope, "#include \"" + (*p).first + "_kernel.cu\"\n", AstUnparseAttribute::e_after);
  }
  
  // Add to list of files that need to be unparsed.
 	out_project = file->get_project();
	cout << "Finished generating OP Build File" << endl;
}

/*
 * The visit function is reimplemented from AstSimpleProcessing, and is called for every node
 * in the the AST. We are only really interested in looking at function calls to op_par_loop_3.
 */
void OPBuild::visit(SgNode *n)
{
  // We need to put the global scope on the scope stack so that we can look
  // up the oplus datatypes later on (in generateSpecial).
  SgGlobal *globalScope = isSgGlobal(n);
  if(globalScope!=NULL)
  {
    pushScopeStack(globalScope);
  }

	// Lookout for const declaraitons
	SgFunctionCallExp *fn = isSgFunctionCallExp(n);
  if(fn != NULL)
  {
    string fn_name = fn->getAssociatedFunctionDeclaration()->get_name().getString();
    if(fn_name.compare("op_decl_const")==0) 
    {
			SgExpressionPtrList& args = fn->get_args()->get_expressions();
      int dim = isSgIntVal(args[0])->get_value();

			SgVarRefExp* varRef = isSgVarRefExp(args[1]);
			if(!varRef)
			{
				SgAddressOfOp *e = isSgAddressOfOp(args[1]);
				varRef = isSgVarRefExp(e->get_operand_i());
			}
						
			if(varRef)
			{
				SgDeclarationStatement* decl = varRef->get_symbol()->get_declaration()->get_declaration();
				sharedConstVariables.push_back(decl);
				//cout << "***caching shared constant variable***" << sharedConstVariables.size() << endl;
			}
		}
	}
}

/* 
 * Outputs each generated kernel file to disk.
 */
void OPBuild::unparse()
{
	if(out_project)
		out_project->unparse();
	cout << "Finished UNPARSING" << endl;
}

/* 
 * Print out debug info at traversal end
 */
void OPBuild::atTraversalEnd() 
{
  cout << "Traversal Ended." << endl;
}

