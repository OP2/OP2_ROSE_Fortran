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

SgStatement *
FortranCUDAHostSubroutineDirectLoop::createKernelFunctionCallStatement ()
{
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
        actualParameters->append_expression (
            variableDeclarations->getReference (getOpDatDeviceName (i)));
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
      buildOpaqueVarRefExp (size, subroutineScope));

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

  return buildExprStatement (kernelCallExpression);
}

void
FortranCUDAHostSubroutineDirectLoop::createCUDAKernelInitialisationStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace OP2::Macros;
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
   * ======================================================
   */
  int const nblocks = 200;
  int const nthreads = 128;

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::blocksPerGrid), buildIntVal (
          nblocks));

  appendStatement (assignmentStatement1, subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::threadsPerBlock), buildIntVal (
          nthreads));

  appendStatement (assignmentStatement2, subroutineScope);

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
        unsigned int reductionMemoryRequirements =
            parallelLoop->getOpDatDimension (i) * parallelLoop->getSizeOfOpDat (
                i);

        if (reductionMemoryRequirements > sharedReductionMemorySize)
        {
          sharedReductionMemorySize = reductionMemoryRequirements;
        }
      }
    }
  }

  if (sharedOpDatMemorySize == 0 && sharedReductionMemorySize == 0)
  {
    throw Exceptions::CUDA::SharedMemorySizeException (
        "The shared memory size will be set to zero during kernel launch");
  }
  else
  {
    Debug::getInstance ()->debugMessage (
        "The shared memory size will be set to " + lexical_cast <string> (max (
            sharedOpDatMemorySize, sharedReductionMemorySize))
            + " during kernel launch", Debug::OUTER_LOOP_LEVEL, __FILE__,
        __LINE__);
  }

  SgExprStatement * assignmentStatement4 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::sharedMemorySize), buildIntVal (
          max (sharedOpDatMemorySize, sharedReductionMemorySize)));

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

  createCUDAKernelInitialisationStatements ();

  appendStatement (createTransferOpDatStatements (), subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }

  appendStatement (createKernelFunctionCallStatement (), subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::threadSynchRet),
      CUDA::createHostThreadSynchronisationCallStatement (subroutineScope));

  appendStatement (assignmentStatement2, subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }

  createDeallocateStatements ();
}

void
FortranCUDAHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createOpDatDimensionsDeclaration ();

  createOpDatCardinalitiesDeclaration ();

  createDataMarshallingDeclarations ();

  createCUDAConfigurationLaunchDeclarations ();

  createCUDAKernelActualParameterDeclarations ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }
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
