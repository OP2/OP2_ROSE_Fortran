
#include <iostream>
#include <sstream>

#include "rose_op2parloop.h"


const int op_par_loop::num_params = 2;



///////////////////////////// Utility string functions for creating argument names /////////////////////////////////////////

inline string buildStr(int i)
{
  stringstream s;
  s << i;
  return s.str();
}

inline string arg(int i)
{
  return "arg"+buildStr(i);
}

inline string argLocal(int i)
{
  return arg(i)+"_l";
}

///////////////////////////// op_par_loop : Implementation ///////////////////////////////////////////////////////////////////

/*
 *	Update container
 */
void op_par_loop::updatePlanContainer(op_argument* argument)
{
	if(argument->usesIndirection())
	{
		string cur_name = argument->arg->get_symbol()->get_name().getString();
		if(prev_name.compare(cur_name) != 0)
		{
				planContainer.push_back(argument);
				prev_name = cur_name;
		}
		argument->plan_index = planContainer.size()-1;
	}
}


///////////////////////////// OPParLoop : Implementation /////////////////////////////////////////////////////////////////////

/*
 * Empty default constructor
 */
OPParLoop::OPParLoop() 
{
  
}

/* 
 * ParLoop needs to know which ROSE project it is working on
 */
void OPParLoop::setProject(SgProject *p)
{
  project = p;
}

/*
 * The visit function is reimplemented from AstSimpleProcessing, and is called for every node
 * in the the AST. We are only really interested in looking at function calls to op_par_loop_3.
 */
void OPParLoop::visit(SgNode *n) 
{
  // We need to put the global scope on the scope stack so that we can look
  // up the oplus datatypes later on (in generateSpecial).
  SgGlobal *globalScope = isSgGlobal(n);
  if(globalScope!=NULL)
    pushScopeStack(globalScope);

  SgFunctionCallExp *fn = isSgFunctionCallExp(n);
  if(fn != NULL)
  {
    string fn_name = fn->getAssociatedFunctionDeclaration()->get_name().getString();
    if(   fn_name.compare("op_par_loop_2")==0
       || fn_name.compare("op_par_loop_3")==0
			 || fn_name.compare("op_par_loop_4")==0 
			 || fn_name.compare("op_par_loop_5")==0
			 || fn_name.compare("op_par_loop_6")==0
			 || fn_name.compare("op_par_loop_7")==0
			 || fn_name.compare("op_par_loop_8")==0
			 || fn_name.compare("op_par_loop_9")==0
			)
    {
      cout << "Located Function " << fn_name << endl;
      SgExpressionPtrList& args = fn->get_args()->get_expressions();
      
      // We parse the arguments to the op_par_loop_3 call into our internal 
      // representation that is a little more convenient for later on.
      SgExpressionPtrList::iterator i=args.begin();
      op_par_loop *pl = new op_par_loop();
      pl->kernel = isSgFunctionRefExp(*i++);
      //pl->label = isSgStringVal(*i++)->get_value();
      pl->set = isSgVarRefExp(*i++);
      
      // Calculate number of args = total - 3 i.e. kernel, label and set and create arg objects
      int numArgs = (args.size() - op_par_loop::num_params) / op_argument::num_params;
      for(int j=0; j<numArgs; j++)
      {
				op_argument* parg = new op_argument(i);
				parg->own_index = pl->args.size();
				pl->updatePlanContainer(parg);
				
				pl->args.push_back(parg);
				if(parg->usesIndirection())
					pl->ind_args.push_back(parg);
      }

      // Generate kernels
      if(pl->numIndArgs() == 0)
        generateSpecial(fn, pl);
      else
        generateStandard(fn, pl);
    }
  }
}

/* 
 * Outputs each generated kernel file to disk.
 */
void OPParLoop::unparse()
{
  for(vector<SgProject*>::iterator i=kernels.begin(); i!=kernels.end(); ++i)
  {  
    cout << "Running AST tests." << endl;
    AstTests::runAllTests(*i);
    cout << "AST tests passed." <<endl;
    (*i)->unparse();
    cout << "Unparsed." << endl;
  }
}

/*
 *  Generate global kernel header
 */
void OPParLoop::generateGlobalKernelsHeader()
{
  // We build a new file for the CUDA kernel and its stub function
  string file_name = "kernels.h";
  cout << "Generating CUDA Kernels File" << endl;
  
  SgSourceFile *file = isSgSourceFile(buildFile("blank.cpp", file_name, NULL));
  ROSE_ASSERT(file!=NULL);
  SgGlobal *globalScope = file->get_globalScope();
  
  addTextForUnparser(globalScope, "#ifndef OP_KERNELS\n", AstUnparseAttribute::e_before);
  addTextForUnparser(globalScope, "#define OP_KERNELS\n", AstUnparseAttribute::e_before);
  addTextForUnparser(globalScope, "#include \"op_datatypes.h\"\n", AstUnparseAttribute::e_before);
  
  map<string, SgFunctionDeclaration*>::iterator p;
  for(p=cudaFunctionDeclarations.begin(); p != cudaFunctionDeclarations.end() ; ++p)
  {
    SgFunctionDeclaration* copy = (*p).second;
    SgFunctionDeclaration* d = buildNondefiningFunctionDeclaration(copy, globalScope);
    appendStatement(d,globalScope);
  }
  
  addTextForUnparser(globalScope, "#endif\n", AstUnparseAttribute::e_after);
  

  // Add to list of files that need to be unparsed.
  kernels.push_back(file->get_project());
}

/*
 * Returns the name of the function pointed to by a function pointer.
 */
inline string OPParLoop::getName(SgFunctionRefExp *fn)
{
  return fn->get_symbol_i()->get_name().getString();
}

/* 
 * Print out debug info at traversal end
 */
void OPParLoop::atTraversalEnd() 
{
  cout << "Traversal Ended." << endl;
}


/*
 *	Generate Seperate File For the Special Kernel
 *	---------------------------------------------
 */ 
void OPParLoop::generateSpecial(SgFunctionCallExp *fn, op_par_loop *pl)
{
  // We build a new file for the CUDA kernel and its stub function
  string kernel_name = getName(pl->kernel);
  string file_name = kernel_name + "_kernel.cu";
  cout << "Generating (Special) CUDA Parallel Loop File for " << kernel_name << endl;
  
  SgSourceFile *file = isSgSourceFile(buildFile("blank.cpp", file_name, NULL));
  ROSE_ASSERT(file!=NULL);
  SgGlobal *globalScope = file->get_globalScope();
  
	addTextForUnparser(globalScope,"#include \"user_defined_types.h\"\n",AstUnparseAttribute::e_before);
  addTextForUnparser(globalScope,"#include \"op_datatypes.h\"\n",AstUnparseAttribute::e_before);
  addTextForUnparser(globalScope,"#include \"kernels.h\"\n\n",AstUnparseAttribute::e_before);
	
    
  // In order to build the prototype for the plan function, we need to get hold of the types 
  // that we intend to pass it. Since these are declared in op_datatypes.h, we need to 
  // loop them up before we can use them.
  SgFunctionParameterTypeList *paramTypes = buildFunctionParameterTypeList();
  SgType *op_set, *op_dat, *op_ptr, *op_access, *op_datatype, *op_plan;
  op_set = lookupNamedTypeInParentScopes("op_set");
  op_dat = SgClassType::createType(buildStructDeclaration(SgName("op_dat<void>"), globalScope));
  op_ptr = lookupNamedTypeInParentScopes("op_ptr");
  op_access = lookupNamedTypeInParentScopes("op_access");
  op_plan = lookupNamedTypeInParentScopes("op_plan");

  // Now we do forward declaration of the external functions
  forwardDeclareUtilFunctions(globalScope, op_set, op_dat, op_ptr, op_access, op_plan);

  // C kernel prefixed with the __device__ keyword. However, we could copy the AST
  // subtree representing the kernel into the new file, which would remove the
  // requirement for writing each kernel in a separate .h file.
  addTextForUnparser(globalScope, "\n\n__device__\n#include <"+kernel_name+".h>\n",AstUnparseAttribute::e_before);

	
	// 1 FUNCTION DEFINITION <START>
  // =======================================

  // We need to build a list of parameters for our __global__ function,
  // based on the arguments given to op_par_loop_3 earlier:
  SgFunctionParameterList *paramList = buildFunctionParameterList();
  SgInitializedName *initName;
	bool reduction_required = false;
  for(int i=0; i<pl->numArgs(); i++)
  {
		if(pl->args[i]->consideredAsReduction())
			reduction_required = true;
    SgInitializedName *name;
    SgType *argType = buildPointerType(pl->args[i]->type);
    name = buildInitializedName(arg(i), argType);
    paramList->append_arg(name);
  }
  SgInitializedName *set_size = buildInitializedName("set_size", buildIntType());
  appendArg(paramList, set_size);
	for(int i=0; i<pl->numArgs(); i++)
  {
		if(pl->args[i]->consideredAsReduction())
		{
			SgInitializedName *block_reduct = buildInitializedName("block_reduct" + buildStr(i), buildPointerType(buildVoidType()));
			appendArg(paramList, block_reduct);
		}
	}
  
  // We can build the __global__ function using the parameter list and add it to our new file. We get a reference to
  // the body of this function so that we can add code to it later on.
  SgFunctionDeclaration *func = buildDefiningFunctionDeclaration("op_cuda_"+kernel_name, buildVoidType(), paramList, globalScope);
  addTextForUnparser(func,"\n\n__global__",AstUnparseAttribute::e_before);
  appendStatement(func, globalScope);
  SgBasicBlock *body = func->get_definition()->get_body();

  // We Add the declarations of local variables first.
  for(int i=0; i<pl->numArgs(); i++)
  {
		if(pl->args[i]->isGlobal())
		{
    	SgVariableDeclaration *varDec;
    	varDec = buildVariableDeclaration(argLocal(i), buildArrayType(pl->args[i]->type, buildIntVal(pl->args[i]->dim)), NULL, body);
    	appendStatement(varDec,body);
		}
  }

	
  // 2 HANDLE GLOBAL DATA <TRANSFER TO DEVICE>
  // =======================================
	preKernelGlobalDataHandling(fn, pl, body);


  // 3 MAIN EXECUTION LOOP <BEGIN>
  // =======================================
	//cout << "MAIN EXECUTION LOOP <BEGIN>" << endl;
  SgScopeStatement *loopBody = buildBasicBlock();
  SgExpression *rhs = buildAddOp(buildOpaqueVarRefExp("threadIdx.x"), buildMultiplyOp(buildOpaqueVarRefExp("blockIdx.x"), buildOpaqueVarRefExp("blockDim.x")));
  SgVariableDeclaration *loopVarDec = buildVariableDeclaration(SgName("n"), buildIntType(), buildAssignInitializer(rhs), loopBody);
  SgName loopVar = loopVarDec->get_definition()->get_vardefn()->get_name();

  SgExpression *lhs = buildVarRefExp(loopVar, loopBody);
  rhs = buildOpaqueVarRefExp(SgName("set_size"));
  SgExprStatement *test = buildExprStatement(buildLessThanOp(lhs, rhs));

  rhs = buildMultiplyOp(buildOpaqueVarRefExp("blockDim.x"), buildOpaqueVarRefExp("gridDim.x"));
  SgPlusAssignOp *increment = buildPlusAssignOp(lhs, rhs);
  SgForStatement *forLoop = buildForStatement(loopVarDec, test, increment, loopBody);

  // 3.1 FIRE KERNEL
  // ===============================================
	//cout << "FIRE KERNEL" << endl;
  // Next we build a call to the __device__ function. We build the parameters
  // for this call first, then the call, and add it into the outer loop body.
  SgExprListExp *kPars = buildExprListExp();
  for(int i=0; i<pl->numArgs(); i++)
  {
		if(pl->args[i]->isNotGlobal())
		{
			SgExpression *e = buildAddOp(buildOpaqueVarRefExp(arg(i)), buildMultiplyOp(buildOpaqueVarRefExp(loopVar), buildIntVal(pl->args[i]->dim)));
    	kPars->append_expression(e);
		}
		else
		{
			SgExpression *e = buildOpaqueVarRefExp(argLocal(i), body);
    	kPars->append_expression(e);
		}
  }
  SgExprStatement *uf = buildFunctionCallStmt(SgName(kernel_name), buildVoidType(), kPars);
  appendStatement(uf,loopBody);

  // 3 MAIN EXECUTION LOOP <END>
  // =======================================
	//cout << "MAIN EXECUTION LOOP <END>" << endl;
  // Now we have completed the body of the outer for loop, we can build an initialiser, 
  // an increment and a test statement. The we insert this loop into the __gloabl__ function.
  // Because threadIdx.x etc are not really variables, we invent "opaque" variables with these
  // names.
  appendStatement(forLoop,body);
  

  // 4 HANDLE GLOBAL DATA <TRANSFER FROM DEVICE>
  // =======================================
	postKernelGlobalDataHandling(fn, pl, body);


	// 1 FUNCTION DEFINITION <END>
  // =======================================	



	// -----------------------------------------------------------------------------------------------
	// Reductions require additional kernel launch - create that definition
	if(reduction_required)
	{
		paramList = buildFunctionParameterList();
		SgInitializedName *grid_size = buildInitializedName("gridsize", buildIntType());
		appendArg(paramList, grid_size);
		for(int i=0; i<pl->numArgs(); i++)
		{
			if(pl->args[i]->consideredAsReduction())
			{
				SgType *argType = buildPointerType(pl->args[i]->type);
				SgInitializedName *name = buildInitializedName(arg(i), argType);
				paramList->append_arg(name);

				SgInitializedName *block_reduct = buildInitializedName("block_reduct" + buildStr(i), buildPointerType(buildVoidType()));
				appendArg(paramList, block_reduct);
			}
		}

		// We can build the __global__ function using the parameter list and add it to our new file. We get a reference to
		// the body of this function so that we can add code to it later on.
		func = buildDefiningFunctionDeclaration("op_cuda_"+kernel_name+"_reduction", buildVoidType(), paramList, globalScope);
		addTextForUnparser(func,"\n\n__global__",AstUnparseAttribute::e_before);
		appendStatement(func, globalScope);
		body = func->get_definition()->get_body();

		//cout << "HANDLE REDUCT DATA <TRANSFER FROM DEVICE>" << endl;
		for(int i=0; i<pl->numArgs(); i++)
		{
			if(pl->args[i]->consideredAsReduction())
			{
				// Create loop
				SgStatement *viInit = buildVariableDeclaration(SgName("d"), buildIntType(), buildAssignInitializer(buildIntVal(0)));
		    SgName indVar = isSgVariableDeclaration(viInit)->get_definition()->get_vardefn()->get_name();
					    
		    // Call function
				SgExprListExp *kPars2 = buildExprListExp();
				SgExpression *e = buildOpaqueVarRefExp(arg(i));
				e = buildAddOp(e, buildVarRefExp(indVar));
				kPars2->append_expression(e);
				e = buildOpaqueVarRefExp(SgName("block_reduct" + buildStr(i)));
				kPars2->append_expression(e);
				e = buildOpaqueVarRefExp(SgName("gridsize"));
				kPars2->append_expression(e);

				SgExprStatement *uf1 = NULL;
				switch(pl->args[i]->access)
				{
					case OP_INC:
						uf1 = buildFunctionCallStmt(SgName("op_reduction2_2<OP_INC>"), buildVoidType(), kPars2);
						break;
					case OP_MAX:
						uf1 = buildFunctionCallStmt(SgName("op_reduction2_2<OP_MAX>"), buildVoidType(), kPars2);
						break;
					case OP_MIN:
						uf1 = buildFunctionCallStmt(SgName("op_reduction2_2<OP_MIN>"), buildVoidType(), kPars2);
						break;
				}

				// build test and increment, and add the loop into the body of the inner loop.
				SgScopeStatement *viLoopBody = buildBasicBlock(uf1);
		    SgExprStatement *viTest = buildExprStatement(buildLessThanOp(buildOpaqueVarRefExp(indVar), buildIntVal(pl->args[i]->dim)));
		    SgPlusPlusOp *viIncrement = buildPlusPlusOp(buildOpaqueVarRefExp(indVar));
		    SgStatement *viForLoop = buildForStatement(viInit, viTest, viIncrement, viLoopBody);
		    appendStatement(viForLoop,body);
			}
		}
	}
  
  
	








	

  // The following code builds the stub function. A little of the code that is output by
  // op2.m is not generated here, simply as it is not necessary and I was running out of
  // time before the meeting on 27 Jan. (cudaThreadSynchronize and cudaGetLastError, and
  // some debugging statements are missing, but would be trivial to add).
  // As usual we build a list of parameters for the function.
  paramList = buildFunctionParameterList();
  SgInitializedName *name = buildInitializedName(SgName("name"), buildPointerType(buildConstType(buildCharType())));
  appendArg(paramList, name);
  name = buildInitializedName(SgName("set"), op_set);
  appendArg(paramList, name);
  // Add other arguments
  for(int i=0; i<pl->numArgs(); i++)
  {
    name = buildInitializedName(SgName("arg"+buildStr(i)), buildPointerType(op_dat));
    appendArg(paramList, name);
    name = buildInitializedName(SgName("idx"+buildStr(i)), buildIntType());
    appendArg(paramList, name);
    name = buildInitializedName(SgName("ptr"+buildStr(i)), buildPointerType(op_ptr));
    appendArg(paramList, name);
    //name = buildInitializedName(SgName("dim"+buildStr(i)), buildIntType());
    //appendArg(paramList, name);
    name = buildInitializedName(SgName("acc"+buildStr(i)), op_access);
    appendArg(paramList, name);
  }

  // We build the function with the parameter list and insert it into the global
  // scope of our file as before.
  string kernelFuncName = "op_par_loop_" + kernel_name;
  func = buildDefiningFunctionDeclaration(kernelFuncName, buildFloatType(), paramList, globalScope);
  cudaFunctionDeclarations.insert(pair<string, SgFunctionDeclaration*>(kernel_name, func));
  appendStatement(func, globalScope);
  body = func->get_definition()->get_body();

	// Declare gridsize and bsize
	SgExpression *e = buildOpaqueVarRefExp(SgName("BSIZE"));
	SgVariableDeclaration *varDec = buildVariableDeclaration(SgName("bsize"), buildIntType(), buildAssignInitializer(e));
	appendStatement(varDec, body);
	
	e = buildOpaqueVarRefExp(SgName("set.size"));
	e = buildSubtractOp(e, buildIntVal(1));
	e = buildDivideOp(e, buildOpaqueVarRefExp(SgName("bsize")));
	e = buildAddOp(e, buildIntVal(1));
	varDec = buildVariableDeclaration(SgName("gridsize"), buildIntType(), buildAssignInitializer(e));
	appendStatement(varDec, body);

	/*
  by Zohirul : PRE-Handle const and global data
  */
  preHandleConstAndGlobalData(fn, pl, body);

  // Add the timer block <start>
	//-----------------------------------------
  /*
  float elapsed_time_ms=0.0f;
  cudaEvent_t start, stop;
  cudaEventCreate( &start );
  cudaEventCreate( &stop  );
  cudaEventRecord( start, 0 );
	*/
  //-----------------------------------------
	varDec = buildVariableDeclaration(SgName("elapsed_time_ms"), buildFloatType(), buildAssignInitializer(buildFloatVal(0.0f)), body);
	addTextForUnparser(varDec,"\ncudaEvent_t start, stop;", AstUnparseAttribute::e_after);
  appendStatement(varDec,body);
	SgExprStatement *kCall = buildFunctionCallStmt("cudaEventCreate", buildVoidType(), buildExprListExp(buildAddressOfOp(buildOpaqueVarRefExp(SgName("start")))), body);
  appendStatement(kCall,body);
	kCall = buildFunctionCallStmt("cudaEventCreate", buildVoidType(), buildExprListExp(buildAddressOfOp(buildOpaqueVarRefExp(SgName("stop")))), body);
  appendStatement(kCall,body);
	kCall = buildFunctionCallStmt("cudaEventRecord", buildVoidType(), buildExprListExp(buildOpaqueVarRefExp(SgName("start")), buildIntVal(0)), body);
  appendStatement(kCall,body);



  // To add a call to the CUDA function, we need to build a list of parameters that
  // we pass to it. The easiest way to do this is to name the members of the 
  // struct to which they belong, but this is not the most elegant approach.
  kPars = buildExprListExp();
  for(int i=0; i<pl->numArgs(); i++)
  {
    SgExpression *e = buildOpaqueVarRefExp(SgName("arg"+buildStr(i)+"->dat_d"));
    SgCastExp* e_cast = buildCastExp(e, buildPointerType(pl->args[i]->type));
    kPars->append_expression(e_cast);
  }
  e = buildOpaqueVarRefExp(SgName("set.size"));
  kPars->append_expression(e);
	for(int i=0; i<pl->numArgs(); i++)
  {
		if(pl->args[i]->consideredAsReduction())
		{
			kPars->append_expression(buildOpaqueVarRefExp(SgName("block_reduct" + buildStr(i))));
		}
	}
		

  // We have to add the kernel configuration as part of the function name
  // as CUDA is not directly supported by ROSE - however, I understand
  // that CUDA and OpenCL support is coming soon!
  kCall = buildFunctionCallStmt("op_cuda_"+kernel_name+"<<<gridsize,bsize,reduct_shared>>>", buildVoidType(), kPars, body);
  appendStatement(kCall,body);

	// If we have reduction operations it requires a second kernel launch (gridsize = 1, blocksize = 1)
	if(reduction_required)
	{
		kPars = buildExprListExp();
		kPars->append_expression(buildOpaqueVarRefExp(SgName("gridsize")));
		for(int i=0; i<pl->numArgs(); i++)
		{
			if(pl->args[i]->consideredAsReduction())
			{
				SgExpression *e = buildOpaqueVarRefExp(SgName("arg"+buildStr(i)+"->dat_d"));
    		SgCastExp* e_cast = buildCastExp(e, buildPointerType(pl->args[i]->type));
    		kPars->append_expression(e_cast);

				kPars->append_expression(buildOpaqueVarRefExp(SgName("block_reduct" + buildStr(i))));
			}
		}
		kCall = buildFunctionCallStmt("op_cuda_"+kernel_name+"_reduction<<<1,1,reduct_shared>>>", buildVoidType(), kPars, body);
		appendStatement(kCall,body);
	}


	// Add the timer block <end>
	//------------------------------------------------------
	/*
  cudaEventRecord( stop, 0 );
  cudaThreadSynchronize();
  cudaEventElapsedTime( &elapsed_time_ms, start, stop );
  cudaEventDestroy( start );
  cudaEventDestroy( stop );
	*/  
	//------------------------------------------------------
	kCall = buildFunctionCallStmt("cudaEventRecord", buildVoidType(), buildExprListExp(buildOpaqueVarRefExp(SgName("stop")), buildIntVal(0)), body);
  appendStatement(kCall,body);
	kCall = buildFunctionCallStmt("cudaThreadSynchronize", buildVoidType(), NULL, body);
	appendStatement(kCall, body);
	kCall = buildFunctionCallStmt("cudaEventElapsedTime", buildVoidType(), buildExprListExp( buildOpaqueVarRefExp(SgName("&elapsed_time_ms")), buildOpaqueVarRefExp(SgName("start")), buildOpaqueVarRefExp(SgName("stop")) ), body);
	appendStatement(kCall, body);
	kCall = buildFunctionCallStmt("cudaEventDestroy", buildVoidType(), buildExprListExp( buildOpaqueVarRefExp(SgName("start")) ), body);
	appendStatement(kCall, body);
	kCall = buildFunctionCallStmt("cudaEventDestroy", buildVoidType(), buildExprListExp( buildOpaqueVarRefExp(SgName("stop")) ), body);
	appendStatement(kCall, body);

  /*
  by Zohirul : PRE-Handle const and global data
  */
  postHandleConstAndGlobalData(fn, pl, body);

	/*
  by Zohirul : PRE-Handle const and global data
  */
  SgReturnStmt* rtstmt = buildReturnStmt(buildOpaqueVarRefExp(SgName("elapsed_time_ms")));
	appendStatement(rtstmt, body);
  
  // Add to list of files that need to be unparsed.
  kernels.push_back(file->get_project());
}


/*
 *	Generate Seperate File For the Standard Kernel
 *	----------------------------------------------
 */ 
void OPParLoop::generateStandard(SgFunctionCallExp *fn, op_par_loop *pl)
{
  // We build a new file for the CUDA kernel and its stub function
  string kernel_name = getName(pl->kernel);
  string file_name = kernel_name + "_kernel.cu";
  cout << "Generating (Standard) CUDA Parallel Loop File for " << kernel_name << endl;
  
  SgSourceFile *file = isSgSourceFile(buildFile("blank.cpp", file_name, NULL));
  ROSE_ASSERT(file!=NULL);
  SgGlobal *globalScope = file->get_globalScope();
  
  // Start adding the include files
	addTextForUnparser(globalScope,"#include \"user_defined_types.h\"\n",AstUnparseAttribute::e_before);
  addTextForUnparser(globalScope,"#include \"op_datatypes.h\"\n",AstUnparseAttribute::e_before);
  addTextForUnparser(globalScope,"#include \"kernels.h\"\n\n",AstUnparseAttribute::e_before);
  
  // At present we copy Mike's implementation in that we prefix the include to the
  addTextForUnparser(globalScope, "\n\n__device__\n#include <"+kernel_name+".h>\n",AstUnparseAttribute::e_before);
  
  // In order to build the prototype for the plan function, we need to get hold of the types 
  // that we intend to pass it. Since these are declared in op_datatypes.h, we need to 
  // loop them up before we can use them.
  SgType *op_set, *op_dat, *op_ptr, *op_access, *op_datatype, *op_plan;
  op_set = lookupNamedTypeInParentScopes("op_set");
  op_dat = SgClassType::createType(buildStructDeclaration(SgName("op_dat<void>"), globalScope));
  op_ptr = lookupNamedTypeInParentScopes("op_ptr");
  op_access = lookupNamedTypeInParentScopes("op_access");
  op_plan = lookupNamedTypeInParentScopes("op_plan");

  // Now we append all the external forward declaration functions
  forwardDeclareUtilFunctions(globalScope, op_set, op_dat, op_ptr, op_access, op_plan);

  // 1.1 FUNCTION DEFINITION - CREATE PARAMS
  // ===============================================
  
  SgFunctionParameterList *paramList = buildFunctionParameterList();
  SgType *argType = NULL;
  SgInitializedName *nm = NULL;
  
	// First Assemble all expressions using plan container <for arguments with indirection>
	for(int i=0; i<pl->planContainer.size(); i++)
	{
		// Add "ind_arg0"
		argType = buildPointerType(pl->planContainer[i]->type);
		nm = buildInitializedName(SgName("ind_arg" + buildStr(i)), argType);
		appendArg(paramList, nm);

		// Add "ind_arg0_ptrs"
		argType = buildPointerType(buildIntType());
		nm = buildInitializedName(SgName("ind_arg" + buildStr(i) + "_ptrs"), argType);
		appendArg(paramList, nm);

		// Add "ind_arg0_sizes"
		nm = buildInitializedName(SgName("ind_arg" + buildStr(i) + "_sizes"), argType);
		appendArg(paramList, nm);
	
		// Add "ind_arg0_offset"
		nm = buildInitializedName(SgName("ind_arg" + buildStr(i) + "_offset"), argType);
		appendArg(paramList, nm);
	}
	// Then add all the pointers
	bool reduction_required = false;
	for(int i=0; i<pl->args.size(); i++)
	{
		if(pl->args[i]->consideredAsReduction())
			reduction_required = true;
		if(pl->args[i]->usesIndirection())
		{
			// Add "arg1_ptr"
			argType = buildPointerType(buildIntType());
			nm = buildInitializedName(arg(i) + SgName("_ptrs"), argType);
			appendArg(paramList, nm);
		}
		else if(pl->args[i]->isGlobal())
		{
			argType = buildPointerType(pl->args[i]->type);
			nm = buildInitializedName(arg(i), argType);
		  appendArg(paramList, nm);
		}
		else
		{
			argType = buildPointerType(pl->args[i]->type);
			nm = buildInitializedName(arg(i) + SgName("_d"), argType);
		  appendArg(paramList, nm);
		}
	}
	// Other stuff
	argType = buildIntType();
	nm = buildInitializedName(SgName("block_offset"), argType);
	appendArg(paramList, nm);
	argType = buildPointerType(argType);
	nm = buildInitializedName(SgName("blkmap"), argType);
	appendArg(paramList, nm);
	nm = buildInitializedName(SgName("offset"), argType);
	appendArg(paramList, nm);
	nm = buildInitializedName(SgName("nelems"), argType);
	appendArg(paramList, nm);
	nm = buildInitializedName(SgName("ncolors"), argType);
	appendArg(paramList, nm);
	nm = buildInitializedName(SgName("colors"), argType);
	appendArg(paramList, nm);
	if(reduction_required)
		appendArg(paramList, buildInitializedName(SgName("block_reduct"), buildVoidType()));
  
	// 1.2 ADD FUNCTION DEFINITION
  // ===============================================

  // We can build the __global__ function using the parameter list and add it to our new file. We get a reference to
  // the body of this function so that we can add code to it later on.
  SgFunctionDeclaration *func = buildDefiningFunctionDeclaration("op_cuda_" + kernel_name, buildVoidType(), paramList, globalScope);
  addTextForUnparser(func,"\n\n__global__",AstUnparseAttribute::e_before);
  appendStatement(func, globalScope);
  SgBasicBlock *body = func->get_definition()->get_body();
  
	// 2. ADD DECLARATION OF LOCAL VARIABLES
  // ===============================================

  // We Add the declarations of local variables first, required only for INC
  for(int i=0; i<pl->numArgs(); i++)
  {
		if((pl->args[i]->isNotGlobal() && pl->args[i]->access == OP_INC) || (pl->args[i]->isNotGlobal() && !pl->args[i]->usesIndirection()))
		{
		  SgType *argType = pl->args[i]->type;
		  SgVariableDeclaration *varDec = buildVariableDeclaration(argLocal(i), buildArrayType(argType, buildIntVal(pl->args[i]->dim)), NULL, body);
		  appendStatement(varDec,body);
  	}
  }

	
	// 3. ADD SHARED MEMORY DECLARATIONS
  // ===============================================

  // Add shared memory declaration
  SgVariableDeclaration *varDec = buildVariableDeclaration(SgName("shared"), buildArrayType(buildCharType(), NULL), NULL, body);
  addTextForUnparser(varDec,"\n\n  extern __shared__ ", AstUnparseAttribute::e_after);
  appendStatement(varDec,body);

	// Add shared variables for the planContainer variables - for each category <ptr>
	for(int i=0; i<pl->planContainer.size(); i++)
	{
		SgVariableDeclaration *varDec = buildVariableDeclaration(SgName("ind_" + arg(i) + "_ptr"), buildPointerType(buildIntType()), NULL, body);
  	addTextForUnparser(varDec,"\n\n  __shared__ ", AstUnparseAttribute::e_after);
  	appendStatement(varDec,body);
	}

	// Add shared variables for the planContainer variables - for each category <size>
	for(int i=0; i<pl->planContainer.size(); i++)
	{
		SgVariableDeclaration *varDec = buildVariableDeclaration(SgName("ind_" + arg(i) + "_size"), buildIntType(), NULL, body);
  	addTextForUnparser(varDec,"\n\n  __shared__ ", AstUnparseAttribute::e_after);
  	appendStatement(varDec,body);
	}

	// Add shared variables for the planContainer variables - for each category <s for shared>
	for(int i=0; i<pl->planContainer.size(); i++)
	{
		SgVariableDeclaration *varDec = buildVariableDeclaration(SgName("ind_" + arg(i) + "_s"), buildPointerType(pl->planContainer[i]->type), NULL, body);
  	addTextForUnparser(varDec,"\n\n  __shared__ ", AstUnparseAttribute::e_after);
  	appendStatement(varDec,body);
	}

	// Then add respective shared variables for each argument
	for(int i=0; i<pl->args.size(); i++)
	{
		if(pl->args[i]->usesIndirection())
		{
			SgVariableDeclaration *varDec = buildVariableDeclaration(SgName(arg(i) + "_ptr"), buildPointerType(buildIntType()), NULL, body);
			addTextForUnparser(varDec,"\n\n  __shared__ ", AstUnparseAttribute::e_after);
			appendStatement(varDec,body);
		}
		else if(!pl->args[i]->consideredAsConst())
		{
			SgVariableDeclaration *varDec = buildVariableDeclaration(SgName(arg(i)), buildPointerType(pl->args[i]->type), NULL, body);
			addTextForUnparser(varDec,"\n\n  __shared__ ", AstUnparseAttribute::e_after);
			appendStatement(varDec,body);
		}
	}

	// Add nelem
  varDec = buildVariableDeclaration(SgName("nelem2"), buildIntType(), NULL, body);
	addTextForUnparser(varDec,"\n\n  __shared__ ", AstUnparseAttribute::e_after);
  appendStatement(varDec,body);

	// Add ncolor
	varDec = buildVariableDeclaration(SgName("ncolor"), buildIntType(), NULL, body);
	addTextForUnparser(varDec,"\n\n  __shared__ ", AstUnparseAttribute::e_after);
  appendStatement(varDec,body);

	// Add color
	varDec = buildVariableDeclaration(SgName("color"), buildPointerType(buildIntType()), NULL, body);
	addTextForUnparser(varDec,"\n\n  __shared__ ", AstUnparseAttribute::e_after);
  appendStatement(varDec,body);

	// blockId
	varDec = buildVariableDeclaration(SgName("blockId"), buildIntType(), NULL, body);
	addTextForUnparser(varDec,"\n\n  __shared__ ", AstUnparseAttribute::e_after);
  appendStatement(varDec,body);

	// nelem
	varDec = buildVariableDeclaration(SgName("nelem"), buildIntType(), NULL, body);
	addTextForUnparser(varDec,"\n\n  __shared__ ", AstUnparseAttribute::e_after);
  appendStatement(varDec,body);


	// 4.1 GET SIZES AND SHIFT POINTERS AND DIRECT MAPPED DATA
  // ========================================================

	// We put this part within an IF condition, so that threadIdx.x == 0 performs this
	SgScopeStatement *ifBody = buildBasicBlock();
  SgExprStatement *conditionStmt = buildExprStatement( buildEqualityOp( buildOpaqueVarRefExp(SgName("threadIdx.x")), buildIntVal(0) ) );
  SgIfStmt* threadCondition = buildIfStmt(conditionStmt, ifBody, NULL);

  // Add blockId variable
  SgExpression* expression = buildOpaqueVarRefExp(SgName("blkmap[blockIdx.x + block_offset]"));
	expression = buildAssignOp(buildOpaqueVarRefExp(SgName("blockId")), expression);
	appendStatement(buildExprStatement(expression), ifBody);

	// Add blockId variable
  expression = buildOpaqueVarRefExp(SgName("nelems[blockId]"));
	expression = buildAssignOp(buildOpaqueVarRefExp(SgName("nelem")), expression);
	appendStatement(buildExprStatement(expression), ifBody);

	// Add ncolor variable
	expression = buildOpaqueVarRefExp(SgName("ncolors[blockId]"));
	expression = buildAssignOp(buildOpaqueVarRefExp(SgName("ncolor")), expression);
	appendStatement(buildExprStatement(expression), ifBody);

	// Cache offset[blockId]
	expression = buildOpaqueVarRefExp(SgName("offset[blockId]"));
	varDec = buildVariableDeclaration(SgName("cur_offset"), buildIntType(), buildAssignInitializer(expression), ifBody);
	appendStatement(varDec, ifBody);

	// Add color variable
	expression = buildOpaqueVarRefExp(SgName("cur_offset"));
	expression = buildAddOp(buildOpaqueVarRefExp(SgName("colors")), expression);
	expression = buildAssignOp(buildOpaqueVarRefExp(SgName("color")), expression);
	appendStatement(buildExprStatement(expression), ifBody);

	// Example : int nelem2 = blockDim.x*(1+(X)/blockDim.x);
  expression = buildOpaqueVarRefExp(SgName("nelem"));
  expression = buildSubtractOp(expression, buildIntVal(1));
  expression = buildDivideOp(expression, buildOpaqueVarRefExp(SgName("blockDim.x")));
  expression = buildAddOp(buildIntVal(1), expression);
  expression = buildMultiplyOp(buildOpaqueVarRefExp(SgName("blockDim.x")), expression);
  expression = buildAssignOp(buildOpaqueVarRefExp(SgName("nelem2")), expression);
  appendStatement(buildExprStatement(expression), ifBody);

	// Calculate the category sizes
	for(int i=0; i<pl->planContainer.size(); i++)
	{
		expression = buildOpaqueVarRefExp(SgName("ind_" + arg(i) + "_sizes[blockId]"));
		expression = buildAssignOp(buildOpaqueVarRefExp(SgName("ind_" + arg(i) + "_size")), expression);
		appendStatement(buildExprStatement(expression), ifBody);
	}

	// Calculate the category pointers
	for(int i=0; i<pl->planContainer.size(); i++)
	{
		expression = buildOpaqueVarRefExp(SgName("ind_" + arg(i) + "_offset[blockId]"));
		expression = buildAddOp(buildOpaqueVarRefExp(SgName("ind_" + arg(i) + "_ptrs")), expression);
		expression = buildAssignOp(buildOpaqueVarRefExp(SgName("ind_" + arg(i) + "_ptr")), expression);
		appendStatement(buildExprStatement(expression), ifBody);
	}

	// Calculate argument pointers
	expression = buildOpaqueVarRefExp(SgName("cur_offset"));
	for(int i=0; i<pl->args.size(); i++)
	{
		if(pl->args[i]->usesIndirection())
		{
			SgExpression* ex = buildAddOp(buildOpaqueVarRefExp(SgName(arg(i) + "_ptrs")), expression);
			ex = buildAssignOp(buildOpaqueVarRefExp(SgName(arg(i) + "_ptr")), ex);
			appendStatement(buildExprStatement(ex), ifBody);
		}
		else if(!pl->args[i]->consideredAsConst())
		{
			SgExpression* ex = buildMultiplyOp(expression, buildIntVal(pl->args[i]->dim));
			ex = buildAddOp(buildOpaqueVarRefExp(SgName(arg(i) + "_d")), ex);
			ex = buildAssignOp(buildOpaqueVarRefExp(SgName(arg(i))), ex);
			appendStatement(buildExprStatement(ex), ifBody);
		}
	}

	// Set Shared Memory Pointers
	for(int i=0; i<pl->planContainer.size(); i++)
	{
		if(i==0)
		{
			SgVariableDeclaration* var_dec = buildVariableDeclaration(SgName("nbytes"), buildIntType(), buildAssignInitializer(buildIntVal(0)), ifBody);
			appendStatement(var_dec, ifBody);
		}
		else
		{
			// Example: nbytes += ROUND_UP(ind_arg0_size*sizeof(float)*2);
			expression = buildMultiplyOp(buildSizeOfOp(buildFloatType()), buildIntVal(pl->planContainer[i-1]->dim));
			expression = buildMultiplyOp(buildOpaqueVarRefExp(SgName("ind_" + arg(i-1) + "_size")), expression);
			SgExprListExp* expressions = buildExprListExp();
			expressions->append_expression(expression);
			expression = buildFunctionCallExp(SgName("ROUND_UP"), buildIntType(), expressions);
			expression = buildPlusAssignOp(buildOpaqueVarRefExp(SgName("nbytes")), expression);
			appendStatement(buildExprStatement(expression), ifBody);
		}
		expression = buildOpaqueVarRefExp(SgName("shared[nbytes]"));
		expression = buildAddressOfOp(expression);
		expression = buildCastExp(expression, buildPointerType(pl->planContainer[i]->type));
		expression = buildAssignOp(buildOpaqueVarRefExp(SgName("ind_" + arg(i) + "_s")), expression);
		appendStatement(buildExprStatement(expression), ifBody);
	}
	appendStatement(threadCondition, body);

	// 4.2 CALL SYNCTHREADS
  // ========================================================
	SgExprStatement *kCall = buildFunctionCallStmt("__syncthreads", buildVoidType(), NULL, body);
  appendStatement(kCall, body);

	// 4.3 COPY INDIRECT DATA SETS INTO SHARED MEMORY
  // ========================================================
  for(int i=0; i<pl->planContainer.size(); i++)
	{
		// Create outer loop
		SgScopeStatement *loopBody = buildBasicBlock();
		SgStatement *loopInit = buildVariableDeclaration( SgName("n"), buildIntType(), buildAssignInitializer(buildOpaqueVarRefExp(SgName("threadIdx.x"))) );
		SgName loopVar = isSgVariableDeclaration(loopInit)->get_definition()->get_vardefn()->get_name();
		SgName loopVarLimit = SgName("ind_") + arg(i) + SgName("_size");
		SgExprStatement *loopTest = buildExprStatement( buildLessThanOp( buildVarRefExp(loopVar), buildOpaqueVarRefExp(loopVarLimit) ) );	
		SgPlusAssignOp *loopIncrement = buildPlusAssignOp(buildVarRefExp(loopVar), buildOpaqueVarRefExp(SgName("blockDim.x")) );
		SgStatement *loopForLoop = buildForStatement(loopInit, loopTest, loopIncrement, loopBody);
		
		// If dim is greater than one then we need to use cached variable
		if(	pl->planContainer[i]->dim > 1)
		{
			if(pl->planContainer[i]->access == OP_READ || pl->planContainer[i]->access == OP_RW)
			{
				// Create cached variable
				SgExpression* e = buildOpaqueVarRefExp(SgName("ind_") + arg(i) + SgName("_ptr[n]"));
				SgStatement *vdec = buildVariableDeclaration( SgName("ind_index"), buildIntType(), buildAssignInitializer(e) );
				appendStatement(vdec, loopBody);
			}

			// Create inner loop body
			for(int j=0; j<pl->planContainer[i]->dim; j++)
			{
				SgName indrvar = SgName("ind_") + arg(i) + SgName("_s[" + buildStr(j) + "+n*" + buildStr(pl->planContainer[i]->dim) + "]");
				SgExpression* asgnExpr;
				SgName asgnName; 
				switch(pl->planContainer[i]->access)
				{
					case OP_READ:
					case OP_RW:
						asgnName = SgName("ind_") + arg(i) + SgName("[" + buildStr(j) + "+ind_index*" + buildStr(pl->planContainer[i]->dim) + "]");
						asgnExpr = buildOpaqueVarRefExp(asgnName);
						break;
					case OP_WRITE:
						break;
					case OP_INC:
						asgnExpr = buildIntVal(0);
						break;
				}
				expression = buildAssignOp( buildOpaqueVarRefExp(indrvar), asgnExpr );
				appendStatement(buildExprStatement(expression), loopBody);
			}
		}
		else
		{
			SgName indrvar = SgName("ind_") + arg(i) + SgName("_s[n*" + buildStr(pl->planContainer[i]->dim) + "]");
			SgExpression* asgnExpr;
			SgName asgnName; 
			switch(pl->planContainer[i]->access)
			{
				case OP_READ:
				case OP_RW:
					asgnName = SgName("ind_") + arg(i) + SgName("[ind_") + arg(i) + SgName("_ptr[n]*") + SgName(buildStr(pl->planContainer[i]->dim)) + SgName("]");
					asgnExpr = buildOpaqueVarRefExp(asgnName);
					break;
				case OP_WRITE:
					break;
				case OP_INC:
					asgnExpr = buildIntVal(0);
					break;
			}
			expression = buildAssignOp( buildOpaqueVarRefExp(indrvar), asgnExpr );
			appendStatement(buildExprStatement(expression), loopBody);
		}
		// Append outer loop
		appendStatement(loopForLoop, body);
  }

  // 4.4 CALL SYNCTHREADS
  // ========================================================
  kCall = buildFunctionCallStmt("__syncthreads", buildVoidType(), NULL, body);
  appendStatement(kCall, body);
    
  // 5. PRE-KERNEL HEADER
  // ========================================================
	preKernelGlobalDataHandling(fn, pl, body);

  // 6. CREATE OUTER MAIN LOOP BODY
  // ========================================================
  SgScopeStatement *mainLoopBody = buildBasicBlock();
  SgStatement *mainLoopInit = buildVariableDeclaration( SgName("n"), buildIntType(), buildAssignInitializer(buildOpaqueVarRefExp(SgName("threadIdx.x"))) );
  SgName mainLoopVar = isSgVariableDeclaration(mainLoopInit)->get_definition()->get_vardefn()->get_name();

  // Part 1: Inside the main outer loop body - the first part, defining col2
  varDec = buildVariableDeclaration(SgName("col2"), buildIntType(), buildAssignInitializer(buildIntVal(-1)), mainLoopBody);
  SgName color2 = isSgVariableDeclaration(varDec)->get_definition()->get_vardefn()->get_name();
  appendStatement(varDec, mainLoopBody);

  // Part 2 <begin>: Create the if statement and do the actual calculation
  SgScopeStatement *condBody2 = buildBasicBlock();				
  SgExprStatement *condStmt2 = buildExprStatement( buildLessThanOp( buildVarRefExp(mainLoopVar), buildVarRefExp(SgName("nelem")) ) );
  SgIfStmt* cond2 = buildIfStmt(condStmt2, condBody2, NULL);
  
  // Part 2_1: Initialize Local Variables
  for(int i=0; i<pl->numArgs(); i++)
  {
		if(pl->args[i]->isNotGlobal() && pl->args[i]->access == OP_INC && pl->args[i]->usesIndirection())
		{
			for(int j=0; j<pl->args[i]->dim; j++)
			{
				// If uses indirection
				SgExpression* exprDst = buildOpaqueVarRefExp(argLocal(i) + SgName("["+buildStr(j)+"]"));
				SgExpression* exprSrc = buildIntVal(0);
			
				// Append statement to the inner loop body
				appendStatement(buildExprStatement( buildAssignOp( exprDst,exprSrc ) ), condBody2);
			}
		}
  }

  // Part 2_1_2: Load directly accessed global memory data into local variables i.e. registers
  for(int i=0; i<pl->numArgs(); i++)
  {
		if(pl->args[i]->isNotGlobal() && !pl->args[i]->usesIndirection())
		{
			if(pl->args[i]->access == OP_READ || pl->args[i]->access == OP_RW)
			{
				for(int j=0; j<pl->args[i]->dim; j++)
				{
					//arg4_l[0] = *(arg4 + n * 4 + 0);
				
					SgExpression* lhs1 = buildMultiplyOp( buildOpaqueVarRefExp(SgName("n")), buildIntVal(pl->args[i]->dim) );
					lhs1 = buildAddOp(lhs1, buildIntVal(j));
					lhs1 = buildAddOp(buildOpaqueVarRefExp(arg(i)), lhs1);
					lhs1 = buildPointerDerefExp(lhs1);

					SgExpression* rhs1 = buildOpaqueVarRefExp(argLocal(i) + SgName("[" + buildStr(j) + "]"));
					rhs1 = buildAssignOp(rhs1, lhs1);			

					SgStatement* expr_statement = buildExprStatement(rhs1);
					appendStatement(expr_statement, condBody2);
				}
			}
		}
	} 
  
  // Part 2_2: Call user kernel <!!COMPLICATED!!>
  SgExprListExp *kPars = buildExprListExp();
  for(int i=0; i<pl->numArgs(); i++)
  {
		if(pl->args[i]->isGlobal())
		{
			if(pl->args[i]->consideredAsReduction())
			{
				expression = buildOpaqueVarRefExp(SgName(arg(i) + "_l"));
				kPars->append_expression(expression);
			}
			else if(pl->args[i]->consideredAsConst())
			{
				expression = buildOpaqueVarRefExp(SgName(arg(i)));
				kPars->append_expression(expression);
			}
		}
		else if(pl->args[i]->isNotGlobal())
		{
			if(pl->args[i]->usesIndirection())
			{
				if(pl->args[i]->access == OP_INC)
				{
					expression = buildOpaqueVarRefExp(SgName(arg(i) + "_l"));
					kPars->append_expression(expression);
				}
				else
				{
					expression = buildMultiplyOp(buildOpaqueVarRefExp(SgName(arg(i)+"_ptr[n]")), buildIntVal(pl->args[i]->dim));
					expression = buildAddOp(buildOpaqueVarRefExp(SgName("ind_" + arg(pl->args[i]->plan_index) + "_s")), expression);
					kPars->append_expression(expression);
				}
			}
			else
			{
				kPars->append_expression( buildOpaqueVarRefExp(SgName(argLocal(i))) );
			}
		}
  }
  SgExprStatement *uf = buildFunctionCallStmt(SgName(kernel_name), buildVoidType(), kPars);
  appendStatement(uf, condBody2);

	// Part 2_2_2: Move directly accessed data back to registers
  for(int i=0; i<pl->numArgs(); i++)
  {
		if(pl->args[i]->isNotGlobal() && !pl->args[i]->usesIndirection())
		{
			if(pl->args[i]->access == OP_WRITE || pl->args[i]->access == OP_RW)
			{
				for(int j=0; j<pl->args[i]->dim; j++)
				{
					//arg4_l[0] = *(arg4 + n * 4 + 0);
				
					SgExpression* lhs1 = buildMultiplyOp( buildOpaqueVarRefExp(SgName("n")), buildIntVal(pl->args[i]->dim) );
					lhs1 = buildAddOp(lhs1, buildIntVal(j));
					lhs1 = buildAddOp(buildOpaqueVarRefExp(arg(i)), lhs1);
					lhs1 = buildPointerDerefExp(lhs1);

					SgExpression* rhs1 = buildOpaqueVarRefExp(argLocal(i) + SgName("[" + buildStr(j) + "]"));
					lhs1 = buildAssignOp(lhs1, rhs1);			

					SgStatement* expr_statement = buildExprStatement(lhs1);
					appendStatement(expr_statement, condBody2);
				}
			}
		}
	}
  
  // Part 2_3: Set the color of the thread
  expression = buildAssignOp( buildOpaqueVarRefExp(color2), buildOpaqueVarRefExp(SgName("color[") +  mainLoopVar + SgName("]")) );
  SgStatement* expr_statement = buildExprStatement(expression);
  appendStatement(expr_statement, condBody2);

  // Part 2 <end>: Add the condition body to the main loop body
  appendStatement(cond2, mainLoopBody);
  
  
  // 7. COPY DATA BACK TO THE SHARED MEMORY
  // ========================================================
	
  // Part 3: Inside the main outer loop body - the third part, copying values from arg to shared memory
	bool brequired = false;
	for(int i=0; i<pl->numArgs(); i++) {
			if(pl->args[i]->isNotGlobal() && pl->args[i]->access == OP_INC && pl->args[i]->usesIndirection()) {
				brequired = true;
			}
	}
	if(brequired)
	{
		// Create outer loop
		SgScopeStatement *loopBody = buildBasicBlock();
		SgStatement *loopInit = buildVariableDeclaration( SgName("col"), buildIntType(), buildAssignInitializer(buildIntVal(0)) );
		SgName loopVar = isSgVariableDeclaration(loopInit)->get_definition()->get_vardefn()->get_name();
		SgName loopVarLimit = SgName("ncolor");
		SgExprStatement *loopTest = buildExprStatement( buildLessThanOp( buildVarRefExp(loopVar), buildOpaqueVarRefExp(loopVarLimit) ) );	
		SgPlusPlusOp *loopIncrement = buildPlusPlusOp(buildVarRefExp(loopVar));
		SgStatement *loopForLoop = buildForStatement(loopInit, loopTest, loopIncrement, loopBody);

		// Create if color match condition
		SgScopeStatement *condBody = buildBasicBlock();				
		SgExprStatement *condStmt = buildExprStatement( buildEqualityOp( buildVarRefExp(loopVar), buildVarRefExp(color2) ) );
		SgIfStmt* cond = buildIfStmt(condStmt, condBody, NULL);

		bool alreadyDefined = false;
		for(int i=0; i<pl->numArgs(); i++)
		{
			if(pl->args[i]->isNotGlobal() && pl->args[i]->access == OP_INC && pl->args[i]->usesIndirection())
			{
				if(pl->args[i]->dim > 1)
				{
					// Create cached variable
					SgExpression* e = buildOpaqueVarRefExp(arg(i)+ SgName("_ptr[n]"));
					if(!alreadyDefined)
					{
						SgStatement *vdec = buildVariableDeclaration( SgName("ind_index"), buildIntType(), buildAssignInitializer(e) );
						appendStatement(vdec, condBody);
						alreadyDefined = true;
					}
					else
					{
						SgExpression* ee = buildAssignOp(buildOpaqueVarRefExp(SgName("ind_index")), e);
						appendStatement(buildExprStatement(ee), condBody);
					}
			
					// Create inner loop
					for(int j=0; j<pl->args[i]->dim; j++)
					{
						SgName dstName = SgName("ind_")+ arg(pl->args[i]->plan_index)+ SgName("_s[" + buildStr(j) + "+ind_index*" + buildStr(pl->args[i]->dim) + "]");
						SgName srcName = argLocal(i) + SgName("[" + buildStr(j) + "]");
						expression = buildPlusAssignOp( buildOpaqueVarRefExp(dstName), buildOpaqueVarRefExp(srcName) );
						appendStatement(buildExprStatement(expression), condBody);			
					}
				}
				else
				{
						SgName dstName = SgName("ind_")+ arg(pl->args[i]->plan_index)+ SgName("_s[")+ arg(i)+ SgName("_ptr[n]*" + buildStr(pl->args[i]->dim) + "]");
						SgName srcName = argLocal(i) + SgName("[0]");
						expression = buildPlusAssignOp( buildOpaqueVarRefExp(dstName), buildOpaqueVarRefExp(srcName) );
						appendStatement(buildExprStatement(expression), condBody);			
				}
			}
		}
		// Append condition statement to outer loop body
		appendStatement(cond, loopBody);
		// Create syncfunction statement
		kCall = buildFunctionCallStmt("__syncthreads", buildVoidType(), NULL, body);
		// Append syncthreads
		appendStatement(kCall, loopBody);
		// Append outer loop to the main body
		appendStatement(loopForLoop, mainLoopBody);
	}
	
  // Append main outer loop statement
  SgExprStatement *mainLoopTest = buildExprStatement( buildLessThanOp( buildVarRefExp(mainLoopVar), buildOpaqueVarRefExp(SgName("nelem2")) ) );	
  SgPlusAssignOp *mainLoopIncrement = buildPlusAssignOp(buildVarRefExp(mainLoopVar), buildOpaqueVarRefExp(SgName("blockDim.x")) );
  SgStatement *mainForLoop = buildForStatement(mainLoopInit, mainLoopTest, mainLoopIncrement, mainLoopBody);
  appendStatement(mainForLoop, body);
  
	

	// 8. COPY DATA BACK TO DRAM
  // ========================================================

  // For write and icrement
  // Copy indirect datasets into shared memory or zero increment
  for(int i=0; i<pl->planContainer.size(); i++)
	{
			if(pl->planContainer[i]->access == OP_READ)
				continue;
			if(pl->planContainer[i]->access == OP_MAX)
				continue;
			if(pl->planContainer[i]->access == OP_MIN)
				continue;
			
			// Create outer loop
			SgScopeStatement *loopBody = buildBasicBlock();
			SgStatement *loopInit = buildVariableDeclaration( SgName("n"), buildIntType(), buildAssignInitializer(buildOpaqueVarRefExp(SgName("threadIdx.x"))) );
			SgName loopVar = isSgVariableDeclaration(loopInit)->get_definition()->get_vardefn()->get_name();
			SgName loopVarLimit = SgName("ind_") + arg(i) + SgName("_size");
			SgExprStatement *loopTest = buildExprStatement( buildLessThanOp( buildVarRefExp(loopVar), buildOpaqueVarRefExp(loopVarLimit) ) );	
			SgPlusAssignOp *loopIncrement = buildPlusAssignOp(buildOpaqueVarRefExp(loopVar), buildOpaqueVarRefExp(SgName("blockDim.x")) );
			SgStatement *loopForLoop = buildForStatement(loopInit, loopTest, loopIncrement, loopBody);

			if(pl->planContainer[i]->dim > 1)
			{
				// Create cached variable
				SgExpression* e = buildOpaqueVarRefExp(SgName("ind_") + arg(i) + SgName("_ptr[n]"));
				SgStatement *vdec = buildVariableDeclaration( SgName("ind_index"), buildIntType(), buildAssignInitializer(e) );
				appendStatement(vdec, loopBody);
				
				for(int j=0; j<pl->planContainer[i]->dim; j++)
				{
					SgExpression* rhs = buildOpaqueVarRefExp(SgName("ind_" + arg(i)) + SgName("_s[" + buildStr(j) + "+n*" + buildStr(pl->planContainer[i]->dim) + "]"));
					SgExpression* lhs = buildOpaqueVarRefExp(SgName("ind_" + arg(i)) + SgName("[" + buildStr(j) + "+ind_index*" + buildStr(pl->planContainer[i]->dim) + "]"));
					expression = buildAssignOp( lhs, rhs );
					switch(pl->planContainer[i]->access)
					{
					case OP_RW:
					case OP_WRITE:
						expression = buildAssignOp( lhs, rhs );	
						break;
					case OP_INC:
						expression = buildPlusAssignOp( lhs, rhs );
						break;
					}
					appendStatement(buildExprStatement(expression), loopBody);
				}
			}
			else
			{
				SgExpression* rhs = buildOpaqueVarRefExp(SgName("ind_" + arg(i)) + SgName("_s[n*" + buildStr(pl->planContainer[i]->dim) + "]"));
				SgExpression* lhs = buildOpaqueVarRefExp(SgName("ind_" + arg(i)) + SgName("[ind_" + arg(i) + "_ptr[n]*" + buildStr(pl->planContainer[i]->dim) + "]"));
				expression = buildAssignOp( lhs, rhs );
				appendStatement(buildExprStatement(expression), loopBody);
			}
			// Append outer loop to the main body
			appendStatement(loopForLoop, body);
  }

  // Handle post global data handling
	postKernelGlobalDataHandling(fn, pl, body);

  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  
  // The following code builds the stub function. A little of the code that is output by
  // op2.m is not generated here, simply as it is not necessary and I was running out of
  // time before the meeting on 27 Jan. (cudaThreadSynchronize and cudaGetLastError, and
  // some debugging statements are missing, but would be trivial to add).

  // As usual we build a list of parameters for the function.
  paramList = buildFunctionParameterList();
  SgInitializedName *name = buildInitializedName(SgName("name"), buildPointerType(buildConstType(buildCharType())));
  appendArg(paramList, name);
  name = buildInitializedName(SgName("set"), op_set);
  appendArg(paramList, name);
  // Add other arguments
  for(int i=0; i<pl->numArgs(); i++)
  {
    name = buildInitializedName(SgName("arg"+buildStr(i)), buildPointerType(op_dat));
    appendArg(paramList, name);
    name = buildInitializedName(SgName("idx"+buildStr(i)), buildIntType());
    appendArg(paramList, name);
    name = buildInitializedName(SgName("ptr"+buildStr(i)), buildPointerType(op_ptr));
    appendArg(paramList, name);
    name = buildInitializedName(SgName("acc"+buildStr(i)), op_access);
    appendArg(paramList, name);
  }

  // We build the function with the parameter list and insert it into the global
  // scope of our file as before.
  string kernelFuncName = "op_par_loop_" + kernel_name;
  func = buildDefiningFunctionDeclaration(kernelFuncName, buildFloatType(), paramList, globalScope);
  cudaFunctionDeclarations.insert(pair<string, SgFunctionDeclaration*>(kernel_name, func));
  appendStatement(func, globalScope);
  body = func->get_definition()->get_body();

  /*
  By Zohirul: Add variables nargs and 'ninds'
  Example : int nargs = 3, ninds = 2;
  */
  
  varDec = buildVariableDeclaration(SgName("nargs"), buildIntType(), buildAssignInitializer(buildIntVal(pl->numArgs())), body);
  appendStatement(varDec,body);
  varDec = buildVariableDeclaration(SgName("ninds"), buildIntType(), buildAssignInitializer(buildIntVal(pl->planContainer.size())), body);
  appendStatement(varDec,body);

	/*
  By Zohirul: Add maximum grid size
	Example : int gridsize = (set.size - 1) / BSIZE + 1;
	*/
	SgExpression *expresn = buildOpaqueVarRefExp(SgName("set.size"));
	expresn = buildSubtractOp(expresn, buildIntVal(1));
	expresn = buildDivideOp(expresn, buildOpaqueVarRefExp(SgName("BSIZE")));
	expresn = buildAddOp(expresn, buildIntVal(1));
	varDec = buildVariableDeclaration(SgName("gridsize"), buildIntType(), buildAssignInitializer(expresn));
	appendStatement(varDec, body);
  
  /*
  By Zohirul: Add plan variables
  */
  SgExprListExp* exprList_args = buildExprListExp();
  SgExprListExp* exprList_idxs = buildExprListExp();
  SgExprListExp* exprList_ptrs = buildExprListExp();
  SgExprListExp* exprList_dims = buildExprListExp();
  SgExprListExp* exprList_typs = buildExprListExp();
  SgExprListExp* exprList_accs = buildExprListExp();
  SgExprListExp* exprList_inds = buildExprListExp();

  // Use the counter to indentify the index, e.g. 1st argument to use indireciton, or 2nd argument to use indirection
  // and we keep incrementing the counter. So the first arg to use indirection will get 0 and second arg to use indirection
  // will get 1.
  for(int i = 0; i < pl->numArgs(); i++)
  {
		exprList_args->append_expression( buildPointerDerefExp(buildOpaqueVarRefExp(SgName("arg"+buildStr(i)))) );
		if(pl->args[i]->usesIndirection())
		{
    	exprList_idxs->append_expression( buildOpaqueVarRefExp(SgName("idx"+buildStr(i))) );
			exprList_ptrs->append_expression( buildPointerDerefExp(buildOpaqueVarRefExp(SgName("ptr"+buildStr(i)))) );
		}
		else
		{
			exprList_idxs->append_expression( buildIntVal(-1) );
			exprList_ptrs->append_expression( buildOpaqueVarRefExp(SgName("OP_ID")) );
		}
		exprList_dims->append_expression( buildOpaqueVarRefExp(SgName(arg(i)+"->dim"))     );
		exprList_accs->append_expression( buildOpaqueVarRefExp(SgName("acc"+buildStr(i))) );
		exprList_inds->append_expression( buildIntVal(pl->args[i]->plan_index) );
  }
  varDec = buildVariableDeclaration( SgName("args"), buildArrayType(op_dat, buildIntVal(pl->numArgs())), buildAggregateInitializer(exprList_args), body);
  appendStatement(varDec,body);
  varDec = buildVariableDeclaration( SgName("idxs"), buildArrayType(buildIntType(), buildIntVal(pl->numArgs())), buildAggregateInitializer(exprList_idxs), body);
  appendStatement(varDec,body);
  varDec = buildVariableDeclaration( SgName("ptrs"), buildArrayType(op_ptr, buildIntVal(pl->numArgs())), buildAggregateInitializer(exprList_ptrs), body);
  appendStatement(varDec,body);
  varDec = buildVariableDeclaration( SgName("dims"), buildArrayType(buildIntType(), buildIntVal(pl->numArgs())), buildAggregateInitializer(exprList_dims), body);
  appendStatement(varDec,body);
  varDec = buildVariableDeclaration( SgName("accs"), buildArrayType(op_access, buildIntVal(pl->numArgs())), buildAggregateInitializer(exprList_accs), body);
  appendStatement(varDec,body);
  varDec = buildVariableDeclaration( SgName("inds"), buildArrayType(buildIntType(), buildIntVal(pl->numArgs())), buildAggregateInitializer(exprList_inds), body);
  appendStatement(varDec,body);
  

  /* 
  by Zohirul : Create and initialize the Plan variable pointer
  Example: op_plan *Plan = plan(name,set,nargs,args,idxs,ptrs,dims,typs,accs,ninds,inds);
  */
  // Create the plan function call, 1) first create params, 2) then call the function
  SgExprListExp *planPars = buildExprListExp();
  planPars->append_expression(buildOpaqueVarRefExp(SgName("name")));
  planPars->append_expression(buildOpaqueVarRefExp(SgName("set")));
  planPars->append_expression(buildOpaqueVarRefExp(SgName("nargs")));
  planPars->append_expression(buildOpaqueVarRefExp(SgName("args")));
  planPars->append_expression(buildOpaqueVarRefExp(SgName("idxs")));
  planPars->append_expression(buildOpaqueVarRefExp(SgName("ptrs")));
  planPars->append_expression(buildOpaqueVarRefExp(SgName("dims")));
  planPars->append_expression(buildOpaqueVarRefExp(SgName("accs")));
  planPars->append_expression(buildOpaqueVarRefExp(SgName("ninds")));
  planPars->append_expression(buildOpaqueVarRefExp(SgName("inds")));
  SgFunctionCallExp *expPlanFunc = buildFunctionCallExp(SgName("plan"), op_plan, planPars);
  
  // 3) then as the initializer of the Plan variable
  varDec = buildVariableDeclaration( SgName("Plan"), buildPointerType(op_plan),  buildAssignInitializer(expPlanFunc), body);
  appendStatement(varDec,body);

  /*
  by Zohirul : Add block offset
  */
  varDec = buildVariableDeclaration(SgName("block_offset"), buildIntType(), buildAssignInitializer(buildIntVal(0)), body);
  appendStatement(varDec,body);

	/*
  by Zohirul : PRE-Handle const and global data
  */
  preHandleConstAndGlobalData(fn, pl, body);

	/*
  by Zohirul : Add Total Time
  */
  varDec = buildVariableDeclaration(SgName("total_time"), buildFloatType(), buildAssignInitializer(buildFloatVal(0.0f)), body);
  appendStatement(varDec,body);

  /*
  by Zohirul : Add for loop for executing op_cuda_res<<<gridsize,bsize,nshared>>>
  */
  // Create loop body
  SgScopeStatement *blockLoopBody = buildBasicBlock();
  SgStatement *blockLoopInit = buildVariableDeclaration(SgName("col"), buildIntType(), buildAssignInitializer(buildIntVal(0)));
  SgName blockLoopVar = isSgVariableDeclaration(blockLoopInit)->get_definition()->get_vardefn()->get_name();
  
  // Add nshared and nblocks
  SgExpression* e = buildOpaqueVarRefExp(SgName("Plan->ncolblk[") + blockLoopVar + SgName("]"));
  varDec = buildVariableDeclaration(SgName("nblocks"), buildIntType(), buildAssignInitializer(e), blockLoopBody);
  appendStatement(varDec,blockLoopBody);
  e = buildOpaqueVarRefExp(SgName("Plan->nshared"));
  varDec = buildVariableDeclaration(SgName("nshared"), buildIntType(), buildAssignInitializer(e), blockLoopBody);
  appendStatement(varDec,blockLoopBody);


	// Add the timer block
	//-----------------------------------------
  /*
  float elapsed_time_ms=0.0f;
  cudaEvent_t start, stop;
  cudaEventCreate( &start );
  cudaEventCreate( &stop  );
  cudaEventRecord( start, 0 );
	*/
  //-----------------------------------------
	varDec = buildVariableDeclaration(SgName("elapsed_time_ms"), buildFloatType(), buildAssignInitializer(buildFloatVal(0.0f)), body);
	addTextForUnparser(varDec,"\ncudaEvent_t start, stop;", AstUnparseAttribute::e_after);
  appendStatement(varDec,blockLoopBody);
	kCall = buildFunctionCallStmt("cudaEventCreate", buildVoidType(), buildExprListExp(buildAddressOfOp(buildOpaqueVarRefExp(SgName("start")))), body);
  appendStatement(kCall,blockLoopBody);
	kCall = buildFunctionCallStmt("cudaEventCreate", buildVoidType(), buildExprListExp(buildAddressOfOp(buildOpaqueVarRefExp(SgName("stop")))), body);
  appendStatement(kCall,blockLoopBody);
	kCall = buildFunctionCallStmt("cudaEventRecord", buildVoidType(), buildExprListExp(buildOpaqueVarRefExp(SgName("start")), buildIntVal(0)), body);
	appendStatement(kCall,blockLoopBody);


  // To add a call to the CUDA function, we need to build a list of parameters that
  // we pass to it. The easiest way to do this is to name the members of the 
  // struct to which they belong, but this is not the most elegant approach.
  kPars = buildExprListExp();
	{
		// First Assemble all expressions using plan container <for arguments with indirection>
		for(int i=0; i<pl->planContainer.size(); i++)
		{
			SgExpression *e = buildOpaqueVarRefExp(SgName("arg"+buildStr(pl->planContainer[i]->own_index)+"->dat_d"));
			e = buildCastExp(e, buildPointerType(pl->planContainer[i]->type));
			kPars->append_expression(e);
			e = buildOpaqueVarRefExp(SgName("Plan->ind_ptrs["+buildStr(i)+"]"));
			kPars->append_expression(e);
			e = buildOpaqueVarRefExp(SgName("Plan->ind_sizes["+buildStr(i)+"]"));
			kPars->append_expression(e);
			e = buildOpaqueVarRefExp(SgName("Plan->ind_offs["+buildStr(i)+"]"));
			kPars->append_expression(e);
		}
		// Then add all the pointers
		for(int i=0; i<pl->args.size(); i++)
		{
			if(pl->args[i]->usesIndirection())
			{
				SgExpression *e = buildOpaqueVarRefExp(SgName("Plan->ptrs["+buildStr(i)+"]"));
				kPars->append_expression(e);
			}
			else
			{
				SgExpression *e = buildOpaqueVarRefExp(SgName("arg"+buildStr(i)+"->dat_d"));
				e = buildCastExp(e, buildPointerType(pl->args[i]->type));
				kPars->append_expression(e);
			}
		}
		// Add additional parameters
		e = buildOpaqueVarRefExp(SgName("block_offset"));
		kPars->append_expression(e);
		e = buildOpaqueVarRefExp(SgName("Plan->blkmap"));
		kPars->append_expression(e);
		e = buildOpaqueVarRefExp(SgName("Plan->offset"));
		kPars->append_expression(e);
		e = buildOpaqueVarRefExp(SgName("Plan->nelems"));
		kPars->append_expression(e);
		e = buildOpaqueVarRefExp(SgName("Plan->nthrcol"));
		kPars->append_expression(e);
		e = buildOpaqueVarRefExp(SgName("Plan->thrcol"));
		kPars->append_expression(e);
		if(reduction_required)
			kPars->append_expression(buildOpaqueVarRefExp(SgName("block_reduct")));
	}

  // We have to add the kernel configuration as part of the function name
  // as CUDA is not directly supported by ROSE - however, I understand
  // that CUDA and OpenCL support is coming soon!
  kCall = buildFunctionCallStmt("op_cuda_"+kernel_name+"<<<nblocks,BSIZE,nshared>>>", buildVoidType(), kPars, body);
  appendStatement(kCall,blockLoopBody);


	// Add the timer block
	//------------------------------------------------------
	/*
  cudaEventRecord( stop, 0 );
  cudaThreadSynchronize();
  cudaEventElapsedTime( &elapsed_time_ms, start, stop );
  cudaEventDestroy( start );
  cudaEventDestroy( stop );
	total_time += elapsed_time_ms;
	*/  
	//------------------------------------------------------
	kCall = buildFunctionCallStmt("cudaEventRecord", buildVoidType(), buildExprListExp(buildOpaqueVarRefExp(SgName("stop")), buildIntVal(0)), body);
  appendStatement(kCall,blockLoopBody);
	kCall = buildFunctionCallStmt("cudaThreadSynchronize", buildVoidType(), NULL, body);
	appendStatement(kCall, blockLoopBody);
	kCall = buildFunctionCallStmt("cudaEventElapsedTime", buildVoidType(), buildExprListExp( buildOpaqueVarRefExp(SgName("&elapsed_time_ms")), buildOpaqueVarRefExp(SgName("start")), buildOpaqueVarRefExp(SgName("stop")) ), body);
	appendStatement(kCall, blockLoopBody);
	kCall = buildFunctionCallStmt("cudaEventDestroy", buildVoidType(), buildExprListExp( buildOpaqueVarRefExp(SgName("start")) ), body);
	appendStatement(kCall, blockLoopBody);
	kCall = buildFunctionCallStmt("cudaEventDestroy", buildVoidType(), buildExprListExp( buildOpaqueVarRefExp(SgName("stop")) ), body);
	appendStatement(kCall, blockLoopBody);
	e = buildPlusAssignOp( buildOpaqueVarRefExp(SgName("total_time")), buildOpaqueVarRefExp(SgName("elapsed_time_ms")) );
	appendStatement(buildExprStatement(e),blockLoopBody);	


	// Call cuda thread synchronize
	kCall = buildFunctionCallStmt("cudaThreadSynchronize", buildVoidType(), NULL, body);
	appendStatement(kCall, blockLoopBody);
 
  // Increment the block_offset now
  e = buildPlusAssignOp( buildOpaqueVarRefExp(SgName("block_offset")), buildOpaqueVarRefExp(SgName("nblocks")) );
  appendStatement(buildExprStatement(e),blockLoopBody);
  
  // We can build a test and an increment for the loop, then insert 
  // the loop into the body of the outer loop.
  SgExprStatement *blockLoopTest = buildExprStatement( buildLessThanOp( buildVarRefExp(blockLoopVar), buildOpaqueVarRefExp(SgName("Plan->ncolors")) ) );
  SgPlusPlusOp *blockLoopIncrement = buildPlusPlusOp(buildVarRefExp(blockLoopVar));
  SgStatement *blockLoopForLoop = buildForStatement(blockLoopInit, blockLoopTest, blockLoopIncrement, blockLoopBody);
  appendStatement(blockLoopForLoop,body);

  /*
  by Zohirul : PRE-Handle const and global data
  */
  postHandleConstAndGlobalData(fn, pl, body);

	/*
  by Zohirul : return statement
  */
  SgReturnStmt* rtstmt = buildReturnStmt(buildOpaqueVarRefExp(SgName("total_time")));
	appendStatement(rtstmt, body);

  // Add to list of files that need to be unparsed.
  kernels.push_back(file->get_project());
}


void OPParLoop::forwardDeclareUtilFunctions(SgGlobal* globalScope, SgType* op_set, SgType* op_dat, SgType* op_ptr, SgType* op_access, SgType* op_plan)
{
	/*
  // Parameter list for "plan"
  SgFunctionParameterTypeList *paramTypes = buildFunctionParameterTypeList();
  paramTypes->append_argument(buildPointerType(buildConstType(buildCharType())));
  paramTypes->append_argument(op_set);
  paramTypes->append_argument(buildIntType());
  paramTypes->append_argument(buildPointerType(op_dat));
  paramTypes->append_argument(buildPointerType(buildIntType()));
  paramTypes->append_argument(buildPointerType(op_ptr));
  paramTypes->append_argument(buildPointerType(buildIntType()));
  paramTypes->append_argument(buildPointerType(op_access));
  paramTypes->append_argument(buildIntType());
  paramTypes->append_argument(buildPointerType(buildIntType()));
  SgFunctionParameterList *paramList = buildFunctionParameterList(paramTypes);
  
  // Forward declare "plan" function
  SgFunctionDeclaration *decl = buildNondefiningFunctionDeclaration("plan",buildPointerType(op_plan),paramList,globalScope);
  addTextForUnparser(decl,"\nextern ",AstUnparseAttribute::e_before); 
  appendStatement(decl,globalScope);

  // Parameter list for "prepare_op_dat_as_const"
  paramTypes = buildFunctionParameterTypeList();
  paramTypes->append_argument(buildReferenceType(op_dat));
  paramTypes->append_argument(buildIntType());
  paramList = buildFunctionParameterList(paramTypes);

  // Forward declare "prepare_op_dat_as_const"
  decl = buildNondefiningFunctionDeclaration("push_op_dat_as_const",buildVoidType(),paramList,globalScope);
  addTextForUnparser(decl,"\nextern ",AstUnparseAttribute::e_before); 
  appendStatement(decl,globalScope);

  // Forward declare "prepare_op_dat_as_gbl" 
  decl = buildNondefiningFunctionDeclaration("push_op_dat_as_reduct",buildVoidType(),deepCopy(paramList),globalScope);
  addTextForUnparser(decl,"\nextern ",AstUnparseAttribute::e_before); 
  appendStatement(decl,globalScope);

  paramTypes = buildFunctionParameterTypeList();
  paramTypes->append_argument(buildReferenceType(op_dat));
  paramList = buildFunctionParameterList(paramTypes);

  // Forward declare "prepare_op_dat_as_gbl" 
  decl = buildNondefiningFunctionDeclaration("pop_op_dat_as_reduct",buildVoidType(),paramList,globalScope);
  addTextForUnparser(decl,"\nextern ",AstUnparseAttribute::e_before); 
  appendStatement(decl,globalScope);

	// reduct and const function params
  paramTypes = buildFunctionParameterTypeList();
  paramTypes->append_argument(buildIntType());
  paramList = buildFunctionParameterList(paramTypes);

	// extern void reallocConstArrays(int consts_bytes)
  decl = buildNondefiningFunctionDeclaration("reallocConstArrays",buildVoidType(),paramList,globalScope);
  addTextForUnparser(decl,"\nextern ",AstUnparseAttribute::e_before); 
  appendStatement(decl,globalScope);
 
	//extern void reallocReductArrays(int reduct_bytes)
  decl = buildNondefiningFunctionDeclaration("reallocReductArrays",buildVoidType(),deepCopy(paramList),globalScope);
  addTextForUnparser(decl,"\nextern ",AstUnparseAttribute::e_before); 
  appendStatement(decl,globalScope);

	//extern void mvConstArraysToDevice(int consts_bytes)
  decl = buildNondefiningFunctionDeclaration("mvConstArraysToDevice",buildVoidType(),deepCopy(paramList),globalScope);
  addTextForUnparser(decl,"\nextern ",AstUnparseAttribute::e_before); 
  appendStatement(decl,globalScope);

	//extern void mvReductArraysToDevice(int reduct_bytes)
  decl = buildNondefiningFunctionDeclaration("mvReductArraysToDevice",buildVoidType(),deepCopy(paramList),globalScope);
  addTextForUnparser(decl,"\nextern ",AstUnparseAttribute::e_before); 
  appendStatement(decl,globalScope);
	
  //extern void mvReductArraysToHost(int reduct_bytes)
	decl = buildNondefiningFunctionDeclaration("mvReductArraysToHost",buildVoidType(),deepCopy(paramList),globalScope);
  addTextForUnparser(decl,"\nextern ",AstUnparseAttribute::e_before); 
  appendStatement(decl,globalScope);
	*/
}



void OPParLoop::preKernelGlobalDataHandling(SgFunctionCallExp *fn, op_par_loop *pl, SgBasicBlock *body)
{
	//cout << "HANDLE REDUCT DATA <TRANSFER TO DEVICE>" << endl;
  for(int i=0; i<pl->numArgs(); i++)
  {
    SgStatement *viInit = buildVariableDeclaration(SgName("d"), buildIntType(), buildAssignInitializer(buildIntVal(0)));
    SgName indVar = isSgVariableDeclaration(viInit)->get_definition()->get_vardefn()->get_name();

    if(pl->args[i]->consideredAsReduction())
    {
	    // Build the body of the loop.
	    SgExpression *lhs, *rhs, *subscript;
	    lhs = buildPntrArrRefExp(buildVarRefExp(argLocal(i), body), buildVarRefExp(indVar));
	    switch(pl->args[i]->access)
	    {
	      case OP_INC:
					rhs = buildIntVal(0);
					break;
	      default:
					rhs = buildPntrArrRefExp(buildVarRefExp(arg(i), body), buildVarRefExp(indVar));
					break;
	    }
	    SgStatement *action = buildAssignStatement(lhs, rhs);
	    SgStatement *viLoopBody = buildBasicBlock(action);

	    // We can build a test and an increment for the loop, then insert 
	    // the loop into the body of the outer loop.
	    SgExprStatement *viTest = buildExprStatement(buildLessThanOp(buildVarRefExp(indVar), buildIntVal(pl->args[i]->dim)));
	    SgPlusPlusOp *viIncrement = buildPlusPlusOp(buildVarRefExp(indVar));
	    SgStatement *viForLoop = buildForStatement(viInit, viTest, viIncrement, viLoopBody);
	    appendStatement(viForLoop,body);
    }
  }
}

void OPParLoop::postKernelGlobalDataHandling(SgFunctionCallExp *fn, op_par_loop *pl, SgBasicBlock *body)
{
	//cout << "HANDLE REDUCT DATA <TRANSFER FROM DEVICE>" << endl;
  for(int i=0; i<pl->numArgs(); i++)
  {
		if(pl->args[i]->consideredAsReduction())
		{
			// Create loop
			SgStatement *viInit = buildVariableDeclaration(SgName("d"), buildIntType(), buildAssignInitializer(buildIntVal(0)));
      SgName indVar = isSgVariableDeclaration(viInit)->get_definition()->get_vardefn()->get_name();
			      
      // Call function
		  SgExprListExp *kPars1 = buildExprListExp();
		  SgExpression *e = buildOpaqueVarRefExp(arg(i));
			e = buildAddOp(e, buildVarRefExp(indVar));
		  kPars1->append_expression(e);
			e = buildOpaqueVarRefExp(argLocal(i) + SgName("[d]"));
			kPars1->append_expression(e);
			e = buildOpaqueVarRefExp(SgName("block_reduct" + buildStr(i)));
			kPars1->append_expression(e);

			SgExprStatement *uf1 = NULL;
		  switch(pl->args[i]->access)
		  {
				case OP_INC:
					uf1 = buildFunctionCallStmt(SgName("op_reduction2_1<OP_INC>"), buildVoidType(), kPars1);
		  		break;
				case OP_MAX:
					uf1 = buildFunctionCallStmt(SgName("op_reduction2_1<OP_MAX>"), buildVoidType(), kPars1);
					break;
				case OP_MIN:
					uf1 = buildFunctionCallStmt(SgName("op_reduction2_1<OP_MIN>"), buildVoidType(), kPars1);
					break;
		  }

			// build test and increment, and add the loop into the body of the inner loop.
			SgScopeStatement *viLoopBody = buildBasicBlock(uf1);
      SgExprStatement *viTest = buildExprStatement(buildLessThanOp(buildOpaqueVarRefExp(indVar), buildIntVal(pl->args[i]->dim)));
      SgPlusPlusOp *viIncrement = buildPlusPlusOp(buildOpaqueVarRefExp(indVar));
      SgStatement *viForLoop = buildForStatement(viInit, viTest, viIncrement, viLoopBody);
      appendStatement(viForLoop,body);
		}
  }
}




void OPParLoop::preHandleConstAndGlobalData(SgFunctionCallExp *fn, op_par_loop *pl, SgBasicBlock *body)
{
	// Handle Reduct
	///////////////////////

	bool required = false;
	SgVariableDeclaration* varDec = buildVariableDeclaration(SgName("reduct_bytes"), buildIntType(), buildAssignInitializer(buildIntVal(0)), body);
	SgName varName = isSgVariableDeclaration(varDec)->get_definition()->get_vardefn()->get_name();	
	appendStatement(varDec,body);

	SgVariableDeclaration* varDec2 = buildVariableDeclaration(SgName("reduct_size"), buildIntType(), buildAssignInitializer(buildIntVal(0)), body);
	SgName varName2 = isSgVariableDeclaration(varDec2)->get_definition()->get_vardefn()->get_name();	
	appendStatement(varDec2,body);

	SgExpression* varExp = buildVarRefExp(varName, body);
	SgExpression* varExp2 = buildVarRefExp(varName2, body);
	for(int i = 0; i < pl->args.size(); i++)
	{
		if(pl->args[i]->consideredAsReduction())
		{
			required = true;
			SgExpression* rhs =  buildMultiplyOp(buildIntVal(pl->args[i]->dim), buildSizeOfOp(pl->args[i]->type));
			SgExprListExp* list = buildExprListExp();
			list->append_expression(rhs);
			rhs = buildFunctionCallExp(SgName("ROUND_UP"), buildIntType(), list);
			SgExpression* expr = buildPlusAssignOp(varExp , rhs);
  		appendStatement(buildExprStatement(expr), body);

			list = buildExprListExp();
			list->append_expression(varExp2);
			list->append_expression(buildSizeOfOp(pl->args[i]->type));
			rhs = buildFunctionCallExp(SgName("MAX"), buildIntType(), list);
			expr = buildAssignOp(varExp2, rhs);
			appendStatement(buildExprStatement(expr), body);
		}
	}

	SgExpression* expShared = buildMultiplyOp( varExp2, buildDivideOp(buildOpaqueVarRefExp(SgName("BSIZE")), buildIntVal(2)) );
	SgVariableDeclaration* varDec3 = buildVariableDeclaration(SgName("reduct_shared"), buildIntType(), buildAssignInitializer(expShared), body);
	SgName varName3 = isSgVariableDeclaration(varDec2)->get_definition()->get_vardefn()->get_name();	
	appendStatement(varDec3,body);
	SgExpression* varExp3 = buildVarRefExp(varName3, body);

  if(required)
	{
		// call reallocReductArrays(reduct_bytes);
		SgExprListExp* kPars = buildExprListExp();
		kPars->append_expression(varExp);
		SgStatement *kCall = buildFunctionCallStmt("reallocReductArrays", buildVoidType(), kPars, body);
  		appendStatement(kCall, body);

		// fixup with reduct_bytes
		SgExpression* expr = buildAssignOp(varExp, buildIntVal(0));
		appendStatement(buildExprStatement(expr), body);
		
		for(int i = 0; i < pl->args.size(); i++)
		{
			if(pl->args[i]->consideredAsReduction())
			{
				kPars = buildExprListExp();
				kPars->append_expression((buildOpaqueVarRefExp(SgName("*"+arg(i)))));
				kPars->append_expression(varExp);
				kCall = buildFunctionCallStmt("push_op_dat_as_reduct", buildVoidType(), kPars, body);
				appendStatement(kCall,body);

				expr =  buildMultiplyOp(buildIntVal(pl->args[i]->dim), buildSizeOfOp(pl->args[i]->type));
				SgExprListExp* expressions = buildExprListExp();
				expressions->append_expression(expr);
				expr = buildFunctionCallExp(SgName("ROUND_UP"), buildIntType(), expressions);
				expr = buildPlusAssignOp(varExp , expr);
  			appendStatement(buildExprStatement(expr), body);
			}
		}

		// call mvReductArraysToDevice(reduct_bytes)
		kPars = buildExprListExp();
		kPars->append_expression(varExp);
		kCall = buildFunctionCallStmt("mvReductArraysToDevice", buildVoidType(), kPars, body);
  	appendStatement(kCall,body);

		// handling global reduction - requires global memory allocation
		for(int i = 0; i < pl->args.size(); i++)
		{
			if(pl->args[i]->consideredAsReduction())
			{
				// declare block_reduct
				varDec = buildVariableDeclaration(SgName("block_reduct" + buildStr(i)), buildPointerType(buildVoidType()), buildAssignInitializer(buildIntVal(0)));
				appendStatement(varDec, body);

				// allocate memory
				kPars = buildExprListExp();
				kPars->append_expression(buildAddressOfOp( buildOpaqueVarRefExp(SgName("block_reduct") + buildStr(i)) ));
				kPars->append_expression(buildMultiplyOp( buildOpaqueVarRefExp(SgName("gridsize")), buildSizeOfOp(pl->args[i]->type) ));
				kCall = buildFunctionCallStmt("cudaMalloc", buildVoidType(), kPars, body);
  			appendStatement(kCall,body);
			}
		}
	}


	// Handle Const
	///////////////////////

	required = false;
	varDec = buildVariableDeclaration(SgName("const_bytes"), buildIntType(), buildAssignInitializer(buildIntVal(0)), body);
	varName = isSgVariableDeclaration(varDec)->get_definition()->get_vardefn()->get_name();	
	appendStatement(varDec,body);
	varExp = buildVarRefExp(varName, body);
	for(int i = 0; i < pl->args.size(); i++)
	{
		if(pl->args[i]->consideredAsConst())
		{
			required = true;
			SgExpression* rhs =  buildMultiplyOp(buildIntVal(pl->args[i]->dim), buildSizeOfOp(pl->args[i]->type));
			SgExprListExp* expressions = buildExprListExp();
			expressions->append_expression(rhs);
			rhs = buildFunctionCallExp(SgName("ROUND_UP"), buildIntType(), expressions);
			SgExpression* expr = buildPlusAssignOp(varExp , rhs);
  		appendStatement(buildExprStatement(expr), body);
		}
	}

  if(required)
	{
		// call reallocConstArrays(reduct_bytes);
		SgExprListExp* kPars = buildExprListExp();
		kPars->append_expression(varExp);
		SgStatement *kCall = buildFunctionCallStmt("reallocConstArrays", buildVoidType(), kPars, body);
  	appendStatement(kCall, body);

		// fixup with reduct_bytes
		SgExpression* expr = buildAssignOp(varExp, buildIntVal(0));
		appendStatement(buildExprStatement(expr), body);
		
		for(int i = 0; i < pl->args.size(); i++)
		{
			if(pl->args[i]->consideredAsConst())
			{
				kPars = buildExprListExp();
				kPars->append_expression((buildOpaqueVarRefExp(SgName("*"+arg(i)))));
				kPars->append_expression(varExp);
				kCall = buildFunctionCallStmt("push_op_dat_as_const", buildVoidType(), kPars, body);
				appendStatement(kCall,body);

				expr =  buildMultiplyOp(buildIntVal(pl->args[i]->dim), buildSizeOfOp(pl->args[i]->type));
				SgExprListExp* expressions = buildExprListExp();
				expressions->append_expression(expr);
				expr = buildFunctionCallExp(SgName("ROUND_UP"), buildIntType(), expressions);
				expr = buildPlusAssignOp(varExp , expr);
  			appendStatement(buildExprStatement(expr), body);
			}
		}

		// call mvReductArraysToDevice(reduct_bytes)
		kPars = buildExprListExp();
		kPars->append_expression(varExp);
		kCall = buildFunctionCallStmt("mvConstArraysToDevice", buildVoidType(), kPars, body);
  	appendStatement(kCall,body);
	}
}

void OPParLoop::postHandleConstAndGlobalData(SgFunctionCallExp *fn, op_par_loop *pl, SgBasicBlock *body)
{
	// Handle Reduct
	///////////////////////

	bool required = false;
	for(int i = 0; i < pl->args.size(); i++)
	{
		if(pl->args[i]->consideredAsReduction())
		{
			required = true;
			break;
		}
	}
	if(required)
	{
		// call reallocReductArrays(reduct_bytes);
		SgExprListExp* kPars = buildExprListExp();
		kPars->append_expression(buildOpaqueVarRefExp(SgName("reduct_bytes")));
		SgStatement *kCall = buildFunctionCallStmt("mvReductArraysToHost", buildVoidType(), kPars, body);
	  appendStatement(kCall, body);

    for(int i = 0; i < pl->args.size(); i++)
		{
			if(pl->args[i]->consideredAsReduction())
			{
        kPars = buildExprListExp();
				kPars->append_expression(buildOpaqueVarRefExp(SgName("*"+arg(i))));
				kCall = buildFunctionCallStmt("pop_op_dat_as_reduct", buildVoidType(), kPars, body);
	 			appendStatement(kCall, body);
			}
		}
		
		// free block_reduct memory
		kPars = buildExprListExp();
		kPars->append_expression( buildOpaqueVarRefExp(SgName("block_reduct")) );
		kCall = buildFunctionCallStmt("cudaFree", buildVoidType(), kPars, body);
  	appendStatement(kCall,body);
	}

	// Handle Const
	///////////////////////
	
	// We dont need to do anything here for const
}


