#include <FortranCUDAKernelSubroutineDirectLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <Debug.h>
#include <CUDA.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgStatement *
FortranCUDAKernelSubroutineDirectLoop::createUserSubroutineCallStatement ()
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
      actualParameters->append_expression (
          buildOpGlobalActualParameterExpression (i));
    }
    else if (parallelLoop->isDirect (i))
    {
      Debug::getInstance ()->debugMessage ("Direct OP_DAT",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      if (parallelLoop->getOpDatDimension (i) == 1)
      {
        SgAddOp * addExpression = buildAddOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), buildIntVal (
                parallelLoop->getOpDatDimension (i) - 1));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (RoseHelper::getFileInfo (),
                buildVarRefExp (variableDeclarations->get (
                    CommonVariableNames::iterationCounter1)), addExpression,
                buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            RoseHelper::getFileInfo ());

        SgPntrArrRefExp * parameterExpression = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getOpDatName (i))),
            arraySubscriptExpression);

        actualParameters->append_expression (parameterExpression);
      }
      else
      {
        actualParameters->append_expression (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getOpDatLocalName (i))));
      }
    }
  }

  return buildFunctionCallStmt (userSubroutineName, buildVoidType (),
      actualParameters, subroutineScope);
}

SgBasicBlock *
FortranCUDAKernelSubroutineDirectLoop::createStageInFromDeviceMemoryToLocalThreadVariablesStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildNullExpression;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating stage in statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * outerBlock = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isGlobal (i) == false && parallelLoop->isWritten (i)
        == false && parallelLoop->getOpDatDimension (i) != 1)
    {
      /*
       * ======================================================
       * Builds stage in from device memory to shared memory
       * ======================================================
       */

      string const autosharedVariableName =
          OP2::VariableNames::getAutosharedDeclarationName (
              parallelLoop->getOpDatBaseType (i), parallelLoop->getSizeOfOpDat (
                  i));

      SgAssignOp * initialisationExpression = buildAssignOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter2)),
          buildIntVal (0));

      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getDimensionsVariableDeclarationName (
                  userSubroutineName))), buildVarRefExp (
          opDatDimensionsDeclaration->getOpDatDimensionField (i)));

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter2)),
          buildVarRefExp (
              variableDeclarations->get (OP2::VariableNames::nelems)));

      SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::threadID)),
          multiplyExpression1);

      SgAddOp * addExpression2 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::offset)),
          addExpression1);

      SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter2)),
          buildVarRefExp (
              variableDeclarations->get (OP2::VariableNames::nelems)));

      SgMultiplyOp * multiplyExpression3 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::offset)),
          dotExpression);

      SgAddOp * addExpression3 = buildAddOp (multiplyExpression2,
          multiplyExpression3);

      SgAddOp * addExpression4 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::threadID)),
          addExpression3);

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (autosharedVariableName)), addExpression2);

      SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::getOpDatName (i))),
          addExpression4);

      SgExprStatement * assignmentStatement1 = buildAssignStatement (
          arrayExpression1, arrayExpression2);

      SgBasicBlock * firstLoopBody = buildBasicBlock (assignmentStatement1);

      SgFortranDo * firstLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              initialisationExpression, dotExpression, buildIntVal (1),
              firstLoopBody);

      appendStatement (firstLoopStatement, outerBlock);

      /*
       * ======================================================
       * Builds stage in from shared memory to local thread
       * variables
       * ======================================================
       */

      SgMultiplyOp * multiplyExpression4 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::threadID)),
          dotExpression);

      SgAddOp * addExpression5 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter2)),
          multiplyExpression4);

      SgAddOp * addExpression6 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::offset)),
          addExpression5);

      SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getOpDatLocalName (i))),
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter2)));

      SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (autosharedVariableName)), addExpression6);

      SgExprStatement * assignmentStatement2 = buildAssignStatement (
          arrayExpression3, arrayExpression4);

      SgBasicBlock * secondLoopBody = buildBasicBlock (assignmentStatement2);

      SgFortranDo * secondLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              initialisationExpression, dotExpression, buildIntVal (1),
              secondLoopBody);

      appendStatement (secondLoopStatement, outerBlock);
    }
  }

  return outerBlock;
}

SgBasicBlock *
FortranCUDAKernelSubroutineDirectLoop::createStageOutFromLocalThreadVariablesToDeviceMemoryStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildNullExpression;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating stage out statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * outerBlock = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isGlobal (i) == false && parallelLoop->isRead (i)
        == false && parallelLoop->getOpDatDimension (i) != 1)
    {
      string const autosharedVariableName =
          OP2::VariableNames::getAutosharedDeclarationName (
              parallelLoop->getOpDatBaseType (i), parallelLoop->getSizeOfOpDat (
                  i));

      SgExpression * loopInitializationExpression = buildAssignOp (
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter2)), buildIntVal (0));

      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getDimensionsVariableDeclarationName (
                  userSubroutineName))), buildVarRefExp (
          opDatDimensionsDeclaration->getOpDatDimensionField (i)));

      SgExpression * upperBoundExpression = buildSubtractOp (dotExpression,
          buildIntVal (1));

      /*
       * ======================================================
       * Builds stage out from local thread variables to
       * shared memory
       * ======================================================
       */

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::threadID)),
          dotExpression);

      SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter2)),
          multiplyExpression1);

      SgAddOp * addExpression2 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::offset)),
          addExpression1);

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getOpDatLocalName (i))),
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter2)));

      SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (autosharedVariableName)), addExpression2);

      SgExprStatement * assignmentStatement1 = buildAssignStatement (
          arrayExpression2, arrayExpression1);

      SgBasicBlock * loopBody1 = buildBasicBlock (assignmentStatement1);

      SgFortranDo * loopStatement1 =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              loopInitializationExpression, upperBoundExpression, buildIntVal (
                  1), loopBody1);

      appendStatement (loopStatement1, outerBlock);

      /*
       * ======================================================
       * Builds stage out from shared memory to device
       * variables
       * ======================================================
       */

      SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter2)),
          buildVarRefExp (
              variableDeclarations->get (OP2::VariableNames::nelems)));

      SgMultiplyOp * multiplyExpression3 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::offset)),
          dotExpression);

      SgAddOp * addExpression3 = buildAddOp (multiplyExpression2,
          multiplyExpression3);

      SgAddOp * addExpression4 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::threadID)),
          addExpression3);

      SgMultiplyOp * multiplyExpression4 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter2)),
          buildVarRefExp (
              variableDeclarations->get (OP2::VariableNames::nelems)));

      SgAddOp * addExpression5 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::threadID)),
          multiplyExpression4);

      SgAddOp * addExpression6 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::offset)),
          addExpression5);

      SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::getOpDatName (i))),
          addExpression4);

      SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (autosharedVariableName)), addExpression6);

      SgExprStatement * assignmentStatement2 = buildAssignStatement (
          arrayExpression3, arrayExpression4);

      SgBasicBlock * loopBody2 = buildBasicBlock (assignmentStatement2);

      SgFortranDo * loopStatement2 =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              loopInitializationExpression, upperBoundExpression, buildIntVal (
                  1), loopBody2);

      appendStatement (loopStatement2, outerBlock);
    }
  }

  return outerBlock;
}

void
FortranCUDAKernelSubroutineDirectLoop::createExecutionLoopStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprStatement;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Creating execution loop statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * loopBody = buildBasicBlock ();

  SgSubtractOp * subtractExpression1 =
      buildSubtractOp (buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter1)), buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::threadID)));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (OP2::VariableNames::offset)),
      subtractExpression1);

  appendStatement (assignmentStatement1, loopBody);

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("min", subroutineScope);

  SgSubtractOp * subtractExpression2 = buildSubtractOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::setSize)), buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::offset)));

  SgExprListExp * actualParameters = buildExprListExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::warpSize)),
      subtractExpression2);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (OP2::VariableNames::nelems)),
      functionCall);

  appendStatement (assignmentStatement2, loopBody);

  appendStatement (
      createStageInFromDeviceMemoryToLocalThreadVariablesStatements (),
      loopBody);

  appendStatement (createUserSubroutineCallStatement (), loopBody);

  appendStatement (
      createStageOutFromLocalThreadVariablesToDeviceMemoryStatements (),
      loopBody);

  SgSubtractOp * subtractExpression3 = buildSubtractOp (CUDA::getBlockId (
      BLOCK_X, subroutineScope), buildIntVal (1));

  SgMultiplyOp * multiplyExpression = buildMultiplyOp (subtractExpression3,
      CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

  SgSubtractOp * subtractExpression4 = buildSubtractOp (CUDA::getThreadId (
      THREAD_X, subroutineScope), buildIntVal (1));

  SgAddOp * addExpression =
      buildAddOp (subtractExpression4, multiplyExpression);

  SgExpression * initialisationExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      addExpression);

  SgMultiplyOp * strideExpression = buildMultiplyOp (
      CUDA::getThreadBlockDimension (THREAD_X, subroutineScope),
      CUDA::getGridDimension (BLOCK_X, subroutineScope));

  SgExpression * upperBoundExpression =
      buildSubtractOp (buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::setSize)), buildIntVal (1));

  SgFortranDo * fortranDoStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initialisationExpression, upperBoundExpression, strideExpression,
          loopBody);

  appendStatement (fortranDoStatement, subroutineScope);
}

void
FortranCUDAKernelSubroutineDirectLoop::createAutoSharedDisplacementInitialisationStatement ()
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating autoshared displacement initialisation statement",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgSubtractOp * subtractExpression = buildSubtractOp (CUDA::getThreadId (
      THREAD_X, subroutineScope), buildIntVal (1));

  SgDivideOp * divideExpression1 =
      buildDivideOp (subtractExpression, buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::warpSize)));

  SgDivideOp * divideExpression2 = buildDivideOp (divideExpression1,
      buildIntVal (parallelLoop->getMaximumSizeOfOpDat ()));

  if (parallelLoop->getMaximumSizeOfOpDat () == 0)
  {
    Debug::getInstance ()->errorMessage (
        "Generating code which will produce divide-by-zero error. The maximum size of all OP_DATs is 0.",
        __FILE__, __LINE__);
  }

  SgExprStatement * assignmentStatement = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::offset)),
      divideExpression2);

  appendStatement (assignmentStatement, subroutineScope);
}

void
FortranCUDAKernelSubroutineDirectLoop::createThreadIDInitialisationStatement ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating thread ID initialisation statement", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgSubtractOp * subtractExpression = buildSubtractOp (CUDA::getThreadId (
      THREAD_X, subroutineScope), buildIntVal (1));

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("mod", subroutineScope);

  SgExprListExp * actualParameters =
      buildExprListExp (subtractExpression, buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::warpSize)));

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  SgExprStatement * assignmentStatement = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::threadID)), functionCall);

  appendStatement (assignmentStatement, subroutineScope);
}

void
FortranCUDAKernelSubroutineDirectLoop::createOpDatFormalParameterDeclarations ()
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildArrayType;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildDotExp;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating OP_DAT formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i) == false)
      {
        string const & variableName = OP2::VariableNames::getOpDatName (i);

        if (parallelLoop->isGlobal (i) && parallelLoop->isRead (i))
        {
          if (parallelLoop->isGlobalScalar (i))
          {
            variableDeclarations->add (
                variableName,
                FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                    variableName, parallelLoop->getOpDatBaseType (i),
                    subroutineScope, formalParameters, 1, VALUE));
          }
          else
          {
            variableDeclarations->add (
                variableName,
                FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                    variableName,
                    FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                        parallelLoop->getOpDatBaseType (i), buildIntVal (0),
                        buildIntVal (parallelLoop->getOpDatDimension (i) - 1)),
                    subroutineScope, formalParameters, 1, DEVICE));
          }
        }
        else
        {
          SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::getDataSizesVariableDeclarationName (
                      userSubroutineName))), buildVarRefExp (
              dataSizesDeclaration->getFieldDeclarations ()->get (
                  OP2::VariableNames::getOpDatSizeName (i))));

          SgSubtractOp * upperBoundExpression = buildSubtractOp (dotExpression,
              buildIntVal (1));

          variableDeclarations->add (
              variableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                  variableName,
                  FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                      parallelLoop->getOpDatBaseType (i), buildIntVal (0),
                      upperBoundExpression), subroutineScope, formalParameters,
                  1, DEVICE));
        }
      }
    }
  }
}

void
FortranCUDAKernelSubroutineDirectLoop::createStatements ()
{
  createThreadIDInitialisationStatement ();

  createAutoSharedDisplacementInitialisationStatement ();

  createInitialiseLocalThreadVariablesStatements ();

  createExecutionLoopStatements ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionEpilogueStatements ();
  }
}

void
FortranCUDAKernelSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  using std::vector;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createLocalThreadDeclarations ();

  createAutoSharedDeclarations ();

  vector <string> fourByteIntegers;

  fourByteIntegers.push_back (CommonVariableNames::iterationCounter1);

  fourByteIntegers.push_back (CommonVariableNames::iterationCounter2);

  fourByteIntegers.push_back (OP2::VariableNames::threadID);

  fourByteIntegers.push_back (OP2::VariableNames::offset);

  fourByteIntegers.push_back (OP2::VariableNames::offset2);

  fourByteIntegers.push_back (OP2::VariableNames::nelems);

  for (vector <string>::iterator it = fourByteIntegers.begin (); it
      != fourByteIntegers.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope));
  }

  if (parallelLoop->isReductionRequired () == true)
  {
    variableDeclarations->add (OP2::VariableNames::offset,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            OP2::VariableNames::offset,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope, 1,
            VALUE));
  }
}

void
FortranCUDAKernelSubroutineDirectLoop::createFormalParameterDeclarations ()
{
  /*
   * ======================================================
   * OP_DAT dimensions formal parameter
   * ======================================================
   */

  variableDeclarations->add (
      OP2::VariableNames::getDimensionsVariableDeclarationName (
          userSubroutineName),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::getDimensionsVariableDeclarationName (
              userSubroutineName), opDatDimensionsDeclaration->getType (),
          subroutineScope, formalParameters, 1, DEVICE));

  /*
   * ======================================================
   * OP_DAT sizes formal parameter
   * ======================================================
   */

  variableDeclarations->add (
      OP2::VariableNames::getDataSizesVariableDeclarationName (
          userSubroutineName),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::getDataSizesVariableDeclarationName (
              userSubroutineName), dataSizesDeclaration->getType (),
          subroutineScope, formalParameters, 1, DEVICE));

  /*
   * ======================================================
   * OP_DAT formal parameters
   * ======================================================
   */

  createOpDatFormalParameterDeclarations ();

  /*
   * ======================================================
   * Set size formal parameter
   * ======================================================
   */

  variableDeclarations->add (
      OP2::VariableNames::setSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::setSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));

  /*
   * ======================================================
   * Warp size formal parameter
   * ======================================================
   */

  variableDeclarations->add (
      OP2::VariableNames::warpSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::warpSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

FortranCUDAKernelSubroutineDirectLoop::FortranCUDAKernelSubroutineDirectLoop (
    std::string const & subroutineName, std::string const & userSubroutineName,
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    FortranReductionSubroutines * reductionSubroutines,
    FortranCUDADataSizesDeclaration * dataSizesDeclaration,
    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration,
    FortranCUDAModuleDeclarations * moduleDeclarations) :
  FortranCUDAKernelSubroutine (subroutineName, userSubroutineName,
      parallelLoop, moduleScope, reductionSubroutines, dataSizesDeclaration,
      opDatDimensionsDeclaration, moduleDeclarations)
{
  Debug::getInstance ()->debugMessage ("<Kernel, Direct, CUDA>",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
