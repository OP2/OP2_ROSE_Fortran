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

  appendStatement (assignmentStatement1, subroutineScope);

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

          appendStatement (assignmentStatement, subroutineScope);
        }
        else
        {
          SgBitOrOp * orExpression = buildBitOrOp (
              variableDeclarations->getReference (OpenCL::errorCode),
              kernelArgumentExpression);

          SgExprStatement * assignmentStatement = buildAssignStatement (
              variableDeclarations->getReference (OpenCL::errorCode),
              orExpression);

          appendStatement (assignmentStatement, subroutineScope);

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

  appendStatement (assignmentStatement2, subroutineScope);

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

  appendStatement (assignmentStatement3, subroutineScope);

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

  appendStatement (assignmentStatement4, subroutineScope);
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

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::blocksPerGrid), buildIntVal (
          200));

  appendStatement (assignmentStatement1, subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::threadsPerBlock),
      variableDeclarations->getReference (getBlockSizeVariableName (
          parallelLoop->getUserSubroutineName ())));

  appendStatement (assignmentStatement2, subroutineScope);

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::sharedMemorySize),
      buildIntVal (0));

  appendStatement (assignmentStatement3, subroutineScope);

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

  SgMultiplyOp * multiplyExpression4 = buildMultiplyOp (
      variableDeclarations->getReference (OpenCL::sharedMemorySize),
      buildOpaqueVarRefExp (warpSizeMacro, subroutineScope));

  SgExprStatement * assignmentStatement4 = buildAssignStatement (
      variableDeclarations->getReference (sharedMemoryOffset),
      multiplyExpression4);

  appendStatement (assignmentStatement4, subroutineScope);

  SgMultiplyOp * multiplyExpression5 = buildMultiplyOp (
      variableDeclarations->getReference (OpenCL::sharedMemorySize),
      variableDeclarations->getReference (OpenCL::threadsPerBlock));

  SgExprStatement * assignmentStatement5 = buildAssignStatement (
      variableDeclarations->getReference (OpenCL::sharedMemorySize),
      multiplyExpression5);

  appendStatement (assignmentStatement5, subroutineScope);
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

  createKernelFunctionCallStatement ();

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
