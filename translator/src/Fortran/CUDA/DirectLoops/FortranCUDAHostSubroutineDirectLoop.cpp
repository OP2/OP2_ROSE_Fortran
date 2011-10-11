#include <FortranCUDAHostSubroutineDirectLoop.h>
#include <FortranParallelLoop.h>
#include <FortranCUDAModuleDeclarations.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <CommonNamespaces.h>
#include <Debug.h>
#include <CUDA.h>

SgStatement *
FortranCUDAHostSubroutineDirectLoop::createKernelFunctionCallStatement ()
{
  using SageBuilder::buildDotExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildFunctionRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildNullExpression;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statement to call CUDA kernel", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::opDatDimensions)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::opDatCardinalities)));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isRead (i))
      {
        if (parallelLoop->isGlobalScalar (i))
        {
          actualParameters->append_expression (buildVarRefExp (
              variableDeclarations->get (OP2::VariableNames::getOpDatHostName (
                  i))));
        }
        else
        {
          actualParameters->append_expression (buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::getOpDatDeviceName (i))));
        }
      }
      else if (parallelLoop->isDirect (i) || parallelLoop->isIndirect (i))
      {
        actualParameters->append_expression (buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::getOpDatDeviceName (
                i))));
      }
    }
  }

  SgExpression * dotExpression = buildDotExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::getOpSetName ())),
      buildOpaqueVarRefExp (OP2::VariableNames::size, subroutineScope));

  actualParameters->append_expression (dotExpression);

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::warpSize)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::sharedMemoryOffset)));

  SgCudaKernelExecConfig * kernelConfiguration = new SgCudaKernelExecConfig (
      RoseHelper::getFileInfo (), buildVarRefExp (variableDeclarations->get (
          CUDA::blocksPerGrid)), buildVarRefExp (variableDeclarations->get (
          CUDA::threadsPerBlock)), buildVarRefExp (variableDeclarations->get (
          CUDA::sharedMemorySize)), buildNullExpression ());

  kernelConfiguration->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgCudaKernelCallExp * kernelCallExpression = new SgCudaKernelCallExp (
      RoseHelper::getFileInfo (), buildFunctionRefExp (
          calleeSubroutine->getSubroutineName (), subroutineScope),
      actualParameters, kernelConfiguration);

  kernelConfiguration->set_endOfConstruct (RoseHelper::getFileInfo ());

  return SageBuilder::buildExprStatement (kernelCallExpression);

  //  string const kernelLaunchString = calleeSubroutine->getSubroutineName ()
  //      + "<<<" + RoseHelper::getFirstVariableName (variableDeclarations->get (
  //      CUDA::blocksPerGrid)) + ", " + RoseHelper::getFirstVariableName (
  //      variableDeclarations->get (CUDA::threadsPerBlock)) + ", "
  //      + RoseHelper::getFirstVariableName (variableDeclarations->get (
  //          CUDA::sharedMemorySize)) + ">>>";
  //
  //  SgExprStatement * callStatement = buildFunctionCallStmt (kernelLaunchString,
  //      buildVoidType (), actualParameters, subroutineScope);

 // return callStatement;
}

void
FortranCUDAHostSubroutineDirectLoop::createCUDAKernelInitialisationStatements ()
{
  using boost::lexical_cast;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageInterface::appendStatement;
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
      buildVarRefExp (variableDeclarations->get (CUDA::blocksPerGrid)),
      buildIntVal (nblocks));

  appendStatement (assignmentStatement1, subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (CUDA::threadsPerBlock)),
      buildIntVal (nthreads));

  appendStatement (assignmentStatement2, subroutineScope);

  SgExprStatement * assignmentStatement3 =
      buildAssignStatement (buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::warpSize)), buildOpaqueVarRefExp (
          OP2::VariableNames::warpSizeMacro, subroutineScope));

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
    Debug::getInstance ()->errorMessage (
        "The shared memory size will be set to zero during kernel launch",
        __FILE__, __LINE__);
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
      buildVarRefExp (variableDeclarations->get (CUDA::sharedMemorySize)),
      buildIntVal (max (sharedOpDatMemorySize, sharedReductionMemorySize)));

  appendStatement (assignmentStatement4, subroutineScope);

  /*
   * ======================================================
   * Initialise the offset into shared memory to <shared
   * memory size> * <warp size>
   * ======================================================
   */

  SgMultiplyOp * multiplyExpression5 =
      buildMultiplyOp (buildVarRefExp (variableDeclarations->get (
          CUDA::sharedMemorySize)), buildOpaqueVarRefExp (
          OP2::VariableNames::warpSizeMacro, subroutineScope));

  SgExprStatement * assignmentStatement5 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::sharedMemoryOffset)), multiplyExpression5);

  appendStatement (assignmentStatement5, subroutineScope);

  SgMultiplyOp * multiplyExpression6 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (CUDA::sharedMemorySize)), buildVarRefExp (
      variableDeclarations->get (CUDA::threadsPerBlock)));

  SgExprStatement * assignmentStatement6 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (CUDA::sharedMemorySize)),
      multiplyExpression6);

  appendStatement (assignmentStatement6, subroutineScope);
}

void
FortranCUDAHostSubroutineDirectLoop::createCUDAKernelLocalVariableDeclarationsForDirectLoop ()
{
  Debug::getInstance ()->debugMessage (
      "Creating CUDA configuration parameters", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (OP2::VariableNames::sharedMemoryOffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::sharedMemoryOffset,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (OP2::VariableNames::warpSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::warpSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}

void
FortranCUDAHostSubroutineDirectLoop::createStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  createCUDAKernelInitialisationStatements ();

  appendStatement (createTransferOpDatStatements (), subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }

  appendStatement (createKernelFunctionCallStatement (), subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (CUDA::threadSynchRet)),
      CUDA::createHostThreadSynchronisationCallStatement (subroutineScope));

  appendStatement (assignmentStatement2, subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }
}

void
FortranCUDAHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  createOpDatDimensionsDeclaration ();

  createOpDatCardinalitiesDeclaration ();

  createDataMarshallingLocalVariableDeclarations ();

  createCUDAKernelLocalVariableDeclarations ();

  createCUDAKernelLocalVariableDeclarationsForDirectLoop ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionLocalVariableDeclarations ();
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
