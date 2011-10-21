#include <FortranCUDAKernelSubroutineIndirectLoop.h>
#include <FortranCUDAUserSubroutine.h>
#include <FortranParallelLoop.h>
#include <FortranOpDatDimensionsDeclaration.h>
#include <FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop.h>
#include <FortranTypesBuilder.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <RoseHelper.h>
#include <Plan.h>
#include <CUDA.h>
#include <Debug.h>
#include <boost/lexical_cast.hpp>

SgStatement *
FortranCUDAKernelSubroutineIndirectLoop::createUserSubroutineCallStatement ()
{
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildSubtractOp;
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

    if (parallelLoop->isIndirect (i))
    {
      if (parallelLoop->isIncremented (i))
      {
        Debug::getInstance ()->debugMessage (
            "Indirect OP_DAT with increment access", Debug::OUTER_LOOP_LEVEL,
            __FILE__, __LINE__);

        parameterExpression = buildVarRefExp (variableDeclarations->get (
            OP2::VariableNames::getOpDatLocalName (i)));
      }
      else if (parallelLoop->isRead (i))
      {
        Debug::getInstance ()->debugMessage (
            "Indirect OP_DAT with read access", Debug::OUTER_LOOP_LEVEL,
            __FILE__, __LINE__);

        string const autosharedVariableName =
            OP2::VariableNames::getCUDASharedMemoryDeclarationName (
                parallelLoop->getOpDatBaseType (i),
                parallelLoop->getSizeOfOpDat (i));

        SgAddOp * addExpression1 = buildAddOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), buildVarRefExp (
                variableDeclarations->get (
                    OP2::VariableNames::sharedMemoryOffset)));

        SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getGlobalToLocalMappingName (i))),
            addExpression1);

        SgDotExp * dotExpression1 = buildDotExp (buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::opDatDimensions)),
            buildVarRefExp (dimensionsDeclaration->getOpDatDimensionField (i)));

        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (arrayExpression1,
            dotExpression1);

        SgAddOp * addExpression2 = buildAddOp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getNumberOfBytesVariableName (i))),
            multiplyExpression1);

        parameterExpression
            = buildPntrArrRefExp (buildVarRefExp (variableDeclarations->get (
                autosharedVariableName)), addExpression2);
      }
      else
      {
        Debug::getInstance ()->debugMessage (
            "Indirect OP_DAT with read/write or write access",
            Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

        string const autosharedVariableName =
            OP2::VariableNames::getCUDASharedMemoryDeclarationName (
                parallelLoop->getOpDatBaseType (i),
                parallelLoop->getSizeOfOpDat (i));

        SgAddOp * addExpression1 = buildAddOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), buildVarRefExp (
                variableDeclarations->get (
                    OP2::VariableNames::sharedMemoryOffset)));

        SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getGlobalToLocalMappingName (i))),
            addExpression1);

        SgDotExp * dotExpression1 = buildDotExp (buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::opDatDimensions)),
            buildVarRefExp (dimensionsDeclaration->getOpDatDimensionField (i)));

        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (arrayExpression1,
            dotExpression1);

        SgAddOp * addExpression2 = buildAddOp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getNumberOfBytesVariableName (i))),
            multiplyExpression1);

        SgAddOp * addExpression3 = buildAddOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), buildVarRefExp (
                variableDeclarations->get (
                    OP2::VariableNames::sharedMemoryOffset)));

        SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getGlobalToLocalMappingName (i))),
            addExpression3);

        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (arrayExpression2,
            dotExpression1);

        SgAddOp * addExpression4 = buildAddOp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getNumberOfBytesVariableName (i))),
            multiplyExpression2);

        SgAddOp * addExpression5 = buildAddOp (addExpression4, dotExpression1);

        SgSubscriptExpression * subscriptExpression =
            new SgSubscriptExpression (RoseHelper::getFileInfo (),
                addExpression2, addExpression5, buildIntVal (1));

        subscriptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

        parameterExpression = buildPntrArrRefExp (buildVarRefExp (
            variableDeclarations->get (autosharedVariableName)),
            buildExprListExp (subscriptExpression));
      }
    }
    else if (parallelLoop->isDirect (i))
    {
      Debug::getInstance ()->debugMessage ("Direct OP_DAT",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter1)),
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::sharedMemoryOffset)));

      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::opDatDimensions)),
          buildVarRefExp (dimensionsDeclaration->getOpDatDimensionField (i)));

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (addExpression1,
          dotExpression);

      SgExpression * accessExpression = NULL;

      if (parallelLoop->getOpDatDimension (i) > 1)
      {
        SgAddOp * addExpression2 = buildAddOp (multiplyExpression,
            dotExpression);

        accessExpression = new SgSubscriptExpression (
            RoseHelper::getFileInfo (), multiplyExpression, addExpression2,
            buildIntVal (1));
        accessExpression->set_endOfConstruct (RoseHelper::getFileInfo ());
      }
      else
      {
        accessExpression = multiplyExpression;
      }

      parameterExpression = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::getOpDatName (i))),
          accessExpression);
    }
    else if (parallelLoop->isReductionRequired (i))
    {
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

        SgDotExp * dotExpression = buildDotExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::opDatCardinalities)), buildVarRefExp (
                cardinalitiesDeclaration->getFieldDeclarations ()->get (
                    OP2::VariableNames::getOpDatCardinalityName (i))));

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

    ROSE_ASSERT (parameterExpression != NULL);

    actualParameters->append_expression (parameterExpression);
  }

  return buildFunctionCallStmt (userSubroutine->getSubroutineName (),
      buildVoidType (), actualParameters, subroutineScope);
}

SgBasicBlock *
FortranCUDAKernelSubroutineIndirectLoop::createIncrementAndWriteAccessEpilogueStatements ()
{
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildWhileStmt;
  using SageInterface::appendStatement;
  using std::string;

  SgBasicBlock * block = buildBasicBlock ();

  unsigned int pindOffsOffset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        if (parallelLoop->isWritten (i) || parallelLoop->isReadAndWritten (i)
            || parallelLoop->isIncremented (i))
        {
          string const autosharedVariableName =
              OP2::VariableNames::getCUDASharedMemoryDeclarationName (
                  parallelLoop->getOpDatBaseType (i),
                  parallelLoop->getSizeOfOpDat (i));
          /*
           * ======================================================
           * Defining lower and upper bounds and increment
           * ======================================================
           */

          SgDotExp * dotExpression2 =
              buildDotExp (buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::opDatDimensions)), buildVarRefExp (
                  dimensionsDeclaration->getOpDatDimensionField (i)));

          SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::getIndirectionArgumentSizeName (i))),
              dotExpression2);

          SgLessThanOp
              * lessThanExpression1 = buildLessThanOp (buildVarRefExp (
                  variableDeclarations->get (
                      CommonVariableNames::iterationCounter1)),
                  multiplyExpression1);

          SgSubtractOp * subtractExpression1 = buildSubtractOp (
              CUDA::getThreadId (THREAD_X, subroutineScope), buildIntVal (1));

          SgExprStatement
              * assignmentStatement1 = buildAssignStatement (buildVarRefExp (
                  variableDeclarations->get (
                      CommonVariableNames::iterationCounter1)),
                  subtractExpression1);

          appendStatement (assignmentStatement1, block);

          /*
           * ======================================================
           * Defining modulo result assignment
           * ======================================================
           */

          SgFunctionSymbol * functionSymbol =
              FortranTypesBuilder::buildNewFortranFunction ("mod",
                  subroutineScope);

          SgExprListExp * actualParameters = buildExprListExp (
              buildVarRefExp (variableDeclarations->get (
                  CommonVariableNames::iterationCounter1)), dotExpression2);

          SgFunctionCallExp * functionCall = buildFunctionCallExp (
              functionSymbol, actualParameters);

          SgExprStatement * assignmentStatement2 = buildAssignStatement (
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::moduloResult)), functionCall);

          /*
           * ======================================================
           * Defining device variable accessing expressions
           * ======================================================
           */

          SgMultiplyOp * multiplyExpression2a = buildMultiplyOp (
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::blockID)), buildIntVal (
                  parallelLoop->getNumberOfDifferentIndirectOpDats ()));

          SgAddOp * addExpression2a = buildAddOp (buildIntVal (pindOffsOffset),
              multiplyExpression2a);

          SgPntrArrRefExp
              * arrayIndexExpression2a = buildPntrArrRefExp (buildVarRefExp (
                  variableDeclarations->get (
                      OP2::VariableNames::PlanFunction::pindOffs)),
                  addExpression2a);

          SgDivideOp * divideExpression2 = buildDivideOp (
              buildVarRefExp (variableDeclarations->get (
                  CommonVariableNames::iterationCounter1)), dotExpression2);

          SgAddOp * addExpression2b = buildAddOp (arrayIndexExpression2a,
              divideExpression2);

          SgAddOp * addExpression2c = buildAddOp (buildIntVal (0),
              addExpression2b);

          SgPntrArrRefExp * arrayIndexExpression2b = buildPntrArrRefExp (
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::getLocalToGlobalMappingName (i))),
              addExpression2c);

          SgMultiplyOp * multiplyExpression2b = buildMultiplyOp (
              arrayIndexExpression2b, dotExpression2);

          SgAddOp * addExpression2d = buildAddOp (buildVarRefExp (
              variableDeclarations->get (OP2::VariableNames::moduloResult)),
              multiplyExpression2b);

          SgPntrArrRefExp * arrayIndexExpression2c = buildPntrArrRefExp (
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::getOpDatName (i))), addExpression2d);

          /*
           * ======================================================
           * Defining shared variable accessing expressions
           * ======================================================
           */

          SgAddOp * addExpression2e = buildAddOp (buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::getNumberOfBytesVariableName (i))),
              buildVarRefExp (variableDeclarations->get (
                  CommonVariableNames::iterationCounter1)));

          SgPntrArrRefExp * arrayIndexExpression2d = buildPntrArrRefExp (
              buildVarRefExp (
                  variableDeclarations->get (autosharedVariableName)),
              addExpression2e);

          SgExprStatement * assignmentStatement3 = NULL;

          if (parallelLoop->isIncremented (i))
          {
            SgAddOp * addExpression2f = buildAddOp (arrayIndexExpression2c,
                arrayIndexExpression2d);

            assignmentStatement3 = buildAssignStatement (
                arrayIndexExpression2c, addExpression2f);
          }

          if (parallelLoop->isWritten (i) || parallelLoop->isReadAndWritten (i))
          {
            assignmentStatement3 = buildAssignStatement (
                arrayIndexExpression2c, arrayIndexExpression2d);
          }

          /*
           * ======================================================
           * Defining loop body
           * ======================================================
           */

          SgAddOp * addExpression2f = buildAddOp (
              buildVarRefExp (variableDeclarations->get (
                  CommonVariableNames::iterationCounter1)),
              CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

          SgExprStatement * assignmentStatement4 = buildAssignStatement (
              buildVarRefExp (variableDeclarations->get (
                  CommonVariableNames::iterationCounter1)), addExpression2f);

          SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement2,
              assignmentStatement3, assignmentStatement4);

          SgWhileStmt * loopStatement = buildWhileStmt (lessThanExpression1,
              loopBody);

          loopStatement->set_has_end_statement (true);

          appendStatement (loopStatement, block);

        }
        pindOffsOffset++;
      }
    }
  }

  return block;
}

SgBasicBlock *
FortranCUDAKernelSubroutineIndirectLoop::createStageOutFromLocalMemoryToSharedMemoryStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildEqualityOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildBasicBlock;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating inner loop statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  SgBasicBlock * ifBody = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      string const autosharedVariableName =
          OP2::VariableNames::getCUDASharedMemoryDeclarationName (
              parallelLoop->getOpDatBaseType (i), parallelLoop->getSizeOfOpDat (
                  i));

      SgBasicBlock * innerLoopBody = buildBasicBlock ();

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getOpDatLocalName (i))),
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter2)));

      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::opDatDimensions)),
          buildVarRefExp (dimensionsDeclaration->getOpDatDimensionField (i)));

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getIncrementAccessMapName (i))),
          dotExpression);

      SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter2)),
          multiplyExpression);

      SgAddOp * addExpression2 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getNumberOfBytesVariableName (i))),
          addExpression1);

      SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (autosharedVariableName)), addExpression2);

      SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (autosharedVariableName)), addExpression2);

      SgAddOp * addExpression3 =
          buildAddOp (arrayExpression2, arrayExpression1);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression3, addExpression3);

      appendStatement (assignmentStatement, innerLoopBody);

      SgAssignOp * innerLoopLowerBoundExpression = buildAssignOp (
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter2)), buildIntVal (0));

      SgSubtractOp * innerLoopUpperBoundExpression = buildSubtractOp (
          dotExpression, buildIntVal (1));

      SgFortranDo * innerLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              innerLoopLowerBoundExpression, innerLoopUpperBoundExpression,
              buildIntVal (1), innerLoopBody);

      appendStatement (innerLoopStatement, ifBody);
    }
  }

  SgEqualityOp * ifGuardExpression = buildEqualityOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::colour2)), buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::colour1)));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  SgBasicBlock * loopBody = buildBasicBlock ();

  appendStatement (ifStatement, loopBody);

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      loopBody);

  SgAssignOp * lowerBoundExpression =
      buildAssignOp (buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::colour1)), buildIntVal (0));

  SgSubtractOp * upperBoundExpression = buildSubtractOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::numberOfColours)),
      buildIntVal (1));

  SgFortranDo
      * loopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              lowerBoundExpression, upperBoundExpression, buildIntVal (1),
              loopBody);

  appendStatement (loopStatement, block);

  return block;
}

SgBasicBlock *
FortranCUDAKernelSubroutineIndirectLoop::createIncrementAdjustmentStatements ()
{
  using boost::lexical_cast;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAddOp;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to adjust incremented OP_DATS",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      Debug::getInstance ()->debugMessage ("Creating statements for OP_DAT "
          + lexical_cast <string> (i), Debug::HIGHEST_DEBUG_LEVEL, __FILE__,
          __LINE__);

      SgAddOp * addExpression = buildAddOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter1)),
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::sharedMemoryOffset)));

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getGlobalToLocalMappingName (i))),
          addExpression);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getIncrementAccessMapName (i))),
          arrayExpression);

      appendStatement (assignmentStatement, block);
    }
  }

  return block;
}

SgBasicBlock *
FortranCUDAKernelSubroutineIndirectLoop::createInitialiseIncrementAccessStatements ()
{
  using boost::lexical_cast;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise scratchpad memory",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      Debug::getInstance ()->debugMessage ("Creating statements for OP_DAT "
          + lexical_cast <string> (i), Debug::HIGHEST_DEBUG_LEVEL, __FILE__,
          __LINE__);

      SgBasicBlock * loopBody = buildBasicBlock ();

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getOpDatLocalName (i))),
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter2)));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression, buildIntVal (0));

      appendStatement (assignmentStatement, loopBody);

      SgAssignOp * lowerBoundExpression = buildAssignOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter2)),
          buildIntVal (0));

      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::opDatDimensions)),
          buildVarRefExp (dimensionsDeclaration->getOpDatDimensionField (i)));

      SgSubtractOp * upperBoundExpression = buildSubtractOp (dotExpression,
          buildIntVal (1));

      SgFortranDo * loopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              lowerBoundExpression, upperBoundExpression, buildIntVal (1),
              loopBody);

      appendStatement (loopStatement, block);
    }
  }

  return block;
}

void
FortranCUDAKernelSubroutineIndirectLoop::createExecutionLoopStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildWhileStmt;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating main execution loop statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * loopBody = buildBasicBlock ();

  if (parallelLoop->hasIncrementedOpDats ())
  {
    SgExprStatement * assignmentStatement1 =
        buildAssignStatement (buildVarRefExp (variableDeclarations->get (
            OP2::VariableNames::colour2)), buildIntVal (-1));

    appendStatement (assignmentStatement1, loopBody);

    SgBasicBlock * ifBody = buildBasicBlock ();

    appendStatement (createInitialiseIncrementAccessStatements (), ifBody);

    appendStatement (createUserSubroutineCallStatement (), ifBody);

    SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
        variableDeclarations->get (CommonVariableNames::iterationCounter1)),
        buildVarRefExp (variableDeclarations->get (
            OP2::VariableNames::sharedMemoryOffset)));

    SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
        variableDeclarations->get (OP2::VariableNames::PlanFunction::pthrcol)),
        addExpression1);

    SgExprStatement * assignmentStatement2 =
        buildAssignStatement (buildVarRefExp (variableDeclarations->get (
            OP2::VariableNames::colour2)), arrayExpression1);

    appendStatement (assignmentStatement2, ifBody);

    SgExpression * ifGuardExpression =
        buildLessThanOp (buildVarRefExp (variableDeclarations->get (
            CommonVariableNames::iterationCounter1)), buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::nelems)));

    SgIfStmt * ifStatement =
        RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
            ifGuardExpression, ifBody);

    appendStatement (ifStatement, loopBody);

    appendStatement (createIncrementAdjustmentStatements (), loopBody);

    appendStatement (createStageOutFromLocalMemoryToSharedMemoryStatements (),
        loopBody);
  }
  else
  {
    appendStatement (createUserSubroutineCallStatement (), loopBody);
  }

  SgAddOp * addExpresssion1 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter1)), addExpresssion1);

  appendStatement (assignmentStatement1, loopBody);

  if (parallelLoop->hasIncrementedOpDats ())
  {
    SgLessThanOp * loopGuardExpression =
        buildLessThanOp (buildVarRefExp (variableDeclarations->get (
            CommonVariableNames::iterationCounter1)), buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::nelems2)));

    SgWhileStmt * loopStatement =
        buildWhileStmt (loopGuardExpression, loopBody);

    loopStatement->set_has_end_statement (true);

    appendStatement (loopStatement, subroutineScope);
  }
  else
  {
    SgLessThanOp * loopGuardExpression =
        buildLessThanOp (buildVarRefExp (variableDeclarations->get (
            CommonVariableNames::iterationCounter1)), buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::nelems)));

    SgWhileStmt * loopStatement =
        buildWhileStmt (loopGuardExpression, loopBody);

    loopStatement->set_has_end_statement (true);

    appendStatement (loopStatement, subroutineScope);
  }
}

SgBasicBlock *
FortranCUDAKernelSubroutineIndirectLoop::createInitialiseCUDASharedVariablesStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildWhileStmt;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise shared memory", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  unsigned int pindOffsOffset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        string const autosharedVariableName =
            OP2::VariableNames::getCUDASharedMemoryDeclarationName (
                parallelLoop->getOpDatBaseType (i),
                parallelLoop->getSizeOfOpDat (i));

        /*
         * ======================================================
         * Initialise the lower bound of the while loop
         * ======================================================
         */

        Debug::getInstance ()->debugMessage (
            "Initialise the lower bound of the while loop",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        SgSubtractOp * subtractExpression1 = buildSubtractOp (
            CUDA::getThreadId (THREAD_X, subroutineScope), buildIntVal (1));

        SgExprStatement * assignmentStatement1 = buildAssignStatement (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), subtractExpression1);

        appendStatement (assignmentStatement1, block);

        /*
         * ======================================================
         * Initialise the upper bound of the while loop
         * ======================================================
         */

        Debug::getInstance ()->debugMessage (
            "Initialise the upper bound of the while loop",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::opDatDimensions)),
            buildVarRefExp (dimensionsDeclaration->getOpDatDimensionField (i)));

        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getIndirectionArgumentSizeName (i))),
            dotExpression2);

        SgExprStatement * assignmentStatement2 = buildAssignStatement (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::upperBound)), multiplyExpression2);

        appendStatement (assignmentStatement2, block);

        /*
         * ======================================================
         * Statement to calculate modulus
         * ======================================================
         */

        Debug::getInstance ()->debugMessage ("While loop body",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        SgBasicBlock * loopBody = buildBasicBlock ();

        if (parallelLoop->isIncremented (i) == false)
        {
          SgFunctionSymbol * modFunctionSymbol =
              FortranTypesBuilder::buildNewFortranFunction ("mod",
                  subroutineScope);

          SgDotExp * dotExpression3 =
              buildDotExp (buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::opDatDimensions)), buildVarRefExp (
                  dimensionsDeclaration->getOpDatDimensionField (i)));

          SgExprListExp * modActualParameters = buildExprListExp (
              buildVarRefExp (variableDeclarations->get (
                  CommonVariableNames::iterationCounter1)), dotExpression3);

          SgFunctionCallExp * modFunctionCall = buildFunctionCallExp (
              modFunctionSymbol, modActualParameters);

          SgExprStatement * assignmentStatement3 = buildAssignStatement (
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::moduloResult)), modFunctionCall);

          appendStatement (assignmentStatement3, loopBody);

          /*
           * ======================================================
           * Statement to index shared memory array
           * ======================================================
           */
          Debug::getInstance ()->debugMessage (
              "Statement to index shared memory", Debug::HIGHEST_DEBUG_LEVEL,
              __FILE__, __LINE__);

          SgAddOp * addExpression4a = buildAddOp (buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::getNumberOfBytesVariableName (i))),
              buildVarRefExp (variableDeclarations->get (
                  CommonVariableNames::iterationCounter1)));

          SgPntrArrRefExp * arrayIndexExpression4a = buildPntrArrRefExp (
              buildVarRefExp (
                  variableDeclarations->get (autosharedVariableName)),
              addExpression4a);

          SgMultiplyOp * multiplyExpression4a = buildMultiplyOp (
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::blockID)), buildIntVal (
                  parallelLoop->getNumberOfDifferentIndirectOpDats ()));

          SgAddOp * addExpression4b = buildAddOp (buildIntVal (pindOffsOffset),
              multiplyExpression4a);

          SgPntrArrRefExp
              * arrayIndexExpression4b = buildPntrArrRefExp (buildVarRefExp (
                  variableDeclarations->get (
                      OP2::VariableNames::PlanFunction::pindOffs)),
                  addExpression4b);

          SgDotExp * dotExpression4 =
              buildDotExp (buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::opDatDimensions)), buildVarRefExp (
                  dimensionsDeclaration->getOpDatDimensionField (i)));

          SgDivideOp * divideExpression4 = buildDivideOp (
              buildVarRefExp (variableDeclarations->get (
                  CommonVariableNames::iterationCounter1)), dotExpression4);

          SgAddOp * addExpression4c = buildAddOp (arrayIndexExpression4b,
              divideExpression4);

          SgAddOp * addExpression4d = buildAddOp (buildIntVal (0),
              addExpression4c);

          SgPntrArrRefExp * arrayIndexExpression4c = buildPntrArrRefExp (
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::getLocalToGlobalMappingName (i))),
              addExpression4d);

          SgMultiplyOp * multiplyExpression4b = buildMultiplyOp (
              arrayIndexExpression4c, dotExpression4);

          SgAddOp * addExpression4e = buildAddOp (buildVarRefExp (
              variableDeclarations->get (OP2::VariableNames::moduloResult)),
              multiplyExpression4b);

          SgPntrArrRefExp * arrayIndexExpression4d = buildPntrArrRefExp (
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::getOpDatName (i))), addExpression4e);

          SgExprStatement * assignmentStatement4 = buildAssignStatement (
              arrayIndexExpression4a, arrayIndexExpression4d);

          appendStatement (assignmentStatement4, loopBody);

          pindOffsOffset++;
        }
        else
        {
          /*
           * ======================================================
           * Statement to index shared memory
           * ======================================================
           */
          SgAddOp * addExpression4 = buildAddOp (buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::getNumberOfBytesVariableName (i))),
              buildVarRefExp (variableDeclarations->get (
                  CommonVariableNames::iterationCounter1)));

          SgPntrArrRefExp * arrayIndexExpression4 = buildPntrArrRefExp (
              buildVarRefExp (
                  variableDeclarations->get (autosharedVariableName)),
              addExpression4);

          SgExprStatement * assignmentStatement4 = buildAssignStatement (
              arrayIndexExpression4, buildIntVal (0));

          appendStatement (assignmentStatement4, loopBody);
        }

        /*
         * ======================================================
         * Statement to increment loop counter
         * ======================================================
         */
        SgAddOp * addExpression5 = buildAddOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)),
            CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

        SgExprStatement * assignmentStatement5 = buildAssignStatement (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), addExpression5);

        appendStatement (assignmentStatement5, loopBody);

        /*
         * ======================================================
         * While loop guard
         * ======================================================
         */
        SgExpression * loopGuard = buildLessThanOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), buildVarRefExp (
                variableDeclarations->get (CommonVariableNames::upperBound)));

        SgWhileStmt * whileStatement = buildWhileStmt (loopGuard, loopBody);

        whileStatement->set_has_end_statement (true);

        appendStatement (whileStatement, block);
      }
    }
  }

  return block;
}

SgBasicBlock *
FortranCUDAKernelSubroutineIndirectLoop::createIncrementAccessThreadZeroStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating thread zero statements for increment access OP_DATs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgSubtractOp * subtractExpression1 = buildSubtractOp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::nelems)), buildIntVal (1));

  SgDivideOp * divideExpression1 = buildDivideOp (subtractExpression1,
      CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

  SgExpression * addExpression1 = buildAddOp (buildIntVal (1),
      divideExpression1);

  SgMultiplyOp * multiplyExpression1 =
      buildMultiplyOp (
          CUDA::getThreadBlockDimension (THREAD_X, subroutineScope),
          addExpression1);

  SgStatement * statement1 = buildAssignStatement (buildOpaqueVarRefExp (
      OP2::VariableNames::nelems2, subroutineScope), multiplyExpression1);

  appendStatement (statement1, block);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (OP2::VariableNames::PlanFunction::pnthrcol,
          subroutineScope), buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::blockID)));

  SgStatement * statement2 = buildAssignStatement (buildOpaqueVarRefExp (
      OP2::VariableNames::numberOfColours, subroutineScope), arrayExpression2);

  appendStatement (statement2, block);

  return block;
}

SgBasicBlock *
FortranCUDAKernelSubroutineIndirectLoop::createInitialiseBytesPerOpDatStatements ()
{
  using SageBuilder::buildIntVal;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise bytes per OP_DAT",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * Initialise round-up variables
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Initialising round-up variable for '"
                + parallelLoop->getOpDatVariableName (i) + "'",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::opDatDimensions)),
            buildVarRefExp (dimensionsDeclaration->getOpDatDimensionField (i)));

        SgMultiplyOp * multiplyExpression = buildMultiplyOp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getIndirectionArgumentSizeName (i))),
            dotExpression);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getRoundUpVariableName (i))),
            multiplyExpression);

        appendStatement (assignmentStatement, block);
      }
    }
  }

  /*
   * ======================================================
   * Initialise number of bytes variables
   * ======================================================
   */

  bool firstInitialization = true;

  for (unsigned int i = 1, lasti = 1; i
      <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      Debug::getInstance ()->debugMessage (
          "Initialising number of bytes variable for '"
              + parallelLoop->getOpDatVariableName (i) + "'",
          Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

      if (parallelLoop->isIndirect (i))
      {
        if (firstInitialization)
        {
          firstInitialization = false;

          SgExprStatement * assignmentStatement = buildAssignStatement (
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::getNumberOfBytesVariableName (i))),
              buildIntVal (0));

          appendStatement (assignmentStatement, block);
        }
        else
        {
          SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::getNumberOfBytesVariableName (lasti))),
              buildIntVal (parallelLoop->getSizeOfOpDat (lasti)));

          SgDivideOp * divideExpression1 = buildDivideOp (multiplyExpression1,
              buildIntVal (parallelLoop->getSizeOfOpDat (i)));

          SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
              variableDeclarations->get (
                  OP2::VariableNames::getRoundUpVariableName (lasti))),
              buildIntVal (parallelLoop->getSizeOfOpDat (lasti)));

          SgDivideOp * divideExpression2 = buildDivideOp (multiplyExpression2,
              buildIntVal (parallelLoop->getSizeOfOpDat (i)));

          SgAddOp * addExpression = buildAddOp (divideExpression1,
              divideExpression2);

          SgExprStatement * assignmentStatement = buildAssignStatement (
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::getNumberOfBytesVariableName (i))),
              addExpression);

          appendStatement (assignmentStatement, block);
        }

        lasti = i;
      }
    }
  }

  return block;
}

SgIfStmt *
FortranCUDAKernelSubroutineIndirectLoop::createThreadZeroStatements ()
{
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildEqualityOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Creating thread zero statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * ifBlock = buildBasicBlock ();

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgSubtractOp * subtractExpression1 = buildSubtractOp (CUDA::getBlockId (
      BLOCK_X, subroutineScope), buildIntVal (1));

  SgAddOp * arrayIndexExpression1 = buildAddOp (subtractExpression1,
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::blockOffset)));

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::PlanFunction::pblkMap)),
      arrayIndexExpression1);

  SgStatement * statement1 = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::blockID)),
      arrayExpression1);

  appendStatement (statement1, ifBlock);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (OP2::VariableNames::PlanFunction::pnelems,
          subroutineScope), buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::blockID)));

  SgStatement * statement2 = buildAssignStatement (buildOpaqueVarRefExp (
      OP2::VariableNames::nelems, subroutineScope), arrayExpression2);

  appendStatement (statement2, ifBlock);

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  if (parallelLoop->hasIncrementedOpDats ())
  {
    appendStatement (createIncrementAccessThreadZeroStatements (), ifBlock);
  }

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (OP2::VariableNames::PlanFunction::poffset,
          subroutineScope), buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::blockID)));

  SgStatement * statement3 = buildAssignStatement (buildOpaqueVarRefExp (
      OP2::VariableNames::sharedMemoryOffset, subroutineScope),
      arrayExpression3);

  appendStatement (statement3, ifBlock);

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  unsigned int offset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgMultiplyOp * multiplyExpression = buildMultiplyOp (buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::blockID)),
            buildIntVal (
                parallelLoop->getNumberOfDistinctIndirectOpDatArguments ()));

        SgAddOp * addExpression = buildAddOp (buildIntVal (offset),
            multiplyExpression);

        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::PlanFunction::pindSizes)), addExpression);

        SgStatement * statement = buildAssignStatement (buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getIndirectionArgumentSizeName (i))),
            arrayExpression);

        appendStatement (statement, ifBlock);

        ++offset;
      }
    }
  }

  /*
   * ======================================================
   * Add the if statement with a NULL else block
   * ======================================================
   */

  SgEqualityOp * ifGuardExpression = buildEqualityOp (buildSubtractOp (
      CUDA::getThreadId (THREAD_X, subroutineScope), buildIntVal (1)),
      buildIntVal (0));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBlock);

  return ifStatement;
}

void
FortranCUDAKernelSubroutineIndirectLoop::createStatements ()
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildExprStatement;
  using SageInterface::appendStatement;

  appendStatement (createThreadZeroStatements (), subroutineScope);

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      subroutineScope);

  appendStatement (createInitialiseBytesPerOpDatStatements (), subroutineScope);

  appendStatement (createInitialiseCUDASharedVariablesStatements (),
      subroutineScope);

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      subroutineScope);

  SgSubtractOp * subtractExpression1 = buildSubtractOp (CUDA::getThreadId (
      THREAD_X, subroutineScope), buildIntVal (1));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter1)), subtractExpression1);

  appendStatement (assignmentStatement1, subroutineScope);

  createExecutionLoopStatements ();

  appendStatement (createIncrementAndWriteAccessEpilogueStatements (),
      subroutineScope);
}

void
FortranCUDAKernelSubroutineIndirectLoop::createIncrementAccessLocalVariableDeclarations ()
{
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed for incremented OP_DATS",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIncremented (i))
    {
      Debug::getInstance ()->debugMessage (
          "Creating increment access mapping for OP_DAT " + lexical_cast <
              string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

      string const variableName =
          OP2::VariableNames::getIncrementAccessMapName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));
    }
  }

  variableDeclarations->add (OP2::VariableNames::numberOfColours,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::numberOfColours,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope, 1,
          CUDA_SHARED));

  variableDeclarations->add (OP2::VariableNames::nelems2,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::nelems2,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope, 1,
          CUDA_SHARED));

  variableDeclarations ->add (OP2::VariableNames::colour1,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::colour1,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations ->add (OP2::VariableNames::colour2,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::colour2,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations ->add (CommonVariableNames::iterationCounter2,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::iterationCounter2,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}

void
FortranCUDAKernelSubroutineIndirectLoop::createExecutionLocalVariableDeclarations ()
{
  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed to execute kernel",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (OP2::VariableNames::sharedMemoryOffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::sharedMemoryOffset,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope, 1,
          CUDA_SHARED));

  variableDeclarations->add (OP2::VariableNames::blockID,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::blockID,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope, 1,
          CUDA_SHARED));

  variableDeclarations->add (OP2::VariableNames::nelems,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::nelems,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope, 1,
          CUDA_SHARED));

  variableDeclarations ->add (OP2::VariableNames::moduloResult,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::moduloResult,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations ->add (OP2::VariableNames::nbytes,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::nbytes,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations ->add (CommonVariableNames::iterationCounter1,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::iterationCounter1,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations ->add (CommonVariableNames::upperBound,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::upperBound,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}

void
FortranCUDAKernelSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createCUDAStageInVariablesVariableDeclarations ();

  createCUDASharedVariableDeclarations ();

  createExecutionLocalVariableDeclarations ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & variableName =
        OP2::VariableNames::getNumberOfBytesVariableName (i);

    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      Debug::getInstance ()->debugMessage (
          "Creating number of bytes declaration for OP_DAT " + lexical_cast <
              string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));

      positionOfNbytes[parallelLoop->getOpDatVariableName (i)] = i;
    }
    else
    {
      Debug::getInstance ()->debugMessage (
          "Number of bytes declaration NOT needed for OP_DAT " + lexical_cast <
              string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

      variableDeclarations->add (variableName, variableDeclarations->get (
          OP2::VariableNames::getNumberOfBytesVariableName (
              positionOfNbytes[parallelLoop->getOpDatVariableName (i)])));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      Debug::getInstance ()->debugMessage ("Creating round up declaration "
          + lexical_cast <string> (i), Debug::INNER_LOOP_LEVEL, __FILE__,
          __LINE__);

      string const & variableName = OP2::VariableNames::getRoundUpVariableName (
          i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating size argument for OP_DAT " + lexical_cast <string> (i),
            Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

        string const variableName =
            OP2::VariableNames::getIndirectionArgumentSizeName (i);

        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, FortranTypesBuilder::getFourByteInteger (),
                subroutineScope, 1, CUDA_SHARED));
      }
    }
  }

  if (parallelLoop->hasIncrementedOpDats ())
  {
    createIncrementAccessLocalVariableDeclarations ();
  }
}

void
FortranCUDAKernelSubroutineIndirectLoop::createPlanFormalParameterDeclarations ()
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using std::string;
  using std::map;

  Debug::getInstance ()->debugMessage (
      "Creating plan formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  map <string, string> variableToSizeMap;

  variableToSizeMap[OP2::VariableNames::PlanFunction::pindSizes]
      = OP2::VariableNames::PlanFunction::pindSizesSize;

  variableToSizeMap[OP2::VariableNames::PlanFunction::pindOffs]
      = OP2::VariableNames::PlanFunction::pindOffsSize;

  variableToSizeMap[OP2::VariableNames::PlanFunction::pblkMap]
      = OP2::VariableNames::PlanFunction::pblkMapSize;

  variableToSizeMap[OP2::VariableNames::PlanFunction::poffset]
      = OP2::VariableNames::PlanFunction::poffsetSize;

  variableToSizeMap[OP2::VariableNames::PlanFunction::pnelems]
      = OP2::VariableNames::PlanFunction::pnelemsSize;

  variableToSizeMap[OP2::VariableNames::PlanFunction::pnthrcol]
      = OP2::VariableNames::PlanFunction::pnthrcolSize;

  variableToSizeMap[OP2::VariableNames::PlanFunction::pthrcol]
      = OP2::VariableNames::PlanFunction::pthrcolSize;

  for (map <string, string>::const_iterator it = variableToSizeMap.begin (); it
      != variableToSizeMap.end (); ++it)
  {
    SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
        variableDeclarations->get (OP2::VariableNames::opDatCardinalities)),
        buildVarRefExp (cardinalitiesDeclaration->getFieldDeclarations ()->get (
            it->second)));

    SgSubtractOp * upperBoundExpression = buildSubtractOp (dotExpression,
        buildIntVal (1));

    variableDeclarations->add (
        it->first,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            it->first,
            FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                FortranTypesBuilder::getFourByteInteger (), buildIntVal (0),
                upperBoundExpression), subroutineScope, formalParameters, 1,
            CUDA_DEVICE));
  }

  variableDeclarations->add (
      OP2::VariableNames::PlanFunction::blockOffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::PlanFunction::blockOffset,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));
}

void
FortranCUDAKernelSubroutineIndirectLoop::createOpDatFormalParameterDeclarations ()
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntVal;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const variableName = OP2::VariableNames::getOpDatName (i);

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
      else if (parallelLoop->isIndirect (i) || parallelLoop->isDirect (i))
      {
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

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgDotExp * dotExpression2 = buildDotExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::opDatCardinalities)), buildVarRefExp (
                cardinalitiesDeclaration->getFieldDeclarations ()->get (
                    OP2::VariableNames::getLocalToGlobalMappingSizeName (i))));

        SgSubtractOp * upperBoundExpression2 = buildSubtractOp (dotExpression2,
            buildIntVal (1));

        variableDeclarations->add (
            OP2::VariableNames::getLocalToGlobalMappingName (i),
            FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                OP2::VariableNames::getLocalToGlobalMappingName (i),
                FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                    FortranTypesBuilder::getFourByteInteger (),
                    buildIntVal (0), upperBoundExpression2), subroutineScope,
                formalParameters, 1, CUDA_DEVICE));
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      SgIntVal * lowerBoundExpression = buildIntVal (0);

      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::opDatCardinalities)),
          buildVarRefExp (
              cardinalitiesDeclaration->getFieldDeclarations ()->get (
                  OP2::VariableNames::getGlobalToLocalMappingSizeName (i))));

      SgSubtractOp * upperBoundExpression = buildSubtractOp (dotExpression,
          buildIntVal (1));

      variableDeclarations->add (
          OP2::VariableNames::getGlobalToLocalMappingName (i),
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              OP2::VariableNames::getGlobalToLocalMappingName (i),
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  FortranTypesBuilder::getTwoByteInteger (),
                  lowerBoundExpression, upperBoundExpression), subroutineScope,
              formalParameters, 1, CUDA_DEVICE));
    }
  }
}

void
FortranCUDAKernelSubroutineIndirectLoop::createFormalParameterDeclarations ()
{
  Debug::getInstance ()->debugMessage (
      "Creating formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (
      OP2::VariableNames::opDatDimensions,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::opDatDimensions,
          dimensionsDeclaration->getType (), subroutineScope, formalParameters,
          1, CUDA_DEVICE));

  variableDeclarations->add (
      OP2::VariableNames::opDatCardinalities,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::opDatCardinalities,
          cardinalitiesDeclaration->getType (), subroutineScope,
          formalParameters, 1, CUDA_DEVICE));

  createOpDatFormalParameterDeclarations ();

  createPlanFormalParameterDeclarations ();
}

FortranCUDAKernelSubroutineIndirectLoop::FortranCUDAKernelSubroutineIndirectLoop (
    SgScopeStatement * moduleScope,
    FortranCUDAUserSubroutine * userSubroutine,
    FortranParallelLoop * parallelLoop,
    FortranReductionSubroutines * reductionSubroutines,
    FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop * cardinalitiesDeclaration,
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
