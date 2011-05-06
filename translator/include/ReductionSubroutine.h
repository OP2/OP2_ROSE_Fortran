/*
 * Written by Carlo Bertolli and Adam Betts
 *
 * This class generates code implementing reduction
 * of op_gbl variables marked with OP_INC, OP_MAX or OP_MIN
 * accessing codes
 */

#ifndef REDUCTION_SUBROUTINE_H
#define REDUCTION_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>

class ReductionSubroutine: public Subroutine
{
  private:

    /*
     * ======================================================
     * Generates a call statement to synchthreads
     * ======================================================
     */

    SgStatement *
    createCallToSynchThreads ();

    /*
     * ======================================================
     * Generates the reduction subroutine formal parameters
     * ======================================================
     */

    void
    createFormalParameters ();

    /*
     * ======================================================
     * Generates the reduction subroutine local variables
     * ======================================================
     */

    void
    createLocalVariables ();

    /*
     * ======================================================
     * Generates the reduction subroutine statements
     * ======================================================
     */

    void
    createStatements ();

    /*
     * ======================================================
     * The first formal parameter in a reduction subroutine
     * is the device variable on which the result of local
     * reductions is stored by the first thread in the block
     * ======================================================
     */

    SgVariableDeclaration * formalParameter_reductionResultDevice;

    /*
     * ======================================================
     * The second formal parameter in a reduction subroutine
     * is the value of the reduction veriable produced by each
     * thread and it is passed by value
     * ======================================================
     */

    SgVariableDeclaration * formalParameter_inputValue;

    /*
     * ======================================================
     * The third formal parameter in a reduction subroutine
     * is the warp size: currently, in CUDA Fortran it is not
     * possible to access the warpsize variable value in a
     * device subroutine (only possible in global ones)
     * ======================================================
     */

    SgVariableDeclaration * formalParameter_warpSize;

    /*
     * ======================================================
     * The fourth formal parameter in a reduction subroutine
     * is the offset from which the automatically allocated
     * shared memory variable space is reserved for use by
     * the reduction subroutine
     * ======================================================
     */

    SgVariableDeclaration * formalParameter_sharedMemStartOffset;

    /*
     * ======================================================
     * Local variable: automatically allocated shared memory
     * variable
     * ======================================================
     */

    SgVariableDeclaration * localVariables_autoshared;

    /*
     * ======================================================
     * Local variable: automatically allocated shared memory
     * variable
     * ======================================================
     */

    SgVariableDeclaration * localVariables_threadID;

    /*
     * ======================================================
     * A local variable to count
     * ======================================================
     */

    SgVariableDeclaration * localVariables_IterationCounter;

    /*
     * ======================================================
     * The following variables are provided as part of the
     * CUDA library
     * ======================================================
     */

    SgVarRefExp * variable_Threadidx;

    SgVarRefExp * variable_Blockdim;

    SgVarRefExp * variable_X;

    /*
     * ======================================================
     * The following subroutine is provided as part of the
     * CUDA library
     * ======================================================
     */

    SgFunctionSymbol * synchthreadsFunctionSymbol;

    /*
     * ======================================================
     * type of the reduction variable (must be an array type)
     * ======================================================
     */
    SgArrayType * reductionVariableType;

  public:
	
    ReductionSubroutine (std::string const & subroutineAndVariableName,
        SgScopeStatement * moduleScope, SgArrayType * reductionVariableType);
				

	/*
	 * ======================================================
	 * Static functions
	 * ======================================================
	 */
	
				
	/*
	 * ======================================================
	 * Generates all proper reduction subroutines: one per
	 * type and kind
	 * ======================================================
	 */
	
	static std::map < unsigned int, SgProcedureHeaderStatement *>
	generateReductionSubroutines ( 
		ParallelLoop & parallelLoop, SgScopeStatement * scopeStatement );
	
};


#endif

