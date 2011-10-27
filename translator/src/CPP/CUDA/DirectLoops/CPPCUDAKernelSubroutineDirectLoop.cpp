#include <CPPCUDAKernelSubroutineDirectLoop.h>
#include <CPPParallelLoop.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <CUDA.h>
#include <CommonNamespaces.h>

SgStatement *
CPPCUDAKernelSubroutineDirectLoop::createUserSubroutineCallStatement ()
{
  using namespace SageBuilder;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Creating call to user device subroutine", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * parameterExpression;

    if (parallelLoop->isGlobal (i))
    {
      Debug::getInstance ()->debugMessage ("OP_GBL",
          Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);
    }
    else if (parallelLoop->isDirect (i))
    {
      Debug::getInstance ()->debugMessage ("Direct OP_DAT",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      if (parallelLoop->getOpDatDimension (i) == 1)
      {
      }
      else
      {
        actualParameters ->append_expression (
            variableDeclarations->getReference (
                OP2::VariableNames::getOpDatLocalName (i)));
      }
    }
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
  using std::string;

  string const autosharedVariableName =
      OP2::VariableNames::getCUDASharedMemoryDeclarationName (
          parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
          parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  string const autosharedOffsetVariableName =
      OP2::VariableNames::getCUDASharedMemoryOffsetDeclarationName (
          parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
          parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter2),
      variableDeclarations->getReference (OP2::VariableNames::nelems));

  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
      variableDeclarations->getReference (OP2::VariableNames::localOffset),
      buildIntVal (parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
      OP2::VariableNames::threadID), multiplyExpression1);

  SgAddOp * addExpression2 = buildAddOp (addExpression1, multiplyExpression2);

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (OP2::VariableNames::getOpDatName (
          OP_DAT_ArgumentGroup)), addExpression2);

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (autosharedVariableName),
      addExpression1);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression2, arrayExpression1);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement1);

  SgAssignOp * initialisationExpression = buildAssignOp (
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter2), buildIntVal (0));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter2), buildIntVal (
          parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgForStatement * loopStatement = buildForStatement (buildExprStatement (
      initialisationExpression), buildExprStatement (upperBoundExpression),
      buildPlusPlusOp (variableDeclarations->getReference (
          CommonVariableNames::iterationCounter2)), loopBody);

  return loopStatement;
}

SgForStatement *
CPPCUDAKernelSubroutineDirectLoop::createStageInFromSharedMemoryToLocalMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using std::string;

  string const autosharedVariableName =
      OP2::VariableNames::getCUDASharedMemoryDeclarationName (
          parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
          parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  string const autosharedOffsetVariableName =
      OP2::VariableNames::getCUDASharedMemoryOffsetDeclarationName (
          parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
          parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
      variableDeclarations->getReference (OP2::VariableNames::threadID),
      buildIntVal (parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
      CommonVariableNames::iterationCounter2), multiplyExpression1);

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (autosharedVariableName),
      addExpression1);

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (
          OP2::VariableNames::getOpDatLocalName (OP_DAT_ArgumentGroup)),
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter2));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression2, arrayExpression1);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement1);

  SgAssignOp * initialisationExpression = buildAssignOp (
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter2), buildIntVal (0));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter2), buildIntVal (
          parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgForStatement * loopStatement = buildForStatement (buildExprStatement (
      initialisationExpression), buildExprStatement (upperBoundExpression),
      buildPlusPlusOp (variableDeclarations->getReference (
          CommonVariableNames::iterationCounter2)), loopBody);

  return loopStatement;
}

SgForStatement *
CPPCUDAKernelSubroutineDirectLoop::createStageOutFromSharedMemoryToDeviceMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using std::string;

  string const autosharedVariableName =
      OP2::VariableNames::getCUDASharedMemoryDeclarationName (
          parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
          parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  string const autosharedOffsetVariableName =
      OP2::VariableNames::getCUDASharedMemoryOffsetDeclarationName (
          parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
          parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter2),
      variableDeclarations->getReference (OP2::VariableNames::nelems));

  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
      variableDeclarations->getReference (OP2::VariableNames::localOffset),
      buildIntVal (parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
      OP2::VariableNames::threadID), multiplyExpression1);

  SgAddOp * addExpression2 = buildAddOp (addExpression1, multiplyExpression2);

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (OP2::VariableNames::getOpDatName (
          OP_DAT_ArgumentGroup)), addExpression2);

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (autosharedVariableName),
      addExpression1);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression1, arrayExpression2);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement1);

  SgAssignOp * initialisationExpression = buildAssignOp (
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter2), buildIntVal (0));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter2), buildIntVal (
          parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgForStatement * loopStatement = buildForStatement (buildExprStatement (
      initialisationExpression), buildExprStatement (upperBoundExpression),
      buildPlusPlusOp (variableDeclarations->getReference (
          CommonVariableNames::iterationCounter2)), loopBody);

  return loopStatement;
}

SgForStatement *
CPPCUDAKernelSubroutineDirectLoop::createStageOutFromLocalMemoryToSharedMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using std::string;

  string const autosharedVariableName =
      OP2::VariableNames::getCUDASharedMemoryDeclarationName (
          parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
          parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  string const autosharedOffsetVariableName =
      OP2::VariableNames::getCUDASharedMemoryOffsetDeclarationName (
          parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
          parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
      variableDeclarations->getReference (OP2::VariableNames::threadID),
      buildIntVal (parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
      CommonVariableNames::iterationCounter2), multiplyExpression1);

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (autosharedVariableName),
      addExpression1);

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (
          OP2::VariableNames::getOpDatLocalName (OP_DAT_ArgumentGroup)),
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter2));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression1, arrayExpression2);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement1);

  SgAssignOp * initialisationExpression = buildAssignOp (
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter2), buildIntVal (0));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter2), buildIntVal (
          parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgForStatement * loopStatement = buildForStatement (buildExprStatement (
      initialisationExpression), buildExprStatement (upperBoundExpression),
      buildPlusPlusOp (variableDeclarations->getReference (
          CommonVariableNames::iterationCounter2)), loopBody);

  return loopStatement;
}

void
CPPCUDAKernelSubroutineDirectLoop::createExecutionLoopStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating execution loop statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * loopBody = buildBasicBlock ();

  SgSubtractOp * subtractExpression1 = buildSubtractOp (
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter1),
      variableDeclarations->getReference (OP2::VariableNames::threadID));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OP2::VariableNames::localOffset),
      subtractExpression1);

  appendStatement (assignmentStatement1, loopBody);

  SgSubtractOp * subtractExpression2 = buildSubtractOp (
      variableDeclarations->getReference (OP2::VariableNames::setSize),
      variableDeclarations->getReference (OP2::VariableNames::localOffset));

  SgExprListExp * actualParameters = buildExprListExp (buildOpaqueVarRefExp (
      OP2::VariableNames::warpSizeMacro, subroutineScope), subtractExpression2);

  SgFunctionCallExp * functionCall = buildFunctionCallExp ("MIN",
      buildIntType (), actualParameters, subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (OP2::VariableNames::nelems),
      functionCall);

  appendStatement (assignmentStatement2, loopBody);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isGlobal (i) == false && parallelLoop->isWritten (i)
        == false && parallelLoop->getOpDatDimension (i) != 1)
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
        == false && parallelLoop->getOpDatDimension (i) != 1)
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
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter1), addExpression1);

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter1),
      variableDeclarations->getReference (OP2::VariableNames::setSize));

  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
      CUDA::getThreadBlockDimension (THREAD_X, subroutineScope),
      CUDA::getGridDimension (BLOCK_X, subroutineScope));

  SgPlusAssignOp * strideExpression = buildPlusAssignOp (
      variableDeclarations->getReference (
          CommonVariableNames::iterationCounter1), multiplyExpression2);

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

          SgMultiplyOp * multiplyExpression1 =
              buildMultiplyOp (variableDeclarations->getReference (
                  OP2::VariableNames::sharedMemoryOffset),
                  variableDeclarations->getReference (
                      OP2::VariableNames::threadID));

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

  Debug::getInstance ()->debugMessage (
      "Creating thread ID initialisation statement", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgModOp * modulusExpression = buildModOp (CUDA::getThreadId (THREAD_X,
      subroutineScope), buildOpaqueVarRefExp (
      OP2::VariableNames::warpSizeMacro, subroutineScope));

  SgExprStatement * assignmentStatement = buildAssignStatement (
      variableDeclarations->getReference (OP2::VariableNames::threadID),
      modulusExpression);

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
  using namespace SageBuilder;
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
      }
    }
  }
}

void
CPPCUDAKernelSubroutineDirectLoop::createFormalParameterDeclarations ()
{
  using namespace SageBuilder;

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
    CPPParallelLoop * parallelLoop,
    CPPCUDAReductionSubroutines * reductionSubroutines) :
  CPPCUDAKernelSubroutine (moduleScope, userSubroutine, parallelLoop,
      reductionSubroutines)
{
  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
