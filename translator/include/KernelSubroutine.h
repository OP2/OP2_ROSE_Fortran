#ifndef KERNEL_SUBROUTINE_H
#define KERNEL_SUBROUTINE_H

#include <Subroutine.h>
#include <UserDeviceSubroutine.h>
#include <ParallelLoop.h>
#include <DeviceDataSizesDeclaration.h>


namespace
{
  std::string const variableName_threadidx = "threadidx";
  std::string const variableName_x = "x";
  std::string const variableName_blockidx = "blockidx";
  std::string const variableName_blockdim = "blockdim";
  std::string const variableName_griddim = "griddim";
}

class KernelSubroutine: public Subroutine
{
  protected:

		/*
		 * ======================================================
		 * The first formal parameter of the CUDA kernel in both
		 * direct and indirect loops is a
		 * variable containing the size information of the other
		 * formal parameters
		 * ======================================================
		 */

		SgVariableDeclaration * formalParameter_argsSizes;	

    /*
     * ======================================================
     * The OP_DAT formal parameters
     * ======================================================
     */

    std::map <unsigned int, SgVariableDeclaration *> formalParameter_OP_DATs;
		
		/*
		 * ======================================================
		 * Local thread variables
		 * ======================================================
		 */	
		std::map < unsigned int, SgVariableDeclaration * > localVariables_localThreadVariables;

		/*
		 * ======================================================
		 * autoshared variable
		 * ======================================================
		 */	
	
		SgVariableDeclaration * localVariables_autoshared;
		
  protected:

		void
		createArgsSizesFormalParameter (
			DeviceDataSizesDeclaration & deviceDataSizesDeclaration );


		/*
		 * ======================================================
		 * Creates the argI_l local thread variables
		 * for CUDA kernels of direct and indirect loops
		 * ======================================================
		 */
		 	
		void
		createLocalThreadVariables ( ParallelLoop & parallelLoop,
		  SgScopeStatement & scopeStatement, bool isDirectLoop );


		/*
		 * ======================================================
		 * Creates the autoshared variable: only for real(8)
		 * ======================================================
		 */
	
		void
		createAutosharedVariable ( ParallelLoop & parallelLoop, 
		  SgScopeStatement * scopeStatement );

		/*
		 * ======================================================
		 * Initialises local thread variables, for those
		 * for which it is needed
		 * ======================================================
		 */
	
		void
		initialiseLocalThreadVariables ( ParallelLoop & parallelLoop, 
		  SgScopeStatement * scopeStatement );


		void
		initialiseLocalThreadVariables ( ParallelLoop & parallelLoop,
		  SgScopeStatement * scopeStatement, SgVarRefExp * iterationVariableReference );
	
	

    KernelSubroutine (std::string const & subroutineName) :
      Subroutine (subroutineName + "_kernel")
    {
    }
};

#endif
