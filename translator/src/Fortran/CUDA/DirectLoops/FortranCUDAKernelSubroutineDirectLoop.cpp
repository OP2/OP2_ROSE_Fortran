#include <FortranCUDAKernelSubroutineDirectLoop.h>
#include <FortranCUDAUserSubroutine.h>
#include <FortranCUDAModuleDeclarations.h>
#include <FortranCUDAOpDatCardinalitiesDeclaration.h>
#include <FortranReductionSubroutines.h>
#include <FortranParallelLoop.h>
#include <FortranOpDatDimensionsDeclaration.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <Debug.h>
#include <CommonNamespaces.h>
#include <CUDA.h>

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
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
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
      Debug::getInstance ()->debugMessage ("Direct argument",
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
    else if (parallelLoop->isReductionRequired (i))
    {
      Debug::getInstance ()->debugMessage ("Reduction argument",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      parameterExpression = buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::getOpDatLocalName (i)));
    }
    else if (parallelLoop->isGlobal (i))
    {
      Debug::getInstance ()->debugMessage ("Read argument",
          Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

      if (parallelLoop->isArray (i) == false)
      {
        Debug::getInstance ()->debugMessage (
            "OP_GBL with read access (Scalar)", Debug::HIGHEST_DEBUG_LEVEL,
            __FILE__, __LINE__);

        parameterExpression = buildVarRefExp (variableDeclarations->get (
            OP2::VariableNames::getOpDatName (i)));
      }
      else
      {
        Debug::getInstance ()->debugMessage ("OP_GBL with read access (Array)",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        string const variableName =
            OP2::VariableNames::getOpDatCardinalityName (i);

        SgDotExp * dotExpression = buildDotExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::opDatCardinalities)), buildOpaqueVarRefExp (
                variableName, subroutineScope));

        SgSubtractOp * subtractExpression = buildSubtractOp (dotExpression,
            buildIntVal (1));

        SgSubscriptExpression * subscriptExpression =
            new SgSubscriptExpression (RoseHelper::getFileInfo (), buildIntVal (
                0), subtractExpression, buildIntVal (1));

        subscriptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

        parameterExpression = buildPntrArrRefExp (buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::getOpDatName (i))),
            subscriptExpression);
      }
    }
  }

  return buildFunctionCallStmt (userSubroutine->getSubroutineName (),
      buildVoidType (), actualParameters, subroutineScope);
}

SgFortranDo *
FortranCUDAKernelSubroutineDirectLoop::createStageInFromDeviceMemoryToSharedMemoryStatements (
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
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprStatement;
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

  SgAssignOp * initialisationExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildIntVal (0));

  SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::opDatDimensions)),
      buildVarRefExp (dimensionsDeclaration->getOpDatDimensionField (
          OP_DAT_ArgumentGroup)));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildVarRefExp (variableDeclarations->get (OP2::VariableNames::nelems)));

  SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::threadID)),
      multiplyExpression1);

  SgAddOp * addExpression2 =
      buildAddOp (buildVarRefExp (variableDeclarations->get (
          autosharedOffsetVariableName)), addExpression1);

  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildVarRefExp (variableDeclarations->get (OP2::VariableNames::nelems)));

  SgMultiplyOp * multiplyExpression3 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::localOffset)),
      dotExpression);

  SgAddOp * addExpression3 = buildAddOp (multiplyExpression2,
      multiplyExpression3);

  SgAddOp * addExpression4 =
      buildAddOp (buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::threadID)), addExpression3);

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), addExpression2);

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::getOpDatName (
          OP_DAT_ArgumentGroup))), addExpression4);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression1, arrayExpression2);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement1);

  SgSubtractOp * upperBoundExpression = buildSubtractOp (dotExpression,
      buildIntVal (1));

  SgFortranDo * loopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initialisationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);

  return loopStatement;
}

SgFortranDo *
FortranCUDAKernelSubroutineDirectLoop::createStageInFromSharedMemoryToLocalMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprStatement;
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

  SgAssignOp * initialisationExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildIntVal (0));

  SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::opDatDimensions)),
      buildVarRefExp (dimensionsDeclaration->getOpDatDimensionField (
          OP_DAT_ArgumentGroup)));

  SgMultiplyOp * multiplyExpression4 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::threadID)), dotExpression);

  SgAddOp * addExpression5 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      multiplyExpression4);

  SgAddOp * addExpression6 =
      buildAddOp (buildVarRefExp (variableDeclarations->get (
          autosharedOffsetVariableName)), addExpression5);

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::getOpDatLocalName (
          OP_DAT_ArgumentGroup))), buildVarRefExp (variableDeclarations->get (
      CommonVariableNames::iterationCounter2)));

  SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), addExpression6);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      arrayExpression3, arrayExpression4);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement2);

  SgSubtractOp * upperBoundExpression = buildSubtractOp (dotExpression,
      buildIntVal (1));

  SgFortranDo * loopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initialisationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);

  return loopStatement;
}

SgFortranDo *
FortranCUDAKernelSubroutineDirectLoop::createStageOutFromSharedMemoryToDeviceMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
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

  SgExpression * loopInitializationExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildIntVal (0));

  SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::opDatDimensions)),
      buildVarRefExp (dimensionsDeclaration->getOpDatDimensionField (
          OP_DAT_ArgumentGroup)));

  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildVarRefExp (variableDeclarations->get (OP2::VariableNames::nelems)));

  SgMultiplyOp * multiplyExpression3 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::localOffset)),
      dotExpression);

  SgAddOp * addExpression3 = buildAddOp (multiplyExpression2,
      multiplyExpression3);

  SgAddOp * addExpression4 =
      buildAddOp (buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::threadID)), addExpression3);

  SgMultiplyOp * multiplyExpression4 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildVarRefExp (variableDeclarations->get (OP2::VariableNames::nelems)));

  SgAddOp * addExpression5 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::threadID)),
      multiplyExpression4);

  SgAddOp * addExpression6 =
      buildAddOp (buildVarRefExp (variableDeclarations->get (
          autosharedOffsetVariableName)), addExpression5);

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::getOpDatName (
          OP_DAT_ArgumentGroup))), addExpression4);

  SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), addExpression6);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      arrayExpression3, arrayExpression4);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement2);

  SgExpression * upperBoundExpression = buildSubtractOp (dotExpression,
      buildIntVal (1));

  SgFortranDo * loopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          loopInitializationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);

  return loopStatement;
}

SgFortranDo *
FortranCUDAKernelSubroutineDirectLoop::createStageOutFromLocalMemoryToSharedMemoryStatements (
    unsigned int OP_DAT_ArgumentGroup)
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

  SgExpression * loopInitializationExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildIntVal (0));

  SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::opDatDimensions)),
      buildVarRefExp (dimensionsDeclaration->getOpDatDimensionField (
          OP_DAT_ArgumentGroup)));

  SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::threadID)), dotExpression);

  SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      multiplyExpression1);

  SgAddOp * addExpression2 =
      buildAddOp (buildVarRefExp (variableDeclarations->get (
          autosharedOffsetVariableName)), addExpression1);

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::getOpDatLocalName (
          OP_DAT_ArgumentGroup))), buildVarRefExp (variableDeclarations->get (
      CommonVariableNames::iterationCounter2)));

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (autosharedVariableName)), addExpression2);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayExpression2, arrayExpression1);

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement1);

  SgExpression * upperBoundExpression = buildSubtractOp (dotExpression,
      buildIntVal (1));

  SgFortranDo * loopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          loopInitializationExpression, upperBoundExpression, buildIntVal (1),
          loopBody);

  return loopStatement;
}

void
FortranCUDAKernelSubroutineDirectLoop::createExecutionLoopStatements ()
{
  using boost::lexical_cast;
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

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("min", subroutineScope);

  SgSubtractOp * subtractExpression2 = buildSubtractOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::setSize)), buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::localOffset)));

  SgExprListExp * actualParameters = buildExprListExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::warpSize)),
      subtractExpression2);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (OP2::VariableNames::nelems)),
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
FortranCUDAKernelSubroutineDirectLoop::createInitialiseOffsetIntoCUDASharedVariableStatements ()
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildIntVal;
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
        string const autosharedOffsetVariableName =
            OP2::VariableNames::getCUDASharedMemoryOffsetDeclarationName (
                parallelLoop->getOpDatBaseType (i),
                parallelLoop->getSizeOfOpDat (i));

        if (find (autosharedOffsetNames.begin (), autosharedOffsetNames.end (),
            autosharedOffsetVariableName) == autosharedOffsetNames.end ())
        {
          autosharedOffsetNames.push_back (autosharedOffsetVariableName);

          SgSubtractOp * subtractExpression = buildSubtractOp (
              CUDA::getThreadId (THREAD_X, subroutineScope), buildIntVal (1));

          SgDivideOp * divideExpression1 = buildDivideOp (subtractExpression,
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::warpSize)));

          SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::sharedMemoryOffset)), divideExpression1);

          SgDivideOp * divideExpression2 = buildDivideOp (multiplyExpression1,
              buildIntVal (parallelLoop->getSizeOfOpDat (i)));

          SgExprStatement * assignmentStatement = buildAssignStatement (
              buildVarRefExp (variableDeclarations->get (
                  autosharedOffsetVariableName)), divideExpression2);

          appendStatement (assignmentStatement, subroutineScope);
        }
      }
    }
  }
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
FortranCUDAKernelSubroutineDirectLoop::createStatements ()
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
FortranCUDAKernelSubroutineDirectLoop::createLocalVariableDeclarations ()
{
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
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope));
  }
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
      if (parallelLoop->isReductionRequired (i))
      {
        string const & variableName =
            OP2::VariableNames::getReductionArrayDeviceName (i);

        variableDeclarations->add (
            variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, FortranTypesBuilder::getArray_RankOne (
                    parallelLoop->getOpDatBaseType (i)), subroutineScope,
                formalParameters, 1, CUDA_DEVICE));
      }
      else if (parallelLoop->isDirect (i))
      {
        string const & variableName = OP2::VariableNames::getOpDatName (i);

        SgDotExp * dotExpression = buildDotExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::opDatCardinalities)), buildVarRefExp (
                cardinalitiesDeclaration->getFieldDeclarations ()->get (
                    OP2::VariableNames::getOpDatCardinalityName (i))));

        SgSubtractOp * upperBoundExpression = buildSubtractOp (dotExpression,
            buildIntVal (1));

        variableDeclarations->add (
            variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName,
                FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                    parallelLoop->getOpDatBaseType (i), buildIntVal (0),
                    upperBoundExpression), subroutineScope, formalParameters,
                1, CUDA_DEVICE));
      }
      else if (parallelLoop->isRead (i))
      {
        string const & variableName = OP2::VariableNames::getOpDatName (i);

        if (parallelLoop->isArray (i))
        {
          variableDeclarations->add (
              variableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                  variableName,
                  FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                      parallelLoop->getOpDatBaseType (i), buildIntVal (0),
                      buildIntVal (parallelLoop->getOpDatDimension (i) - 1)),
                  subroutineScope, formalParameters, 1, CUDA_DEVICE));
        }
        else
        {
          variableDeclarations->add (
              variableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                  variableName, parallelLoop->getOpDatBaseType (i),
                  subroutineScope, formalParameters, 1, VALUE));

        }
      }
    }
  }
}

void
FortranCUDAKernelSubroutineDirectLoop::createFormalParameterDeclarations ()
{
  /*
   * ======================================================
   * OP_DAT dimensions
   * ======================================================
   */

  variableDeclarations->add (
      OP2::VariableNames::opDatDimensions,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::opDatDimensions,
          dimensionsDeclaration->getType (), subroutineScope, formalParameters,
          1, CUDA_DEVICE));

  /*
   * ======================================================
   * OP_DAT cardinalities
   * ======================================================
   */

  variableDeclarations->add (
      OP2::VariableNames::opDatCardinalities,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::opDatCardinalities,
          cardinalitiesDeclaration->getType (), subroutineScope,
          formalParameters, 1, CUDA_DEVICE));

  createOpDatFormalParameterDeclarations ();

  /*
   * ======================================================
   * OP_SET size
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
   * Warp size
   * ======================================================
   */

  variableDeclarations->add (
      OP2::VariableNames::warpSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::warpSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));

  /*
   * ======================================================
   * Offset into shared memory
   * ======================================================
   */

  variableDeclarations->add (
      OP2::VariableNames::sharedMemoryOffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::sharedMemoryOffset,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));
}

FortranCUDAKernelSubroutineDirectLoop::FortranCUDAKernelSubroutineDirectLoop (
    SgScopeStatement * moduleScope, FortranCUDAUserSubroutine * userSubroutine,
    FortranParallelLoop * parallelLoop,
    FortranReductionSubroutines * reductionSubroutines,
    FortranCUDAOpDatCardinalitiesDeclaration * cardinalitiesDeclaration,
    FortranOpDatDimensionsDeclaration * dimensionsDeclaration,
    FortranCUDAModuleDeclarations * moduleDeclarations) :
  FortranCUDAKernelSubroutine (moduleScope, userSubroutine, parallelLoop,
      reductionSubroutines, cardinalitiesDeclaration, dimensionsDeclaration,
      moduleDeclarations)
{
  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
