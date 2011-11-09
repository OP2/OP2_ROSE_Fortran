#include <CPPCUDAHostSubroutineIndirectLoop.h>
#include <CPPModuleDeclarations.h>
#include <RoseHelper.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CUDA.h>
#include <CommonNamespaces.h>
#include <PlanFunction.h>

SgStatement *
CPPCUDAHostSubroutineIndirectLoop::createKernelFunctionCallStatement ()
{
  using namespace SageBuilder;
  using namespace OP2::VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statement to call CUDA kernel", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (getOpDatName (i)),
          buildOpaqueVarRefExp (dataOnDevice, subroutineScope));

      SgCastExp * castExpression = buildCastExp (dotExpression,
          buildPointerType (parallelLoop->getOpDatBaseType (i)));

      actualParameters->append_expression (castExpression);
    }
  }

  unsigned int arrayIndex = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
            buildOpaqueVarRefExp (PlanFunction::ind_maps), buildIntVal (
                arrayIndex));

        SgArrowExp * arrowExpression = buildArrowExp (
            variableDeclarations->getReference (PlanFunction::planRet),
            arrayExpression);

        actualParameters->append_expression (arrowExpression);

        arrayIndex++;
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
          buildOpaqueVarRefExp (PlanFunction::loc_maps), buildIntVal (i - 1));

      SgArrowExp * arrowExpression = buildArrowExp (
          variableDeclarations->getReference (PlanFunction::planRet),
          arrayExpression);

      actualParameters->append_expression (arrowExpression);
    }
  }

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (PlanFunction::planRet),
      buildOpaqueVarRefExp (PlanFunction::ind_sizes, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (PlanFunction::planRet),
      buildOpaqueVarRefExp (PlanFunction::ind_offs, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (PlanFunction::planRet),
      buildOpaqueVarRefExp (PlanFunction::blkmap, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (PlanFunction::planRet),
      buildOpaqueVarRefExp (PlanFunction::offset, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (PlanFunction::planRet),
      buildOpaqueVarRefExp (PlanFunction::nelems, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (PlanFunction::planRet),
      buildOpaqueVarRefExp (PlanFunction::thrcol, subroutineScope)));

  actualParameters->append_expression (buildArrowExp (
      variableDeclarations->getReference (PlanFunction::planRet),
      buildOpaqueVarRefExp (PlanFunction::nthrcol, subroutineScope)));

  actualParameters->append_expression (variableDeclarations->getReference (
      PlanFunction::blockOffset));

  SgCudaKernelExecConfig * kernelConfiguration = new SgCudaKernelExecConfig (
      RoseHelper::getFileInfo (), variableDeclarations->getReference (
          CUDA::blocksPerGrid), variableDeclarations->getReference (
          CUDA::threadsPerBlock), variableDeclarations->getReference (
          CUDA::sharedMemorySize));

  kernelConfiguration->set_endOfConstruct (RoseHelper::getFileInfo ());

  SgCudaKernelCallExp * kernelCallExpression = new SgCudaKernelCallExp (
      RoseHelper::getFileInfo (), buildFunctionRefExp (
          calleeSubroutine->getSubroutineName (), subroutineScope),
      actualParameters, kernelConfiguration);

  kernelCallExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

  return buildExprStatement (kernelCallExpression);
}

SgBasicBlock *
CPPCUDAHostSubroutineIndirectLoop::createPlanFunctionExecutionStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2::VariableNames;
  using namespace CommonVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating plan function execution statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (PlanFunction::blockOffset),
      buildIntVal (0));

  appendStatement (assignmentStatement1, block);

  /*
   * ======================================================
   * For loop body
   * ======================================================
   */

  SgBasicBlock * loopBody = buildBasicBlock ();

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (PlanFunction::ncolblk, subroutineScope),
      variableDeclarations->getReference (getIterationCounterVariableName (3)));

  SgArrowExp * arrowExpression2 = buildArrowExp (
      variableDeclarations->getReference (PlanFunction::planRet),
      arrayExpression2);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::blocksPerGrid),
      arrowExpression2);

  appendStatement (assignmentStatement2, loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgArrowExp * arrowExpression3 = buildArrowExp (
      variableDeclarations->getReference (PlanFunction::planRet),
      buildOpaqueVarRefExp (PlanFunction::nshared, subroutineScope));

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::sharedMemorySize),
      arrowExpression3);

  appendStatement (assignmentStatement3, loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement4 = buildAssignStatement (
      variableDeclarations->getReference (CUDA::threadsPerBlock),
      moduleDeclarations->getBlockSizeReference (
          parallelLoop->getUserSubroutineName ()));

  appendStatement (assignmentStatement4, loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  appendStatement (createKernelFunctionCallStatement (), loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgFunctionCallExp
      * threadSynchronizeExpression =
          CUDA::CPPRuntimeSupport::getCUDASafeHostThreadSynchronisationCallStatement (
              subroutineScope);

  appendStatement (buildExprStatement (threadSynchronizeExpression), loopBody);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement5 = buildExprStatement (
      buildPlusAssignOp (variableDeclarations->getReference (
          PlanFunction::blockOffset), variableDeclarations->getReference (
          CUDA::blocksPerGrid)));

  appendStatement (assignmentStatement5, loopBody);

  /*
   * ======================================================
   * For loop header
   * ======================================================
   */

  SgExprStatement * initialisationExpression = buildAssignStatement (
      variableDeclarations->getReference (getIterationCounterVariableName (3)),
      buildIntVal (0));

  SgArrowExp * arrowExpression = buildArrowExp (
      variableDeclarations->getReference (PlanFunction::planRet),
      buildOpaqueVarRefExp (PlanFunction::ncolors, subroutineScope));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (3)),
      buildOpaqueVarRefExp (numberOfColours, subroutineScope));

  SgPlusPlusOp * strideExpression = buildPlusPlusOp (
      variableDeclarations->getReference (getIterationCounterVariableName (3)));

  SgForStatement * forLoopStatement = buildForStatement (
      initialisationExpression, buildExprStatement (upperBoundExpression),
      strideExpression, loopBody);

  appendStatement (forLoopStatement, block);

  return block;
}

SgStatement *
CPPCUDAHostSubroutineIndirectLoop::createPlanFunctionCallStatement ()
{
  using namespace SageBuilder;
  using namespace OP2::VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statement to call plan function", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParamaters = buildExprListExp ();

  actualParamaters->append_expression (variableDeclarations->getReference (
      getUserSubroutineName ()));

  actualParamaters->append_expression (variableDeclarations->getReference (
      getOpSetName ()));

  actualParamaters->append_expression (
      moduleDeclarations->getPartitionSizeReference (
          parallelLoop->getUserSubroutineName ()));

  actualParamaters->append_expression (buildIntVal (
      parallelLoop->getNumberOfOpDatArgumentGroups ()));

  actualParamaters->append_expression (variableDeclarations->getReference (
      PlanFunction::args));

  actualParamaters->append_expression (buildIntVal (
      parallelLoop->getNumberOfDistinctIndirectOpDatArguments ()));

  actualParamaters->append_expression (variableDeclarations->getReference (
      PlanFunction::inds));

  SgFunctionCallExp * functionCallExpression = buildFunctionCallExp (
      OP2::OP_PLAN_GET, buildVoidType (), actualParamaters, subroutineScope);

  SgExprStatement * assignmentStatement = buildAssignStatement (
      variableDeclarations->getReference (PlanFunction::planRet),
      functionCallExpression);

  return assignmentStatement;
}

void
CPPCUDAHostSubroutineIndirectLoop::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }

  appendStatementList (
      createInitialisePlanFunctionArrayStatements ()->getStatementList (),
      subroutineScope);

  appendStatementList (
      createPlanFunctionExecutionStatements ()->getStatementList (),
      subroutineScope);

  appendStatement (createPlanFunctionCallStatement (), subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }
}

void
CPPCUDAHostSubroutineIndirectLoop::createPlanFunctionDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2::VariableNames;
  using namespace CommonVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations for plan function",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (
      getIterationCounterVariableName (3),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (3), buildIntType (), subroutineScope));

  variableDeclarations->add (PlanFunction::args,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::args, buildArrayType (buildClassDeclaration (
              OP2::OP_ARG, subroutineScope)->get_type (), buildIntVal (
              parallelLoop->getNumberOfOpDatArgumentGroups ())),
          subroutineScope));

  variableDeclarations->add (PlanFunction::inds,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::inds, buildArrayType (buildIntType (), buildIntVal (
              parallelLoop->getNumberOfOpDatArgumentGroups ())),
          subroutineScope));

  variableDeclarations->add (PlanFunction::planRet,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::planRet, buildPointerType (buildClassDeclaration (
              OP2::OP_PLAN, subroutineScope)->get_type ()), subroutineScope));

  variableDeclarations->add (PlanFunction::blockOffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::blockOffset, buildIntType (), subroutineScope));
}

void
CPPCUDAHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  createCUDAConfigurationLaunchDeclarations ();

  createPlanFunctionDeclarations ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }
}

CPPCUDAHostSubroutineIndirectLoop::CPPCUDAHostSubroutineIndirectLoop (
    SgScopeStatement * moduleScope, CPPCUDAKernelSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop, CPPModuleDeclarations * moduleDeclarations) :
  CPPCUDAHostSubroutine (moduleScope, calleeSubroutine, parallelLoop,
      moduleDeclarations)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of indirect loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
