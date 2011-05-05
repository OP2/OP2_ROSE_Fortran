/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models the kernel subroutine for a direct loop.
 * Its declarations and statements do the following:
 *
 */

#ifndef KERNEL_SUBROUTINE_OF_DIRECT_LOOP_H
#define KERNEL_SUBROUTINE_OF_DIRECT_LOOP_H

#include <KernelSubroutine.h>
#include <DeviceDataSizesDeclarationDirectLoops.h>

class KernelSubroutineOfDirectLoop: public KernelSubroutine
{
  private:

		/*
		 * ======================================================
		 * Formal parameter in a direct loop for the offset
		 * in the shared memory variable assigned to each
		 * thread block (see Mike's Developers Guide, direct
		 * loops section)
		 * ======================================================
		 */
		
		SgVariableDeclaration * formalParameter_offsetS;
	

    /*
     * ======================================================
     * Formal parameter in a direct loop for the size
     * of the OP_SET
     * ======================================================
     */

    SgVariableDeclaration * formalParameter_setSize;

		/*
		 * ======================================================
		 * Size of the warp for OP2 when implementing direct
		 * loops. This value must be passed to CUDA kernels of
		 * direct loops because the access to the related		 
		 * intrinsic CUDA Fortran variable gives place to an
		 * error. 
		 * ======================================================
		 */

		SgVariableDeclaration * formalParameter_warpSizeOP2;
	

    /*
     * ======================================================
     * A local variable to count set elements
     * ======================================================
     */

    SgVariableDeclaration * variable_setElementCounter;

		/*
		 * ======================================================
		 * A local variable to count data for each element
		 * ======================================================
		 */
		SgVariableDeclaration * variable_dataPerElementCounter;
	
		/*
		 * ======================================================
		 * A local variable "tid" containing the threadid%x value
		 * moduled by the warpSizeOP2 variable value
		 * ======================================================
		 */
		SgVariableDeclaration * variable_tIdModWarpSize;

		/*
		 * ======================================================
		 * A local variable to count data for each element
		 * ======================================================
		 */
		SgVariableDeclaration * variable_offsetInThreadBlock;
	
		/*
		 * ======================================================
		 * A local variable to count data for each element
		 * ======================================================
		 */
		SgVariableDeclaration * variable_numberOfThreadInWarpOrRemainingElems;

		/*
		 * ======================================================
		 * A local variable storing the displacement for each
		 * thread inside the autoshared variable
		 * ======================================================
		 */
		SgVariableDeclaration * variable_displacementInAutoshared;

		/*
		 * ======================================================
		 * The following expression is 4 in case of real(4)
		 * data and 8 in case of real(8) data
		 * ======================================================
		 */
	
	  SgExpression * compilerExpr_opDatKindSize;

  private:

    /*
     * ======================================================
     * Creates the actual parameters for the call to the
     * device version of the user subroutine
     * ======================================================
     */
    SgStatement *
    createUserSubroutineCall (UserDeviceSubroutine & userDeviceSubroutine,
        ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Creates the statements inside the kernel subroutine
     * ======================================================
     */
    void
    createStatements (UserDeviceSubroutine & userDeviceSubroutine,
        ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Creates local variables inside the kernel subroutine
     * ======================================================
     */
    void
    createLocalVariables ( ParallelLoop & parallelLoop );

    void
    create_OP_DAT_FormalParameters (ParallelLoop & parallelLoop);

	
		/*
		 * ======================================================
		 * The set size formal parameter is declared after
		 * the op_dat arguments (and other arguments), but it
		 * is used in their definition. Therefore, we have
		 * to distinguish the act of building it, to the
		 * act of appending it to the subroutine scope
		 * ======================================================
		 */
    void
    createSetSizeFormalParameter ();

		void
		appendSetSizeFormalParameter ( );

		void
		createAndAppendOffsetSFormalParameter ();

		void
		createAndAppendWarpSizeOP2FormalParameter ();


		/*
		 * ======================================================
		 * Stores in compilerExpr_opDatKindSize either 4
		 * for real(4) dats or 8 for real(8) dats
		 * ======================================================
		 */
		void
		detectOPDatsBaseKindType ( ParallelLoop & parallelLoop );


		/*
		 * ======================================================
		 * Builds the statements included in the main
		 * set elements-based loop
		 * ======================================================
		 */
		SgBasicBlock *
		buildMainLoopStatements ( ParallelLoop & parallelLoop, SgScopeStatement * scopeStatement );
	

		/*
		 * ======================================================
		 * Builds the assignments of shared memory and local 
		 * thread variables, if needed
		 * ======================================================
		 */	
		SgBasicBlock *
		createSharedMemAndLocalThreadVarsAssignments ( 
      ParallelLoop & parallelLoop, SgScopeStatement * scopeStatement );
	
  public:

    KernelSubroutineOfDirectLoop (std::string const & subroutineName,
        UserDeviceSubroutine & userDeviceSubroutine,
				DeviceDataSizesDeclarationDirectLoops & DeviceDataSizesDeclarationDirectLoops,
        ParallelLoop & parallelLoop, SgScopeStatement * moduleScope);
};

#endif
