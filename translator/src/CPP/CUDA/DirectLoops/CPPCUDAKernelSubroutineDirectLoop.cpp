#include <CPPCUDAKernelSubroutineDirectLoop.h>
#include <CPPParallelLoop.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <CUDA.h>
#include <CommonNamespaces.h>

SgStatement *
CPPCUDAKernelSubroutineDirectLoop::createUserSubroutineCallStatement ()
{
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildVarRefExp;
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
        actualParameters->append_expression (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getOpDatLocalName (i))));
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
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildPlusPlusOp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildForStatement;
  using SageInterface::appendStatement;
  using std::string;

  string const autosharedVariableName =
      OP2::VariableNames::getCUDASharedMemoryDeclarationName (
          parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
          parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  string const autosharedOffsetVariableName =
      OP2::VariableNames::getCUDASharedMemoryOffsetDeclarationName (
          parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
          parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildVarRefExp (variableDeclarations->get (OP2::VariableNames::nelems)));

  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::localOffset)),
      buildIntVal (parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::threadID)),
      multiplyExpression1);

  SgAddOp * addExpression2 = buildAddOp (addExpression1, multiplyExpression2);

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::getOpDatName (
          OP_DAT_ArgumentGroup))), addExpression2);

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), addExpression1);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression2, arrayExpression1);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement1);

  SgAssignOp * initialisationExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildIntVal (0));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildIntVal (parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgForStatement * loopStatement = buildForStatement (buildExprStatement (
      initialisationExpression), buildExprStatement (upperBoundExpression),
      buildPlusPlusOp (buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter2))), loopBody);

  return loopStatement;
}

SgForStatement *
CPPCUDAKernelSubroutineDirectLoop::createStageInFromSharedMemoryToLocalMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildPlusPlusOp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildForStatement;
  using SageInterface::appendStatement;
  using std::string;

  string const autosharedVariableName =
      OP2::VariableNames::getCUDASharedMemoryDeclarationName (
          parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
          parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  string const autosharedOffsetVariableName =
      OP2::VariableNames::getCUDASharedMemoryOffsetDeclarationName (
          parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
          parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::threadID)), buildIntVal (
      parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      multiplyExpression1);

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), addExpression1);

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::getOpDatLocalName (
          OP_DAT_ArgumentGroup))), buildVarRefExp (variableDeclarations->get (
      CommonVariableNames::iterationCounter2)));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression2, arrayExpression1);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement1);

  SgAssignOp * initialisationExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildIntVal (0));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildIntVal (parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgForStatement * loopStatement = buildForStatement (buildExprStatement (
      initialisationExpression), buildExprStatement (upperBoundExpression),
      buildPlusPlusOp (buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter2))), loopBody);

  return loopStatement;
}

SgForStatement *
CPPCUDAKernelSubroutineDirectLoop::createStageOutFromSharedMemoryToDeviceMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildPlusPlusOp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildForStatement;
  using SageInterface::appendStatement;
  using std::string;

  string const autosharedVariableName =
      OP2::VariableNames::getCUDASharedMemoryDeclarationName (
          parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
          parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  string const autosharedOffsetVariableName =
      OP2::VariableNames::getCUDASharedMemoryOffsetDeclarationName (
          parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
          parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildVarRefExp (variableDeclarations->get (OP2::VariableNames::nelems)));

  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::localOffset)),
      buildIntVal (parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::threadID)),
      multiplyExpression1);

  SgAddOp * addExpression2 = buildAddOp (addExpression1, multiplyExpression2);

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::getOpDatName (
          OP_DAT_ArgumentGroup))), addExpression2);

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), addExpression1);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression1, arrayExpression2);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement1);

  SgAssignOp * initialisationExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildIntVal (0));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildIntVal (parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgForStatement * loopStatement = buildForStatement (buildExprStatement (
      initialisationExpression), buildExprStatement (upperBoundExpression),
      buildPlusPlusOp (buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter2))), loopBody);

  return loopStatement;
}

SgForStatement *
CPPCUDAKernelSubroutineDirectLoop::createStageOutFromLocalMemoryToSharedMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildPlusPlusOp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildForStatement;
  using SageInterface::appendStatement;
  using std::string;

  string const autosharedVariableName =
      OP2::VariableNames::getCUDASharedMemoryDeclarationName (
          parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
          parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  string const autosharedOffsetVariableName =
      OP2::VariableNames::getCUDASharedMemoryOffsetDeclarationName (
          parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup),
          parallelLoop->getSizeOfOpDat (OP_DAT_ArgumentGroup));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::threadID)), buildIntVal (
      parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      multiplyExpression1);

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), addExpression1);

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::getOpDatLocalName (
          OP_DAT_ArgumentGroup))), buildVarRefExp (variableDeclarations->get (
      CommonVariableNames::iterationCounter2)));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression1, arrayExpression2);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement1);

  SgAssignOp * initialisationExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildIntVal (0));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildIntVal (parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

  SgForStatement * loopStatement = buildForStatement (buildExprStatement (
      initialisationExpression), buildExprStatement (upperBoundExpression),
      buildPlusPlusOp (buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter2))), loopBody);

  return loopStatement;
}

void
CPPCUDAKernelSubroutineDirectLoop::createExecutionLoopStatements ()
{
  using boost::lexical_cast;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildPlusAssignOp;
  using SageBuilder::buildPlusPlusOp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildForStatement;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildIntType;
  using SageBuilder::buildFunctionCallExp;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating execution loop statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * loopBody = buildBasicBlock ();

  SgSubtractOp * subtractExpression1 =
      buildSubtractOp (buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter1)), buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::threadID)));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::localOffset)), subtractExpression1);

  appendStatement (assignmentStatement1, loopBody);

  SgSubtractOp * subtractExpression2 = buildSubtractOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::setSize)), buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::localOffset)));

  SgExprListExp * actualParameters = buildExprListExp (buildOpaqueVarRefExp (
      OP2::VariableNames::warpSizeMacro, subroutineScope), subtractExpression2);

  SgFunctionCallExp * functionCall = buildFunctionCallExp ("MIN",
      buildIntType (), actualParameters, subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (OP2::VariableNames::nelems)),
      functionCall);

  appendStatement (assignmentStatement2, loopBody);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isGlobal (i) == false && parallelLoop->isRead (i)
        && parallelLoop->getOpDatDimension (i) != 1)
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
    if (parallelLoop->isGlobal (i) == false && parallelLoop->isWritten (i)
        && parallelLoop->getOpDatDimension (i) != 1)
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
