#include "CPPOpenCLHostSubroutineDirectLoop.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "RoseHelper.h"
#include "Debug.h"
#include "CompilerGeneratedNames.h"
#include "OpenCL.h"
#include "OP2.h"

SgStatement *
CPPOpenCLHostSubroutineDirectLoop::createKernelFunctionCallStatement ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace OP2::RunTimeVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to call OpenCL kernel", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * Assign the kernel pointer using the run-time support
   * for OpenCL
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::kernelPointer),
      OpenCL::OP2RuntimeSupport::getKernel (subroutineScope,
          calleeSubroutine->getSubroutineName ()));

  appendStatement (assignmentStatement1, block);

  /*
   * ======================================================
   * OpenCL kernel arguments for OP_DATs
   * ======================================================
   */

  unsigned int argumentCounter = 0;
  bool firstAssignment = true;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isDirect (i))
      {
        SgDotExp * dotExpression = buildDotExp (
            variableDeclarations->getReference (getOpDatName (i)),
            buildOpaqueVarRefExp (OP2::RunTimeVariableNames::data_d,
                subroutineScope));

        SgFunctionCallExp * kernelArgumentExpression =
            OpenCL::getSetKernelArgumentCallExpression (subroutineScope,
                variableDeclarations->getReference (OpenCL::kernelPointer),
                argumentCounter, OpenCL::getMemoryType (subroutineScope),
                dotExpression);

        if (firstAssignment)
        {
          firstAssignment = false;

          SgExprStatement * assignmentStatement = buildAssignStatement (
              variableDeclarations->getReference (OpenCL::errorCode),
              kernelArgumentExpression);

          appendStatement (assignmentStatement, block);
        }
        else
        {
          SgBitOrOp * orExpression = buildBitOrOp (
              variableDeclarations->getReference (OpenCL::errorCode),
              kernelArgumentExpression);

          SgExprStatement * assignmentStatement = buildAssignStatement (
              variableDeclarations->getReference (OpenCL::errorCode),
              orExpression);

          appendStatement (assignmentStatement, block);

        }

        argumentCounter++;
      }
    }
  }

  /*
   * ======================================================
   * OpenCL kernel argument for shared memory offset
   * ======================================================
   */
  SgFunctionCallExp * kernelArgumentExpression2 =
      OpenCL::getSetKernelArgumentCallExpression (subroutineScope,
          variableDeclarations->getReference (OpenCL::kernelPointer),
          argumentCounter, buildIntType (), variableDeclarations->getReference (
              sharedMemoryOffset));

  SgBitOrOp * orExpression2 = buildBitOrOp (variableDeclarations->getReference (
      OpenCL::errorCode), kernelArgumentExpression2);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode), orExpression2);

  appendStatement (assignmentStatement2, block);

  argumentCounter++;

  /*
   * ======================================================
   * OpenCL kernel argument for set size
   * ======================================================
   */

  SgArrowExp * arrowExpression3 = buildArrowExp (
      variableDeclarations->getReference (getOpSetName ()),
      buildOpaqueVarRefExp (OP2::RunTimeVariableNames::size, subroutineScope));

  SgFunctionCallExp * kernelArgumentExpression3 =
      OpenCL::getSetKernelArgumentCallExpression (subroutineScope,
          variableDeclarations->getReference (OpenCL::kernelPointer),
          argumentCounter, buildIntType (), arrowExpression3);

  SgBitOrOp * orExpression3 = buildBitOrOp (variableDeclarations->getReference (
      OpenCL::errorCode), kernelArgumentExpression3);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode), orExpression3);

  appendStatement (assignmentStatement3, block);

  argumentCounter++;

  /*
   * ======================================================
   * OpenCL kernel argument for dynamic shared memory size
   * ======================================================
   */

  SgFunctionCallExp * kernelArgumentExpression4 =
      OpenCL::getSetKernelArgumentCallExpression (subroutineScope,
          variableDeclarations->getReference (OpenCL::kernelPointer),
          argumentCounter, OpenCL::getSizeType (subroutineScope),
          variableDeclarations->getReference (OpenCL::sharedMemorySize));

  SgBitOrOp * orExpression4 = buildBitOrOp (variableDeclarations->getReference (
      OpenCL::errorCode), kernelArgumentExpression4);

  SgExprStatement * assignmentStatement4 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode), orExpression4);

  appendStatement (assignmentStatement4, block);

  /*
   * ======================================================
   * Assert statement
   * ======================================================
   */

  SgEqualityOp * equalityExpression1 = buildEqualityOp (
      variableDeclarations->getReference (OpenCL::errorCode),
      buildOpaqueVarRefExp (OpenCL::CL_SUCCESS, subroutineScope));

  appendStatement (buildExprStatement (
      OpenCL::OP2RuntimeSupport::getAssertMessage (subroutineScope,
          equalityExpression1, buildStringVal (
              "Error setting OpenCL kernel arguments"))), block);

  /*
   * ======================================================
   * Execute kernel statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement5 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode),
      OpenCL::getEnqueueKernelCallExpression (subroutineScope,
          buildOpaqueVarRefExp (OpenCL::commandQueue, subroutineScope),
          variableDeclarations->getReference (OpenCL::kernelPointer),
          variableDeclarations->getReference (OpenCL::totalThreadNumber),
          variableDeclarations->getReference (OpenCL::threadsPerBlock),
          variableDeclarations->getReference (OpenCL::event)));

  appendStatement (assignmentStatement5, block);

  /*
   * ======================================================
   * Assert statement
   * ======================================================
   */

  SgEqualityOp * equalityExpression2 = buildEqualityOp (
      variableDeclarations->getReference (OpenCL::errorCode),
      buildOpaqueVarRefExp (OpenCL::CL_SUCCESS, subroutineScope));

  appendStatement (buildExprStatement (
      OpenCL::OP2RuntimeSupport::getAssertMessage (subroutineScope,
          equalityExpression2, buildStringVal (
              "Error setting OpenCL kernel arguments"))), block);

  /*
   * ======================================================
   * Complete device commands statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement6 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::errorCode),
      OpenCL::getFinishCommandQueueCallExpression (subroutineScope,
          buildOpaqueVarRefExp (OpenCL::commandQueue, subroutineScope)));

  appendStatement (assignmentStatement6, block);

  /*
   * ======================================================
   * Assert statement
   * ======================================================
   */

  SgEqualityOp * equalityExpression3 = buildEqualityOp (
      variableDeclarations->getReference (OpenCL::errorCode),
      buildOpaqueVarRefExp (OpenCL::CL_SUCCESS, subroutineScope));

  appendStatement (buildExprStatement (
      OpenCL::OP2RuntimeSupport::getAssertMessage (subroutineScope,
          equalityExpression3, buildStringVal (
              "Error setting OpenCL kernel arguments"))), block);

  return block;
}

void
CPPOpenCLHostSubroutineDirectLoop::createOpenCLKernelInitialisationStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace OP2::Macros;

  Debug::getInstance ()->debugMessage (
      "Creating OpenCL kernel initialisation statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Setting blocks per grid
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::blocksPerGrid), buildIntVal (
          200));

  appendStatement (assignmentStatement1, subroutineScope);

  /*
   * ======================================================
   * Setting threads per block
   * ======================================================
   */

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::threadsPerBlock),
      variableDeclarations->getReference (getBlockSizeVariableName (
          parallelLoop->getUserSubroutineName ())));

  appendStatement (assignmentStatement2, subroutineScope);

  /*
   * ======================================================
   * Setting total number of threads
   * ======================================================
   */

  SgMultiplyOp * multiplyExpression3 = buildMultiplyOp (
      variableDeclarations->getReference (OpenCL::threadsPerBlock),
      variableDeclarations->getReference (OpenCL::blocksPerGrid));

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::totalThreadNumber),
      multiplyExpression3);

  appendStatement (assignmentStatement3, subroutineScope);

  /*
   * ======================================================
   * Setting shared memory size
   * ======================================================
   */

  SgExprStatement * assignmentStatement4 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::sharedMemorySize),
      buildIntVal (0));

  appendStatement (assignmentStatement4, subroutineScope);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isDirect (i))
      {
        SgSizeOfOp * sizeOfExpression = buildSizeOfOp (
            parallelLoop->getOpDatBaseType (i));

        SgMultiplyOp * multiplyExpression = buildMultiplyOp (sizeOfExpression,
            buildIntVal (parallelLoop->getSizeOfOpDat (i)));

        SgFunctionCallExp * maxCallExpression =
            OP2::Macros::createMaxCallStatement (subroutineScope,
                variableDeclarations->getReference (OpenCL::sharedMemorySize),
                multiplyExpression);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            variableDeclarations->getReference (OpenCL::sharedMemorySize),
            maxCallExpression);

        appendStatement (assignmentStatement, subroutineScope);
      }
    }
  }

  SgMultiplyOp * multiplyExpression5 = buildMultiplyOp (
      variableDeclarations->getReference (OpenCL::sharedMemorySize),
      buildOpaqueVarRefExp (warpSizeMacro, subroutineScope));

  SgExprStatement * assignmentStatement5 = buildAssignStatement (
      variableDeclarations->getReference (sharedMemoryOffset),
      multiplyExpression5);

  appendStatement (assignmentStatement5, subroutineScope);

  SgMultiplyOp * multiplyExpression6 = buildMultiplyOp (
      variableDeclarations->getReference (OpenCL::sharedMemorySize),
      variableDeclarations->getReference (OpenCL::threadsPerBlock));

  SgExprStatement * assignmentStatement6 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::sharedMemorySize),
      multiplyExpression6);

  appendStatement (assignmentStatement6, subroutineScope);
}

void
CPPOpenCLHostSubroutineDirectLoop::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createOpenCLKernelInitialisationStatements ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }

  appendStatement (createKernelFunctionCallStatement (), subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }
}

void
CPPOpenCLHostSubroutineDirectLoop::createOpenCLKernelActualParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA configuration parameters", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (sharedMemoryOffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          sharedMemoryOffset, buildIntType (), subroutineScope));
}

void
CPPOpenCLHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  createOpenCLConfigurationLaunchDeclarations ();

  createOpenCLKernelActualParameterDeclarations ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }
}

CPPOpenCLHostSubroutineDirectLoop::CPPOpenCLHostSubroutineDirectLoop (
    SgScopeStatement * moduleScope,
    CPPOpenCLKernelSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop, CPPModuleDeclarations * moduleDeclarations) :
  CPPOpenCLHostSubroutine (moduleScope, calleeSubroutine, parallelLoop,
      moduleDeclarations)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of direct loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
