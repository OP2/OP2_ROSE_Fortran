#include <CPPCUDAKernelSubroutineDirectLoop.h>
#include <CPPParallelLoop.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CUDA.h>
#include <CommonNamespaces.h>

SgStatement *
CPPCUDAKernelSubroutineDirectLoop::createUserSubroutineCallStatement ()
{
}

void
CPPCUDAKernelSubroutineDirectLoop::createExecutionLoopStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildPlusAssignOp;
  using SageBuilder::buildPlusPlusOp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildForStatement;
  using SageInterface::appendStatement;

  SgBasicBlock * loopBody = buildBasicBlock ();

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (CUDA::getBlockId (
      BLOCK_X, subroutineScope), CUDA::getThreadBlockDimension (THREAD_X,
      subroutineScope));

  SgAddOp * addExpression1 = buildAddOp (CUDA::getThreadId (THREAD_X,
      subroutineScope), multiplyExpression1);

  SgAssignOp * initialisationExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      addExpression1);

  SgLessThanOp * upperBoundExpression = buildLessThanOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      buildVarRefExp (variableDeclarations->get (OP2::VariableNames::setSize)));

  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
      CUDA::getThreadBlockDimension (THREAD_X, subroutineScope),
      CUDA::getGridDimension (BLOCK_X, subroutineScope));

  SgPlusAssignOp * strideExpression = buildPlusAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      multiplyExpression2);

  SgForStatement * forStatement = buildForStatement (buildExprStatement (
      initialisationExpression), buildExprStatement (upperBoundExpression),
      strideExpression, loopBody);

  appendStatement (forStatement, subroutineScope);
}

SgForStatement *
CPPCUDAKernelSubroutineDirectLoop::createStageInFromDeviceMemoryToSharedMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
}

SgForStatement *
CPPCUDAKernelSubroutineDirectLoop::createStageInFromSharedMemoryToLocalMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
}

SgForStatement *
CPPCUDAKernelSubroutineDirectLoop::createStageOutFromSharedMemoryToDeviceMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
}

SgForStatement *
CPPCUDAKernelSubroutineDirectLoop::createStageOutFromLocalMemoryToSharedMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
}

void
CPPCUDAKernelSubroutineDirectLoop::createInitialiseOffsetIntoCUDASharedVariableStatements ()
{
  using SageBuilder::buildAddOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;
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
      if (parallelLoop->isGlobal (i) == false)
      {
        string const autosharedVariableName =
            OP2::VariableNames::getCUDASharedMemoryDeclarationName (
                parallelLoop->getOpDatBaseType (i),
                parallelLoop->getSizeOfOpDat (i));

        string const autosharedOffsetVariableName =
            OP2::VariableNames::getCUDASharedMemoryOffsetDeclarationName (
                parallelLoop->getOpDatBaseType (i),
                parallelLoop->getSizeOfOpDat (i));

        if (find (autosharedOffsetNames.begin (), autosharedOffsetNames.end (),
            autosharedOffsetVariableName) == autosharedOffsetNames.end ())
        {
          autosharedOffsetNames.push_back (autosharedOffsetVariableName);

          SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::sharedMemoryOffset)), buildVarRefExp (
                  variableDeclarations->get (OP2::VariableNames::threadID)));

          SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
              variableDeclarations->get (autosharedOffsetVariableName)),
              multiplyExpression1);

          SgExprStatement * assignmentStatement = buildAssignStatement (
              buildVarRefExp (
                  variableDeclarations->get (autosharedVariableName)),
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
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildModOp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating thread ID initialisation statement", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgModOp * modulusExpression = buildModOp (CUDA::getThreadId (THREAD_X,
      subroutineScope), buildOpaqueVarRefExp (
      OP2::VariableNames::warpSizeMacro, subroutineScope));

  SgExprStatement * assignmentStatement = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::threadID)),
      modulusExpression);

  appendStatement (assignmentStatement, subroutineScope);
}

void
CPPCUDAKernelSubroutineDirectLoop::createStatements ()
{
  createThreadIDInitialisationStatement ();

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
  using SageBuilder::buildIntType;
  using std::vector;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createCUDAStageInVariablesVariableDeclarations ();

  createCUDASharedVariableDeclarations ();

  vector <string> fourByteIntegers;

  fourByteIntegers.push_back (CommonVariableNames::iterationCounter1);
  fourByteIntegers.push_back (CommonVariableNames::iterationCounter2);
  fourByteIntegers.push_back (OP2::VariableNames::localOffset);
  fourByteIntegers.push_back (OP2::VariableNames::nelems);
  fourByteIntegers.push_back (OP2::VariableNames::threadID);

  for (vector <string>::iterator it = fourByteIntegers.begin (); it
      != fourByteIntegers.end (); ++it)
  {
    variableDeclarations->add (*it,
        RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            buildIntType (), subroutineScope));
  }
}

void
CPPCUDAKernelSubroutineDirectLoop::createOpDatFormalParameterDeclarations ()
{
  using SageBuilder::buildPointerType;
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
        string const & variableName =
            OP2::VariableNames::getReductionArrayDeviceName (i);
      }
      else if (parallelLoop->isDirect (i))
      {
        string const & variableName = OP2::VariableNames::getOpDatName (i);

        variableDeclarations->add (
            variableName,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, buildPointerType (parallelLoop->getOpDatBaseType (
                    i)), subroutineScope, formalParameters));
      }
      else if (parallelLoop->isRead (i))
      {
        string const & variableName = OP2::VariableNames::getOpDatName (i);

        if (parallelLoop->isGlobalScalar (i))
        {
        }
        else
        {
        }
      }
    }
  }
}

void
CPPCUDAKernelSubroutineDirectLoop::createFormalParameterDeclarations ()
{
  using SageBuilder::buildIntType;

  createOpDatFormalParameterDeclarations ();

  /*
   * ======================================================
   * Offset into shared memory
   * ======================================================
   */

  variableDeclarations->add (
      OP2::VariableNames::sharedMemoryOffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::sharedMemoryOffset, buildIntType (),
          subroutineScope, formalParameters));

  /*
   * ======================================================
   * OP_SET size
   * ======================================================
   */

  variableDeclarations->add (
      OP2::VariableNames::setSize,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::setSize, buildIntType (), subroutineScope,
          formalParameters));
}

CPPCUDAKernelSubroutineDirectLoop::CPPCUDAKernelSubroutineDirectLoop (
    SgScopeStatement * moduleScope, CPPCUDAUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop) :
  CPPCUDAKernelSubroutine (moduleScope, userSubroutine, parallelLoop)
{
  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
