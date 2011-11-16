#include <CPPCUDAKernelSubroutineDirectLoop.h>
#include <CPPParallelLoop.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <CUDA.h>
#include <CompilerGeneratedNames.h>
#include <OP2Definitions.h>

SgStatement *
CPPCUDAKernelSubroutineDirectLoop::createUserSubroutineCallStatement ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Creating call to user device subroutine", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * parameterExpression;

    if (parallelLoop->isDirect (i))
    {
      Debug::getInstance ()->debugMessage ("Direct OP_DAT",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      if (parallelLoop->getOpDatDimension (i) == 1)
      {
        parameterExpression = buildAddOp (variableDeclarations->getReference (
            getOpDatName (i)), variableDeclarations->getReference (
            getIterationCounterVariableName (1)));
      }
      else
      {
        parameterExpression = variableDeclarations->getReference (
            getOpDatLocalName (i));
      }
    }
    else if (parallelLoop->isReductionRequired (i))
    {
      Debug::getInstance ()->debugMessage ("Reduction argument",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      parameterExpression = variableDeclarations->getReference (
          getOpDatLocalName (i));
    }
    else
    {
      Debug::getInstance ()->debugMessage ("Global with read access",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      if (parallelLoop->isPointer (i) || parallelLoop->isArray (i))
      {
        parameterExpression = variableDeclarations->getReference (
            getOpDatLocalName (i));
      }
      else
      {
        parameterExpression = variableDeclarations->getReference (getOpDatName (
            i));
      }
    }

    ROSE_ASSERT (parameterExpression != NULL);

    actualParameters->append_expression (parameterExpression);
  }

  return buildFunctionCallStmt (userSubroutine->getSubroutineName (),
      buildVoidType (), actualParameters, subroutineScope);
}

SgForStatement *
CPPCUDAKernelSubroutineDirectLoop::createStageInFromDeviceMemoryToSharedMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;

  string const autosharedVariableName = getCUDASharedMemoryDeclarationName (
      parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
      parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      variableDeclarations->getReference (nelems));

  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
      variableDeclarations->getReference (localOffset), buildIntVal (
          parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
      threadID), multiplyExpression1);

  SgAddOp * addExpression2 = buildAddOp (addExpression1, multiplyExpression2);

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (getOpDatName (OP_DAT_ArgumentGroup)),
      addExpression2);

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (autosharedVariableName),
      addExpression1);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression2, arrayExpression1);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement1);

  SgAssignOp * initialisationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      buildIntVal (0));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      buildIntVal (parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgForStatement * loopStatement = buildForStatement (buildExprStatement (
      initialisationExpression), buildExprStatement (upperBoundExpression),
      buildPlusPlusOp (variableDeclarations->getReference (
          getIterationCounterVariableName (2))), loopBody);

  return loopStatement;
}

SgForStatement *
CPPCUDAKernelSubroutineDirectLoop::createStageInFromSharedMemoryToLocalMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using std::string;

  string const autosharedVariableName = getCUDASharedMemoryDeclarationName (
      parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
      parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
      variableDeclarations->getReference (threadID), buildIntVal (
          parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
      getIterationCounterVariableName (2)), multiplyExpression1);

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (autosharedVariableName),
      addExpression1);

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (getOpDatLocalName (
          OP_DAT_ArgumentGroup)), variableDeclarations->getReference (
          getIterationCounterVariableName (2)));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression2, arrayExpression1);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement1);

  SgAssignOp * initialisationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      buildIntVal (0));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      buildIntVal (parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgForStatement * loopStatement = buildForStatement (buildExprStatement (
      initialisationExpression), buildExprStatement (upperBoundExpression),
      buildPlusPlusOp (variableDeclarations->getReference (
          getIterationCounterVariableName (2))), loopBody);

  return loopStatement;
}

SgForStatement *
CPPCUDAKernelSubroutineDirectLoop::createStageOutFromSharedMemoryToDeviceMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;

  string const autosharedVariableName = getCUDASharedMemoryDeclarationName (
      parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
      parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      variableDeclarations->getReference (nelems));

  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
      variableDeclarations->getReference (localOffset), buildIntVal (
          parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
      threadID), multiplyExpression1);

  SgAddOp * addExpression2 = buildAddOp (addExpression1, multiplyExpression2);

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (getOpDatName (OP_DAT_ArgumentGroup)),
      addExpression2);

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (autosharedVariableName),
      addExpression1);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression1, arrayExpression2);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement1);

  SgAssignOp * initialisationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      buildIntVal (0));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      buildIntVal (parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgForStatement * loopStatement = buildForStatement (buildExprStatement (
      initialisationExpression), buildExprStatement (upperBoundExpression),
      buildPlusPlusOp (variableDeclarations->getReference (
          getIterationCounterVariableName (2))), loopBody);

  return loopStatement;
}

SgForStatement *
CPPCUDAKernelSubroutineDirectLoop::createStageOutFromLocalMemoryToSharedMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using std::string;

  string const autosharedVariableName = getCUDASharedMemoryDeclarationName (
      parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
      parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
      variableDeclarations->getReference (threadID), buildIntVal (
          parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
      getIterationCounterVariableName (2)), multiplyExpression1);

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (autosharedVariableName),
      addExpression1);

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (getOpDatLocalName (
          OP_DAT_ArgumentGroup)), variableDeclarations->getReference (
          getIterationCounterVariableName (2)));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression1, arrayExpression2);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement1);

  SgAssignOp * initialisationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      buildIntVal (0));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      buildIntVal (parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgForStatement * loopStatement = buildForStatement (buildExprStatement (
      initialisationExpression), buildExprStatement (upperBoundExpression),
      buildPlusPlusOp (variableDeclarations->getReference (
          getIterationCounterVariableName (2))), loopBody);

  return loopStatement;
}

void
CPPCUDAKernelSubroutineDirectLoop::createExecutionLoopStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating execution loop statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * loopBody = buildBasicBlock ();

  SgSubtractOp * subtractExpression1 = buildSubtractOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      variableDeclarations->getReference (threadID));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (localOffset), subtractExpression1);

  appendStatement (assignmentStatement1, loopBody);

  SgSubtractOp * subtractExpression2 = buildSubtractOp (
      variableDeclarations->getReference (setSize),
      variableDeclarations->getReference (localOffset));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (nelems),
      OP2::Macros::createMinCallStatement (subroutineScope,
          buildOpaqueVarRefExp (OP2::Macros::warpSizeMacro, subroutineScope),
          subtractExpression2));

  appendStatement (assignmentStatement2, loopBody);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isGlobal (i) == false && parallelLoop->isWritten (i)
        == false && parallelLoop->getOpDatDimension (i) > 1)
    {
      Debug::getInstance ()->debugMessage (
          "Creating statements to stage in from device memory to shared memory for OP_DAT "
              + lexical_cast <string> (i), Debug::OUTER_LOOP_LEVEL, __FILE__,
          __LINE__);

      appendStatement (
          createStageInFromDeviceMemoryToSharedMemoryStatements (i), loopBody);

      Debug::getInstance ()->debugMessage (
          "Creating statements to stage in from shared memory to local memory for OP_DAT "
              + lexical_cast <string> (i), Debug::OUTER_LOOP_LEVEL, __FILE__,
          __LINE__);

      appendStatement (
          createStageInFromSharedMemoryToLocalMemoryStatements (i), loopBody);
    }
  }

  appendStatement (createUserSubroutineCallStatement (), loopBody);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isGlobal (i) == false && parallelLoop->isRead (i)
        == false && parallelLoop->getOpDatDimension (i) > 1)
    {
      Debug::getInstance ()->debugMessage (
          "Creating statements to stage out from local memory to shared memory for OP_DAT "
              + lexical_cast <string> (i), Debug::OUTER_LOOP_LEVEL, __FILE__,
          __LINE__);

      appendStatement (
          createStageOutFromLocalMemoryToSharedMemoryStatements (i), loopBody);

      Debug::getInstance ()->debugMessage (
          "Creating statements to stage out from shared memory to device memory for OP_DAT "
              + lexical_cast <string> (i), Debug::OUTER_LOOP_LEVEL, __FILE__,
          __LINE__);

      appendStatement (createStageOutFromSharedMemoryToDeviceMemoryStatements (
          i), loopBody);
    }
  }

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (CUDA::getBlockId (
      BLOCK_X, subroutineScope), CUDA::getThreadBlockDimension (THREAD_X,
      subroutineScope));

  SgAddOp * addExpression1 = buildAddOp (CUDA::getThreadId (THREAD_X,
      subroutineScope), multiplyExpression1);

  SgAssignOp * initialisationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      addExpression1);

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      variableDeclarations->getReference (setSize));

  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
      CUDA::getThreadBlockDimension (THREAD_X, subroutineScope),
      CUDA::getGridDimension (BLOCK_X, subroutineScope));

  SgPlusAssignOp * strideExpression = buildPlusAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      multiplyExpression2);

  SgForStatement * forStatement = buildForStatement (buildExprStatement (
      initialisationExpression), buildExprStatement (upperBoundExpression),
      strideExpression, loopBody);

  appendStatement (forStatement, subroutineScope);
}

void
CPPCUDAKernelSubroutineDirectLoop::createInitialiseOffsetIntoCUDASharedVariableStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using std::find;
  using std::vector;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating initialisation statements for offset in CUDA shared variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  vector <string> autosharedOffsetNames;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isDirect (i) && parallelLoop->getOpDatDimension (i) > 1)
      {
        string const autosharedVariableName =
            getCUDASharedMemoryDeclarationName (parallelLoop->getOpDatBaseType (
                i), parallelLoop->getSizeOfOpDat (i));

        string const autosharedOffsetVariableName =
            getCUDASharedMemoryOffsetDeclarationName (
                parallelLoop->getOpDatBaseType (i),
                parallelLoop->getSizeOfOpDat (i));

        if (find (autosharedOffsetNames.begin (), autosharedOffsetNames.end (),
            autosharedOffsetVariableName) == autosharedOffsetNames.end ())
        {
          autosharedOffsetNames.push_back (autosharedOffsetVariableName);

          SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
              variableDeclarations->getReference (sharedMemoryOffset),
              variableDeclarations->getReference (threadID));

          SgAddOp * addExpression1 =
              buildAddOp (variableDeclarations->getReference (
                  autosharedOffsetVariableName), multiplyExpression1);

          SgExprStatement * assignmentStatement = buildAssignStatement (
              variableDeclarations->getReference (autosharedVariableName),
              addExpression1);

          appendStatement (assignmentStatement, subroutineScope);
        }
      }
    }
  }
}

void
CPPCUDAKernelSubroutineDirectLoop::createThreadIDInitialisationStatement ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace OP2::Macros;

  Debug::getInstance ()->debugMessage (
      "Creating thread ID initialisation statement", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgModOp * modulusExpression = buildModOp (CUDA::getThreadId (THREAD_X,
      subroutineScope), buildOpaqueVarRefExp (warpSizeMacro, subroutineScope));

  SgExprStatement * assignmentStatement = buildAssignStatement (
      variableDeclarations->getReference (threadID), modulusExpression);

  appendStatement (assignmentStatement, subroutineScope);
}

void
CPPCUDAKernelSubroutineDirectLoop::createStatements ()
{
  createThreadIDInitialisationStatement ();

  createReductionPrologueStatements ();

  createInitialiseOffsetIntoCUDASharedVariableStatements ();

  createExecutionLoopStatements ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionEpilogueStatements ();
  }
}

void
CPPCUDAKernelSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::vector;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createCUDAStageInVariablesVariableDeclarations ();

  createCUDASharedVariableDeclarations ();

  variableDeclarations->add (
      getIterationCounterVariableName (1),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1), buildIntType (), subroutineScope));

  variableDeclarations->add (
      getIterationCounterVariableName (2),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (2), buildIntType (), subroutineScope));

  variableDeclarations->add (localOffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          localOffset, buildIntType (), subroutineScope));

  variableDeclarations->add (nelems,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (nelems,
          buildIntType (), subroutineScope));

  variableDeclarations->add (threadID,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (threadID,
          buildIntType (), subroutineScope));
}

void
CPPCUDAKernelSubroutineDirectLoop::createOpDatFormalParameterDeclarations ()
{
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace SageBuilder;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating OP_DAT formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i))
      {
        string const & variableName = getReductionArrayDeviceName (i);

        Debug::getInstance ()->debugMessage ("Reduction type",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        variableDeclarations->add (
            variableName,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, buildPointerType (parallelLoop->getOpDatBaseType (
                    i)), subroutineScope, formalParameters));

      }
      else if (parallelLoop->isDirect (i))
      {
        Debug::getInstance ()->debugMessage ("OP_DAT: direct",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        string const & variableName = getOpDatName (i);

        variableDeclarations->add (
            variableName,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, buildPointerType (parallelLoop->getOpDatBaseType (
                    i)), subroutineScope, formalParameters));
      }
      else if (parallelLoop->isRead (i))
      {
        Debug::getInstance ()->debugMessage ("Read",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        string const & variableName = getOpDatName (i);
      }
    }
  }
}

void
CPPCUDAKernelSubroutineDirectLoop::createFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;

  createOpDatFormalParameterDeclarations ();

  /*
   * ======================================================
   * Offset into shared memory
   * ======================================================
   */

  variableDeclarations->add (
      sharedMemoryOffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          sharedMemoryOffset, buildIntType (), subroutineScope,
          formalParameters));

  /*
   * ======================================================
   * OP_SET size
   * ======================================================
   */

  variableDeclarations->add (
      setSize,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          setSize, buildIntType (), subroutineScope, formalParameters));
}

CPPCUDAKernelSubroutineDirectLoop::CPPCUDAKernelSubroutineDirectLoop (
    SgScopeStatement * moduleScope, CPPCUDAUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop,
    CPPReductionSubroutines * reductionSubroutines) :
  CPPCUDAKernelSubroutine (moduleScope, userSubroutine, parallelLoop,
      reductionSubroutines)
{
  Debug::getInstance ()->debugMessage (
      "Creating kernel subroutine of direct loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
