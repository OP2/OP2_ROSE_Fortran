/*
 * Written by Adam Betts
 *
 * The parallel loop class used to recognise and manipulate 'op_par_loop' calls
 * in Fortran code.
 *
 * This class extends AstSimpleProcessing since that class provides
 * utilities to traverse the Abstract Syntax Tree (AST).
 */

#ifndef OPPARLOOP_H
#define OPPARLOOP_H

#include <rose.h>

// macros for positions inside the op_par_loop arguments
// we assume the following style:
// op_par_loop_* ( kernel, iteration_set,
//								 arg0, idx0, map0, acc0,
//								 ...
//								 argN-1, idxN-1, mapN-1, accN-1
//							 )
#define ARG_LINE_LENGTH 4
#define OP_SET_INDEX_IN_ARG 1
#define BASE_OPDAT_ARG_INDEX 2


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
     * The kernel output files
     */
    std::vector <SgSourceFile *> kernelOutputFiles;

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
    std::vector <SgStatement*> statements;
    std::vector <SgScopeStatement*> scopes;

	
		/*
		 * SgFunctionDeclaration object corresponding to input user kernel (copied inside the new module)
		 */
		SgProcedureHeaderStatement * userKernelFunction;
		
		/*
		 * References to all subroutines found during the visit of the input files
		 */
		std::vector < SgProcedureHeaderStatement * > inputSubroutines;
		
		/*
		 * Number of arguments in a OP_DAT line of op_par_loop
		 */
		int argLineLength;
	
		/*
		 * compute the number of op_dat arguments passed to a op_par_loop based on total arguments number
		 * It is equal to the subscript of op_par_loop_*, but at a certain point we are going to remove it!
		 * The formula is: (totArgNum - 2) / argLineLength
		 */
		inline unsigned int computeOpDatArgNumber ( int argSize ) 
		{ return ( (unsigned int ) (argSize - 2) / argLineLength ); }
	

		inline void setUserKernelDeclaration ( SgProcedureHeaderStatement * kernelRef )
		{ userKernelFunction = kernelRef; }
	
	
    /*
     * ====================================================================================================
     * Private functions
     * ====================================================================================================
     */

    void
    setSourcePosition (SgLocatedNode* locatedNode);

    /*
     * used to tell Rose that the generated node is actually generated by ourselves, and not as some transformation of the
     * parser result (i.e. from input source files)
     */
    void
    setSourcePositionCompilerGenerated (SgLocatedNode * locatedNode);

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
    createHostDeviceLocals (SgScopeStatement* scope);

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
    void
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
		lookupArgumentsTypes ( std::vector<SgType *> * argTypes, SgExpressionPtrList& args, int loopDatArgNumber );
	
	
		SgProcedureHeaderStatement *
		buildMainKernelRoutine ( std::string subroutineName,
														 SgScopeStatement * scope,
														 unsigned int opArgNumber, 
														 SgExpressionPtrList& args
													 );
	
	
	
		/*
		 * Creates the whole module including all subroutines implementing an op_par_loop
		 */
		void
		createCudaModule ( std::string kernelName, SgSourceFile& sourceFile, SgExpressionPtrList& args );
	
    /*
     * Creates a Fortran module to which the Fortran subroutines will
     * be inserted
     */
    SgModuleStatement*
    createModule (std::string kernelName, SgSourceFile& sourceFile);

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
    OpParLoop (SgProject *project);

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
