


/*  Open source copyright declaration based on BSD open source template:
 *  http://www.opensource.org/licenses/bsd-license.php
 * 
 * Copyright (c) 2011-2012, Adam Betts, Carlo Bertolli
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "FortranCUDAHostSubroutineDirectLoop.h"
#include "FortranParallelLoop.h"
#include "FortranCUDAModuleDeclarations.h"
#include "FortranTypesBuilder.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "RoseHelper.h"
#include "CompilerGeneratedNames.h"
#include "OP2.h"
#include "Debug.h"
#include "Exceptions.h"
#include "CUDA.h"
#include <Globals.h>

void
FortranCUDAHostSubroutineDirectLoop::createKernelFunctionCallStatement (
    SgScopeStatement * scope)
{
  using namespace SageInterface;
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace OP2::RunTimeVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statement to call CUDA kernel", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (variableDeclarations->getReference (
      opDatDimensions));

  actualParameters->append_expression (variableDeclarations->getReference (
      opDatCardinalities));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i))
      {
        actualParameters->append_expression (
            variableDeclarations->getReference (getReductionArrayDeviceName (i)));
      }
      else if (parallelLoop->isDirect (i))
      {        
/* Carlo: no more opDatNDevice as arguments */        
/*        actualParameters->append_expression (
            moduleDeclarations->getDeclarations()->getReference (getOpDatDeviceName (i) +
            parallelLoop->getUserSubroutineName () + postfixName));*/
      }
      else if (parallelLoop->isRead (i))
      {
        if (parallelLoop->isArray (i))
        {
          actualParameters->append_expression (
              variableDeclarations->getReference (getOpDatDeviceName (i)));
        }
        else
        {
          actualParameters->append_expression (
              variableDeclarations->getReference (getOpDatHostName (i)));
        }
      }
    }
  }

  SgExpression * dotExpression = buildDotExp (
    variableDeclarations->getReference (getOpSetName ()),
      buildDotExp ( buildOpaqueVarRefExp (Fortran::setPtr, scope),
          buildOpaqueVarRefExp (size, scope)));      

  actualParameters->append_expression (dotExpression);

  actualParameters->append_expression (variableDeclarations->getReference (
      warpSize));

  actualParameters->append_expression (variableDeclarations->getReference (
      sharedMemoryOffset));

  SgCudaKernelExecConfig * kernelConfiguration = new SgCudaKernelExecConfig (
      RoseHelper::getFileInfo (), variableDeclarations->getReference (
          CUDA::blocksPerGrid), variableDeclarations->getReference (
          CUDA::threadsPerBlock), variableDeclarations->getReference (
          CUDA::sharedMemorySize), buildNullExpression ());

  kernelConfiguration->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgCudaKernelCallExp * kernelCallExpression = new SgCudaKernelCallExp (
      RoseHelper::getFileInfo (), buildFunctionRefExp (
          calleeSubroutine->getSubroutineName (), subroutineScope),
      actualParameters, kernelConfiguration);

  kernelCallExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  appendStatement (buildExprStatement (kernelCallExpression), scope);
}

void
FortranCUDAHostSubroutineDirectLoop::createCUDAKernelInitialisationStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace OP2::Macros;
  using namespace CUDA;
  using boost::lexical_cast;
  using std::string;
  using std::max;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA kernel initialisation statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * The following values are copied from Mike Giles'
   * implementation and may be subject to future changes
   * \warning: removed magic number, now in CUDA.h
   * ======================================================
   */
  //int const nblocks = 200;

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::blocksPerGrid), buildIntVal (
          nblocksDirectLoops));

  appendStatement (assignmentStatement1, subroutineScope);

  /*
   * ======================================================
   * Statement to initialise the number of threads per block
   * in the CUDA kernel: calls a proper function
   * ======================================================
   */

  Debug::getInstance ()->debugMessage ("Creating call to block size setting function",
    Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    
  appendBlockSizeFunctionCall (subroutineScope);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (warpSize), buildOpaqueVarRefExp (
          warpSizeMacro, subroutineScope));

  appendStatement (assignmentStatement3, subroutineScope);

  /*
   * ======================================================
   * Compute the value of shared memory passed at CUDA kernel
   * launch.
   *
   * An OP_DAT is only copied into shared memory if its
   * dimension exceeds 1 and it is not an OP_GBL. To
   * compute the value, therefore, we need to get the maximum
   * of (dimension * size of primitive type) across all
   * OP_DATs
   * ======================================================
   */

  unsigned int sharedOpDatMemorySize = 0;
  unsigned int sharedReductionMemorySize = 0;
  unsigned int reductionMemoryRequirements = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isGlobal (i) == false)
      {
        unsigned int opDatMemoryRequirements = parallelLoop->getOpDatDimension (
            i) * parallelLoop->getSizeOfOpDat (i);

        if (opDatMemoryRequirements > sharedOpDatMemorySize)
        {
          sharedOpDatMemorySize = opDatMemoryRequirements;
        }
      }
      else if (parallelLoop->isReductionRequired (i))
      {        
        reductionMemoryRequirements = max (
          reductionMemoryRequirements,
          parallelLoop->getOpDatDimension (i) * parallelLoop->getSizeOfOpDat (i));
      }
    }
  }

  SgExprStatement * assignmentStatement4 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::sharedMemorySize), buildIntVal (
          sharedOpDatMemorySize));

  appendStatement (assignmentStatement4, subroutineScope);

  /*
   * ======================================================
   * Initialise the offset into shared memory to <shared
   * memory size> * <warp size>
   * ======================================================
   */

  SgMultiplyOp * multiplyExpression5 = buildMultiplyOp (
      variableDeclarations->getReference (CUDA::sharedMemorySize),
      buildOpaqueVarRefExp (warpSizeMacro, subroutineScope));

  SgExprStatement * assignmentStatement5 = buildAssignStatement (
      variableDeclarations->getReference (sharedMemoryOffset),
      multiplyExpression5);

  appendStatement (assignmentStatement5, subroutineScope);

  SgMultiplyOp * multiplyExpression6 = buildMultiplyOp (
      variableDeclarations->getReference (CUDA::sharedMemorySize),
      variableDeclarations->getReference (CUDA::threadsPerBlock));

  SgExprStatement * assignmentStatement6 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::sharedMemorySize),
      multiplyExpression6);

  appendStatement (assignmentStatement6, subroutineScope);
 
  /*
   * ======================================================
   * Compute shared memory size as maximum between the
   * number of bytes needed by the staging and the ones
   * needed by the reduction (only in case of reduction)
   * ======================================================
   */
  if ( parallelLoop->isReductionRequired () )
  {
    SgExpression * sharedMemoryBytesPerWholeReduction = buildMultiplyOp (
      buildIntVal (reductionMemoryRequirements),
      variableDeclarations->getReference (CUDA::threadsPerBlock));
    
    SgFunctionSymbol * maxFunctionSymbol =
        FortranTypesBuilder::buildNewFortranFunction ("max",
            subroutineScope);

    SgExprListExp * actualParameters = buildExprListExp (
      variableDeclarations->getReference (CUDA::sharedMemorySize),
      sharedMemoryBytesPerWholeReduction);

    SgFunctionCallExp * maxSharedMemoryCall = buildFunctionCallExp (
        maxFunctionSymbol, actualParameters);
      
    SgExprStatement * assignFinalSharedMemorySize = buildAssignStatement (
      variableDeclarations->getReference (CUDA::sharedMemorySize),
      maxSharedMemoryCall);
      
    appendStatement (assignFinalSharedMemorySize, subroutineScope);
  }
}

void
FortranCUDAHostSubroutineDirectLoop::createCUDAKernelActualParameterDeclarations ()
{
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA configuration parameters", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (sharedMemoryOffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          sharedMemoryOffset, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (
      warpSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          warpSize, FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}

void
FortranCUDAHostSubroutineDirectLoop::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  initialiseNumberOfOpArgs (subroutineScope);
  
  appendPopulationOpArgArray (subroutineScope);
      
  if ( Globals::getInstance ()->getIncludesMPI () )
    appendCallMPIHaloExchangeFunction (subroutineScope);
  
  createEarlyExitStatementNewLibrary (subroutineScope);

  initialiseProfilingVariablesDeclaration ();

  createStartTimerHost ();
      
  createCUDAKernelInitialisationStatements ();

  appendStatement (createTransferOpDatStatements (), subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }

  createEndTimerHost ();
  createEndTimerSynchroniseHost ();
  createElapsedTimeHost ();
  createAccumulateTimesHost ();

  createStartTimerKernel ();

  createKernelFunctionCallStatement (subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::threadSynchRet),
      CUDA::createHostThreadSynchronisationCallStatement (subroutineScope));

  appendStatement (assignmentStatement2, subroutineScope);

  createEndTimerKernel ();
  createEndTimerSynchroniseKernel ();
  createElapsedTimeKernel ();
  createAccumulateTimesKernel ();

  createStartTimerHost ();


  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }

  createDeallocateStatements ();
  
  createDumpOfOutputStatements (subroutineScope,
    OP2::FortranSpecific::RunTimeFunctions::getDumpOpDatFromDeviceFunctionName);

  if ( Globals::getInstance ()->getIncludesMPI () )
  {
    appendCallsToMPIReduce (subroutineScope);
    appendCallMPISetDirtyBit (subroutineScope);
  }
  
  createEndTimerHost ();
  createEndTimerSynchroniseHost ();
  createElapsedTimeHost ();
  createAccumulateTimesHost ();
}

void
FortranCUDAHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Declare array of op_args and variable with number
   * of parameters
   * ======================================================
   */
  createCommonLocalVariableDeclarations (subroutineScope);
      
  createOpDatDimensionsDeclaration ();

  createOpDatCardinalitiesDeclaration ();

  createDataMarshallingDeclarations ();

  createCUDAConfigurationLaunchDeclarations ();

  createCUDAKernelActualParameterDeclarations ();

  createDumpOfOutputDeclarations (subroutineScope);
  
  createIterationVariablesDeclarations ();
  
  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }

  /*
   * ======================================================
   * Profiling declarations. Eventually, this should only
   * be done if a certain compiler option is turned on
   * ======================================================
   */

  createProfilingVariablesDeclaration ();

}

FortranCUDAHostSubroutineDirectLoop::FortranCUDAHostSubroutineDirectLoop (
    SgScopeStatement * moduleScope, FortranKernelSubroutine * kernelSubroutine,
    FortranParallelLoop * parallelLoop,
    FortranCUDAOpDatCardinalitiesDeclaration * cardinalitiesDeclaration,
    FortranOpDatDimensionsDeclaration * dimensionsDeclaration,
    FortranCUDAModuleDeclarations * moduleDeclarations) :
  FortranCUDAHostSubroutine (moduleScope, kernelSubroutine, parallelLoop,
      cardinalitiesDeclaration, dimensionsDeclaration, moduleDeclarations)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of direct loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
