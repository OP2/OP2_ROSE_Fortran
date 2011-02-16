/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * The parallel loop class used to recognise and manipulate 'op_par_loop' calls
 * in Fortran code.
 *
 * This class extends AstSimpleProcessing since that class provides
 * utilities to traverse the Abstract Syntax Tree (AST).
 */

// TODO: restructure in such a way that we have a single OpParLoop object per each op_par_loop call 
// found in the code (or an additional data structure per each call)

#ifndef OPPARLOOP_H
#define OPPARLOOP_H

#include <rose.h>


#include "OP2CommonDefinitions.h"
#include "OpDeclaredVariables.h"



class OpParLoop: public AstSimpleProcessing
{
  private:
    /*
     * ====================================================================================================
     * Private data
     * ====================================================================================================
     */

    /*
     * The project which the source-to-source translator operates upon
     */
    SgProject *project;

		/*
		 * The previously filled OpDeclaredVariables object, used to retrieve op_dat types 
		 * (and all other declarations, if needed)
		 */
		OpDeclaredVariables * opDeclaredVariables;
	
		/*
		 *
		 */
		std::string kernelName;
	
    /*
     * The kernel output files
     */
    std::vector <SgSourceFile *> kernelOutputFiles;

		/*
		 * Number of arguments in a OP_DAT line of op_par_loop
		 */
		int argLineLength;	
	
    /*
     * Each 'op_par_loop' in Fortran takes 2+4n arguments.
     * We need the number n to declare a certain number of variables
     * in the host function
     */
    int numberOfArgumentGroups;
	
    /*
     * The statements to append to the file and their associated
     * scope stack
     */
    std::vector < SgStatement * > statements;
    std::vector < SgScopeStatement * > scopes;
	
		/*
		 * SgFunctionDeclaration object corresponding to input user kernel (copied inside the new module)
		 */
		SgProcedureHeaderStatement * userKernelFunction;
		
		/*
		 * References to all subroutines found during the visit of the input files
		 */
		std::vector < SgProcedureHeaderStatement * > inputSubroutines;
		
	
		/*
		 * set of declarations of local variables in the host routine (CRet, data0Size, ..., dataN-1Size)
		 */
		std::vector < SgVariableDeclaration * > declaredHostRoutineLocals;
	
		/*
		 * set of declarations of variables used in the host routine to transform a C pointer into a Fortran pointer
		 * (argument0, ..., argumentN-1, c2fPtr0, ..., c2fPtrN-1)
		 */	
		std::vector < SgVariableDeclaration * > declaredC2FortranVariables;

		/*
		 * set of declarations of CUDA configuration variables
		 * (bsize, gsize, etc.. )
		 */	
		std::vector < SgVariableDeclaration * > declaredCUDAConfigurationParameters;

		/*
		 * set of declarations of op_dat formal parameters
		 * (arg0, ..., argN-1)
		 */
		std::vector < SgVariableDeclaration * > opDatFormalArgs;

		/*
		 * data types of the op_par_loop actual arguments ordered w.r.t. the input ordering
		 */
		std::vector < SgType * > actualArgumentsTypes;
	
		/*
		 * dimension values passed to op_decl_dat calls ordered w.r.t. the input ordering (the same of actualArgumentTypes)
		 */
		std::vector < int > actualArgumentsDimValues;
	
		/*
		 * set of declarations of indirection index formal parameters
		 * (idx0, ..., idxN-1)
		 */
		std::vector < SgVariableDeclaration * > indirectionFormalArgs;

		/*
		 * set of declarations of op_map formal parameters
		 * (map0, ..., mapN-1)
		 */
		std::vector < SgVariableDeclaration * > mapFormalArgs;

		/*
		 * set of declarations of access formal parameters
		 * (access0, ..., accessN-1)
		 */
		std::vector < SgVariableDeclaration * > accessFormalArgs;
	
		/*
		 * declaration of formal argument corresponding to op_set iteration variable
		 */
		SgVariableDeclaration * iterationSetFormalArg;
		
		/*
		 * compute the number of op_dat arguments passed to a op_par_loop based on total arguments number
		 * It is equal to the subscript of op_par_loop_*, but at a certain point we are going to remove it!
		 * The formula is: (totArgNum - 2) / argLineLength
		 */
		inline unsigned int 
		computeOpDatArgNumber ( int argSize ) 
		{ return ( (unsigned int ) (argSize - 2) / argLineLength ); }
	

		 void setUserKernelDeclaration ( SgProcedureHeaderStatement * kernelRef )
		{ userKernelFunction = kernelRef; }
	
	
    /*
     * ====================================================================================================
     * Private functions
     * ====================================================================================================
     */
	
	
	bool checkAlreadyDeclared ( SgVariableDeclaration * possiblyAvoidCopy, SgFunctionParameterList * newParList);

	
	void createAndAppendUserKernelFormalParameters ( SgFunctionParameterList * newParList,
																									SgFunctionParameterList * originalParams, 
																									SgScopeStatement * subroutineScope
																									);
	
	
		/*
		 * This function is called before the creation of the CUDA module to retrieve the op_par_loop arguments data types
		 * It fills up the actualArgumentsTypes vector by comparing op_par_loop arguments with opDeclaredVariables variables
		 */

		void retrieveArgumentsTypes ( SgExpressionPtrList& args );
	
//    void
 //   setSourcePosition (SgLocatedNode* locatedNode);

    /*
     * used to tell Rose that the generated node is actually generated by ourselves, and not as some transformation of the
     * parser result (i.e. from input source files)
     */
//    void
//    setSourcePositionCompilerGenerated (SgLocatedNode * locatedNode);

		SgModuleStatement *
		buildClassDeclarationAndDefinition (std::string name, SgScopeStatement* scope);

    SgContainsStatement *
    buildContainsStatement (Sg_File_Info * fileInfo, SgScopeStatement * scope);

    void
    addUseStatements (SgSourceFile& sourceFile,
        SgDeclarationStatement* definingDeclaration, SgScopeStatement* scope);

    void
    addImplicitStatement (SgSourceFile& sourceFile,
        SgDeclarationStatement* definingDeclaration, SgScopeStatement* scope);

    void
    generateKernelSubroutine (SgSourceFile &sourceFile,
        std::string subroutineName, SgExpressionPtrList& args);

		void
	declareCUDAConfigurationParameters ( SgScopeStatement * scope );
//																				 SgExpressionPtrList& args
//																			 );

		void
		declareC2FortranVariables ( SgScopeStatement * scope );
	
    void
		createHostDeviceLocals ( SgScopeStatement* scope, 
														 SgExpressionPtrList& args
													 );

		void	
		createMainRoutineStatements ( SgScopeStatement * scope,
																	SgExpressionPtrList& args
	//																std::vector < SgVariableDeclaration * > ** decalredHostRoutineLocals,
	//																std::vector < SgVariableDeclaration * > ** declaredC2FortranVariables,
	//																std::vector < SgVariableDeclaration * > ** declaredCUDAConfigurationParameters
																);
	/*
     * Creates an integer declaration, representing the indirection of the data
     * set to be iterated, and attaches it to the formal parameter list
     */
    void
    createIndirectionDeclaration (SgFunctionParameterList* parameters,
        SgScopeStatement* scope);

    /*
     * Creates an OP_ACCESS declaration and attaches it to the
     * formal parameter list
     */
    void
    createOpAccessDeclaration (SgFunctionParameterList* parameters,
        SgScopeStatement* scope);

    /*
     * Creates an OP_DAT declaration and attaches it to the
     * formal parameter list
     */
    void
    createOpDatDeclaration (SgFunctionParameterList* parameters,
        SgScopeStatement* scope, SgType* opDatType);

    /*
     * Creates an OP_Map declaration and attaches it to the
     * formal parameter list
     */
    void
    createOpMapDeclaration (SgFunctionParameterList* parameters,
        SgScopeStatement* scope, SgType* opMapType);

    /*
     * Creates an OP_SET declaration and attaches it to the
     * formal parameter list
     */
    void
    createOpSetDeclaration (SgFunctionParameterList* parameters,
        SgScopeStatement* scope, SgType* opSetType);

    /*
     * Creates a char array declaration, which is the kernel name,
     * and attaches it to the formal parameter list
     */
    void
    createSubroutineName (SgFunctionParameterList* parameters,
        SgScopeStatement* scope);

    /*
     * Creates a host subroutine for the kernel
     */
    SgProcedureHeaderStatement *
		createHostSubroutine (std::string kernelName, SgExpressionPtrList& args,
													SgSourceFile& sourceFile, SgScopeStatement * scope );

		/*
		 * Copies the user kernel function definition, makes some modifications and append it to the scope
		 */
	
		SgProcedureHeaderStatement *
		generateCUDAUserKernel ( SgScopeStatement * scope );


		/*
		 * The following two functions are used to generate the main CUDA kernel routine: not implemented for now
		 */
		void 
		lookupArgumentsTypes ( std::vector<SgType *> * argTypes, 
													 SgExpressionPtrList& args, 
													 int loopDatArgNumber,
													 SgVariableDeclaration * setSizeFormalPar
												 );
	
	
	
		/*
		 * Builds the parameter of the user kernel subroutine called by the main kernel subroutine
		 */		
		SgExprListExp *
		buildUserKernelParams ( SgFunctionParameterList * mainKernelParameters,
														SgVarRefExp * iterSetVarRef,
														SgScopeStatement * subroutineScope
													 );

	
		SgProcedureHeaderStatement *
		buildMainKernelRoutine ( std::string subroutineName,
														 SgScopeStatement * scope,
														 unsigned int opArgNumber, 
														 SgExpressionPtrList& args,
														 SgProcedureHeaderStatement * kernelCUDAVersionDecl,
														 Sg_File_Info & fileInfo
													 );
	
	
	
		/*
		 * Creates a Fortran module to which the Fortran subroutines will
		 * be inserted
		 */
		SgModuleStatement*
		createModule (std::string kernelName, SgSourceFile& sourceFile);
	
	
		/*
		 * Creates the whole module including all subroutines implementing an op_par_loop
		 */
		SgProcedureHeaderStatement *
		createDirectLoopCudaModule ( std::string kernelName, SgSourceFile& sourceFile, SgExpressionPtrList& args );
	
		/*
		 * Transforms a op_par_loop_* in call to proper stub function, like op_par_loop_save_soln
		 */
		void fixParLoops ( SgFunctionCallExp * functionCallExp,
											 std::string kernelName,
											 SgProcedureHeaderStatement * hostSubroutine,
											 SgScopeStatement * scope,
											 std::string createdCUDAModuleName
										 );
    /*
     * Creates the source file to be unparsed
     */
    SgSourceFile*
    createSourceFile (std::string kernelName);

  public:
    /*
     * ====================================================================================================
     * Public functions
     * ====================================================================================================
     */

    /*
     * Constructor requires the source files contained in the
     * project to detect 'op_par_loops'
     */
		OpParLoop ( SgProject * project, OpDeclaredVariables * opDeclaredVariables );

    /*
     * Visit each vertex in the AST
     */
    virtual void
    visit (SgNode *n);

    /*
     * Generate output files for each kernel
     */
    void
    unparse ();
	
		/*
		 * Modifiers
		 */
		inline void setArgLineLength ( int _argLineLength ) { argLineLength = _argLineLength; }
		inline int getArgLineLength ( ) { return argLineLength; }
		
};


class deepVisitAndSetGeneration: public AstSimpleProcessing
{	
public:
	
	deepVisitAndSetGeneration () {}
	
	/*
	 * Special visit: for each node, sets its OutputInCodeGeneration flag to enable unparsing
	 */
	virtual void
	visit (SgNode *n);
	
};



#endif /* OPPARLOOP_H_ */
