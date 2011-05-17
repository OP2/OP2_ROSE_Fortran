
#include <iostream>
#include <sstream>

#include "rose_op2source.h"
#include "rose_op2parloop.h"
#include "rose_op2argument.h"

///////////////////////////// OPSource : Implementation /////////////////////////////////////////////////////////////////////

/*
 * Empty default constructor
 */
OPSource::OPSource() 
{  
}

/* 
 * Source needs to know which ROSE project it is working on
 */
void OPSource::setProject(SgProject *p)
{
  project = p;
}

/*
 *  Replace the op_par_loop with respective kernel function
 */
void OPSource::fixParLoops(SgNode *n)
{
  SgName kernel_name;
  SgFunctionCallExp *fn = isSgFunctionCallExp(n);
  if(fn != NULL)
  {
    string fn_name = fn->getAssociatedFunctionDeclaration()->get_name().getString();
    if(fn_name.compare("op_par_loop_2")==0 
		|| fn_name.compare("op_par_loop_3")==0 
		|| fn_name.compare("op_par_loop_4")==0
		|| fn_name.compare("op_par_loop_5")==0
  	|| fn_name.compare("op_par_loop_6")==0
		|| fn_name.compare("op_par_loop_7")==0
		|| fn_name.compare("op_par_loop_8")==0
		|| fn_name.compare("op_par_loop_9")==0) 
    {
      SgExprListExp* exprList = fn->get_args();
      SgExpressionPtrList &exprs = exprList->get_expressions();
      SgFunctionRefExp* varExp =  isSgFunctionRefExp(exprs[0]);
      if(varExp != NULL)
      {
      	kernel_name = varExp->get_symbol()->get_name();
      }
      exprs.erase(exprs.begin());

			SgExpressionPtrList::iterator it = exprs.begin() + op_par_loop::num_params - 1;
			for(; it != exprs.end(); it += op_argument::num_params)
			{
				*it = buildCastExp( *it, buildPointerType(SgClassType::createType( buildStructDeclaration("op_dat<void>"))) );
			}

			// Inject Name
			exprs.insert(exprs.begin(), buildStringVal(kernel_name));
			
			// Fetch the declaration
      SgName name = SgName("op_par_loop_") + kernel_name;
      SgFunctionDeclaration *funcDecl = cudaFunctionDeclarations[kernel_name];
      if(funcDecl)
      {
		  	SgFunctionRefExp* ref = isSgFunctionRefExp(fn->get_function());
		  	SgFunctionSymbol *symbol = ref->get_symbol();
		  	symbol->set_declaration(funcDecl);
		  	ref->set_symbol(symbol);
		  	fn->set_function(ref);
      }
    }
  }
}

/*
 *	Fix op structure calls and inject debug names
 */
void OPSource::fixOpFunctions(SgNode *n)
{
  SgName var_name;
  SgFunctionCallExp *fn = isSgFunctionCallExp(n);
  if(fn != NULL)
  {
    string fn_name = fn->getAssociatedFunctionDeclaration()->get_name().getString();
    if(fn_name.compare("op_decl_const")==0) 
    {
      SgExprListExp* exprList = fn->get_args();
			SgExpressionPtrList &exprs = exprList->get_expressions();
			if( isSgStringVal(exprs.back()) == NULL )
			{
		    SgVarRefExp* varExp = isSgVarRefExp(exprs[1]);
				if(!varExp)
				{
					varExp = isSgVarRefExp(isSgAddressOfOp(exprs[1])->get_operand_i());
				}

				if(varExp)
		    {
		    	var_name = varExp->get_symbol()->get_name();
		    }
				cout << "---Injecting Debug Name for const: " << var_name.getString() << "---" << endl;
		    exprList->append_expression(buildStringVal(var_name));
			}
    }
  }
}

/*
 *	Fix OP function calls and inject debug names
 */
void OPSource::fixOpStructs(SgNode *n)
{
  SgInitializedName* initname = isSgInitializedName(n);
  if(initname)
  {
    string var_name = initname->get_name().getString();
    SgConstructorInitializer *initer = isSgConstructorInitializer(initname->get_initializer());
    if(initer)
    {
      string class_name = initer->get_class_decl()->get_name().getString();
			//cout << "------------Constructors---------------------" << class_name << "---" << endl;
			if(class_name.find("op_dat") != string::npos
					|| class_name.find("op_dat_gbl") != string::npos
					|| class_name.compare("_op_ptr") == 0 
				  || class_name.compare("_op_set") == 0 
				  || class_name.compare("_op_dat_const") == 0)
			{
        cout << "---Injecting Debug Name: " << var_name << "---" << endl;
        SgExprListExp* list = initer->get_args();
				SgExpressionPtrList &exprs = list->get_expressions();
				if( isSgStringVal(exprs.back()) == NULL )
				{
        	list->append_expression(buildStringVal(var_name));
				}
      }
    }
  }
}

/*
 * The visit function is reimplemented from AstSimpleProcessing, and is called for every node
 * in the the AST. We are only really interested in looking at function calls to op_par_loop_3.
 */
void OPSource::visit(SgNode *n)
{ 
  SgFile *file = isSgFile(n);
  if(file!=NULL)
  {
	file->set_unparse_includes(false);
  }

  // We need to put the global scope on the scope stack so that we can look
  // up the oplus datatypes later on (in generateSpecial).
  SgGlobal *globalScope = isSgGlobal(n);
  if(globalScope!=NULL)
  {
    pushScopeStack(globalScope);
    
    // Add the global kernel header
    insertHeader("kernels.h", PreprocessingInfo::after, false, globalScope);
  }

  fixParLoops(n);
  fixOpStructs(n);
	fixOpFunctions(n);
}

/* 
 * Outputs each generated kernel file to disk.
 */
void OPSource::unparse()
{
  cout << "Running AST tests." << endl;
  AstTests::runAllTests(project);
  cout << "AST tests passed." <<endl;
  project->unparse();
  cout << "Program File Unparsed." << endl; 
}

/* 
 * Print out debug info at traversal end
 */
void OPSource::atTraversalEnd() 
{
  cout << "Traversal Ended." << endl;
}

