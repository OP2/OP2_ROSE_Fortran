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
  using namespace SageBuilder;
  using namespace CommonVariableNames;
  using namespace OP2::VariableNames;
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

        parameterExpression = variableDeclarations->getReference (
            getOpDatLocalName (i));
      }
      else if (parallelLoop->isRead (i))
      {
        Debug::getInstance ()->debugMessage (
            "Indirect OP_DAT with read access", Debug::OUTER_LOOP_LEVEL,
            __FILE__, __LINE__);

        string const autosharedVariableName =
            getCUDASharedMemoryDeclarationName (parallelLoop->getOpDatBaseType (
                i), parallelLoop->getSizeOfOpDat (i));

        SgAddOp * addExpression1 = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)),
            variableDeclarations->getReference (sharedMemoryOffset));

        SgPntrArrRefExp * arrayExpression1 =
            buildPntrArrRefExp (variableDeclarations->getReference (
                getGlobalToLocalMappingName (i)), addExpression1);

        SgDotExp * dotExpression1 = buildDotExp (
            variableDeclarations->getReference (opDatDimensions),
            dimensionsDeclaration->getOpDatDimensionField (i));

        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (arrayExpression1,
            dotExpression1);

        SgAddOp * addExpression2 = buildAddOp (
            variableDeclarations->getReference (
                getNumberOfBytesVariableName (i)), multiplyExpression1);

        parameterExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (autosharedVariableName),
            addExpression2);
      }
      else
      {
        Debug::getInstance ()->debugMessage (
            "Indirect OP_DAT with read/write or write access",
            Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

        string const autosharedVariableName =
            getCUDASharedMemoryDeclarationName (parallelLoop->getOpDatBaseType (
                i), parallelLoop->getSizeOfOpDat (i));

        SgAddOp * addExpression1 = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)),
            variableDeclarations->getReference (sharedMemoryOffset));

        SgPntrArrRefExp * arrayExpression1 =
            buildPntrArrRefExp (variableDeclarations->getReference (
                getGlobalToLocalMappingName (i)), addExpression1);

        SgDotExp * dotExpression1 = buildDotExp (
            variableDeclarations->getReference (opDatDimensions),
            dimensionsDeclaration->getOpDatDimensionField (i));

        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (arrayExpression1,
            dotExpression1);

        SgAddOp * addExpression2 = buildAddOp (
            variableDeclarations->getReference (
                getNumberOfBytesVariableName (i)), multiplyExpression1);

        SgAddOp * addExpression3 = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)),
            variableDeclarations->getReference (sharedMemoryOffset));

        SgPntrArrRefExp * arrayExpression2 =
            buildPntrArrRefExp (variableDeclarations->getReference (
                getGlobalToLocalMappingName (i)), addExpression3);

        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (arrayExpression2,
            dotExpression1);

        SgAddOp * addExpression4 = buildAddOp (
            variableDeclarations->getReference (
                getNumberOfBytesVariableName (i)), multiplyExpression2);

        SgAddOp * addExpression5 = buildAddOp (addExpression4, dotExpression1);

        SgSubscriptExpression * subscriptExpression =
            new SgSubscriptExpression (RoseHelper::getFileInfo (),
                addExpression2, addExpression5, buildIntVal (1));

        subscriptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

        parameterExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (autosharedVariableName),
            buildExprListExp (subscriptExpression));
      }
    }
    else if (parallelLoop->isDirect (i))
    {
      Debug::getInstance ()->debugMessage ("Direct OP_DAT",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      SgAddOp * addExpression1 = buildAddOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              1)), variableDeclarations->getReference (sharedMemoryOffset));

      SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (opDatDimensions),
          dimensionsDeclaration->getOpDatDimensionField (i));

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

      parameterExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (getOpDatName (i)),
          accessExpression);
    }
    else if (parallelLoop->isReductionRequired (i))
    {
      parameterExpression = variableDeclarations->getReference (
          getOpDatLocalName (i));
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

        parameterExpression = variableDeclarations->getReference (getOpDatName (
            i));
      }
      else
      {
        Debug::getInstance ()->debugMessage ("OP_GBL with read access (Array)",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        SgDotExp * dotExpression = buildDotExp (
            variableDeclarations->getReference (opDatCardinalities),
            cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
                getOpDatCardinalityName (i)));

        SgSubtractOp * subtractExpression = buildSubtractOp (dotExpression,
            buildIntVal (1));

        SgSubscriptExpression * subscriptExpression =
            new SgSubscriptExpression (RoseHelper::getFileInfo (), buildIntVal (
                0), subtractExpression, buildIntVal (1));

        subscriptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

        parameterExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (getOpDatName (i)),
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
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace CommonVariableNames;
  using namespace OP2::VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating increment and write access epilogue statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

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
              getCUDASharedMemoryDeclarationName (
                  parallelLoop->getOpDatBaseType (i),
                  parallelLoop->getSizeOfOpDat (i));
          /*
           * ======================================================
           * Defining lower and upper bounds and increment
           * ======================================================
           */

          SgDotExp * dotExpression2 = buildDotExp (
              variableDeclarations->getReference (opDatDimensions),
              dimensionsDeclaration->getOpDatDimensionField (i));

          SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
              variableDeclarations->getReference (
                  getIndirectionArgumentSizeName (i)), dotExpression2);

          SgLessThanOp * lessThanExpression1 = buildLessThanOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), multiplyExpression1);

          SgSubtractOp * subtractExpression1 = buildSubtractOp (
              CUDA::getThreadId (THREAD_X, subroutineScope), buildIntVal (1));

          SgExprStatement * assignmentStatement1 = buildAssignStatement (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), subtractExpression1);

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
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), dotExpression2);

          SgFunctionCallExp * functionCall = buildFunctionCallExp (
              functionSymbol, actualParameters);

          SgExprStatement * assignmentStatement2 = buildAssignStatement (
              variableDeclarations->getReference (moduloResult), functionCall);

          /*
           * ======================================================
           * Defining device variable accessing expressions
           * ======================================================
           */

          SgMultiplyOp * multiplyExpression2a = buildMultiplyOp (
              variableDeclarations->getReference (blockID), buildIntVal (
                  parallelLoop->getNumberOfDifferentIndirectOpDats ()));

          SgAddOp * addExpression2a = buildAddOp (buildIntVal (pindOffsOffset),
              multiplyExpression2a);

          SgPntrArrRefExp * arrayIndexExpression2a = buildPntrArrRefExp (
              variableDeclarations->getReference (PlanFunction::pindOffs),
              addExpression2a);

          SgDivideOp * divideExpression2 = buildDivideOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), dotExpression2);

          SgAddOp * addExpression2b = buildAddOp (arrayIndexExpression2a,
              divideExpression2);

          SgAddOp * addExpression2c = buildAddOp (buildIntVal (0),
              addExpression2b);

          SgPntrArrRefExp * arrayIndexExpression2b = buildPntrArrRefExp (
              variableDeclarations->getReference (getLocalToGlobalMappingName (
                  i)), addExpression2c);

          SgMultiplyOp * multiplyExpression2b = buildMultiplyOp (
              arrayIndexExpression2b, dotExpression2);

          SgAddOp * addExpression2d = buildAddOp (
              variableDeclarations->getReference (moduloResult),
              multiplyExpression2b);

          SgPntrArrRefExp * arrayIndexExpression2c = buildPntrArrRefExp (
              variableDeclarations->getReference (getOpDatName (i)),
              addExpression2d);

          /*
           * ======================================================
           * Defining shared variable accessing expressions
           * ======================================================
           */

          SgAddOp * addExpression2e = buildAddOp (
              variableDeclarations->getReference (getNumberOfBytesVariableName (
                  i)), variableDeclarations->getReference (
                  getIterationCounterVariableName (1)));

          SgPntrArrRefExp * arrayIndexExpression2d = buildPntrArrRefExp (
              variableDeclarations->getReference (autosharedVariableName),
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
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)),
              CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

          SgExprStatement * assignmentStatement4 = buildAssignStatement (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), addExpression2f);

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
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace CommonVariableNames;
  using namespace OP2::VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating stage out from local memory to shared memory statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  SgBasicBlock * ifBody = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      string const autosharedVariableName = getCUDASharedMemoryDeclarationName (
          parallelLoop->getOpDatBaseType (i), parallelLoop->getSizeOfOpDat (i));

      SgBasicBlock * innerLoopBody = buildBasicBlock ();

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
          variableDeclarations->getReference (getOpDatLocalName (i)),
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (opDatDimensions),
          dimensionsDeclaration->getOpDatDimensionField (i));

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (
          variableDeclarations->getReference (getIncrementAccessMapName (i)),
          dotExpression);

      SgAddOp * addExpression1 = buildAddOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), multiplyExpression);

      SgAddOp * addExpression2 =
          buildAddOp (variableDeclarations->getReference (
              getNumberOfBytesVariableName (i)), addExpression1);

      SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
          variableDeclarations->getReference (autosharedVariableName),
          addExpression2);

      SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
          variableDeclarations->getReference (autosharedVariableName),
          addExpression2);

      SgAddOp * addExpression3 =
          buildAddOp (arrayExpression2, arrayExpression1);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression3, addExpression3);

      appendStatement (assignmentStatement, innerLoopBody);

      SgAssignOp * innerLoopLowerBoundExpression = buildAssignOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (0));

      SgSubtractOp * innerLoopUpperBoundExpression = buildSubtractOp (
          dotExpression, buildIntVal (1));

      SgFortranDo * innerLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              innerLoopLowerBoundExpression, innerLoopUpperBoundExpression,
              buildIntVal (1), innerLoopBody);

      appendStatement (innerLoopStatement, ifBody);
    }
  }

  SgEqualityOp * ifGuardExpression = buildEqualityOp (
      variableDeclarations->getReference (colour2),
      variableDeclarations->getReference (colour1));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  SgBasicBlock * loopBody = buildBasicBlock ();

  appendStatement (ifStatement, loopBody);

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      loopBody);

  SgAssignOp * lowerBoundExpression = buildAssignOp (
      variableDeclarations->getReference (colour1), buildIntVal (0));

  SgSubtractOp * upperBoundExpression = buildSubtractOp (
      variableDeclarations->getReference (numberOfColours), buildIntVal (1));

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
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace CommonVariableNames;
  using namespace OP2::VariableNames;
  using boost::lexical_cast;
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

      SgAddOp * addExpression = buildAddOp (variableDeclarations->getReference (
          getIterationCounterVariableName (1)),
          variableDeclarations->getReference (sharedMemoryOffset));

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (getGlobalToLocalMappingName (i)),
          addExpression);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          variableDeclarations->getReference (getIncrementAccessMapName (i)),
          arrayExpression);

      appendStatement (assignmentStatement, block);
    }
  }

  return block;
}

SgBasicBlock *
FortranCUDAKernelSubroutineIndirectLoop::createInitialiseIncrementAccessStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace CommonVariableNames;
  using namespace OP2::VariableNames;
  using boost::lexical_cast;
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
          variableDeclarations->getReference (getOpDatLocalName (i)),
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression, buildIntVal (0));

      appendStatement (assignmentStatement, loopBody);

      SgAssignOp * lowerBoundExpression = buildAssignOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (0));

      SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (opDatDimensions),
          dimensionsDeclaration->getOpDatDimensionField (i));

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
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace CommonVariableNames;
  using namespace OP2::VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating main execution loop statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * loopBody = buildBasicBlock ();

  if (parallelLoop->hasIncrementedOpDats ())
  {
    SgExprStatement * assignmentStatement1 = buildAssignStatement (
        variableDeclarations->getReference (colour2), buildIntVal (-1));

    appendStatement (assignmentStatement1, loopBody);

    SgBasicBlock * ifBody = buildBasicBlock ();

    appendStatement (createInitialiseIncrementAccessStatements (), ifBody);

    appendStatement (createUserSubroutineCallStatement (), ifBody);

    SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
        getIterationCounterVariableName (1)),
        variableDeclarations->getReference (sharedMemoryOffset));

    SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
        variableDeclarations->getReference (PlanFunction::pthrcol),
        addExpression1);

    SgExprStatement * assignmentStatement2 = buildAssignStatement (
        variableDeclarations->getReference (colour2), arrayExpression1);

    appendStatement (assignmentStatement2, ifBody);

    SgExpression * ifGuardExpression =
        buildLessThanOp (variableDeclarations->getReference (
            getIterationCounterVariableName (1)),
            variableDeclarations->getReference (nelems));

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

  SgAddOp * addExpresssion1 = buildAddOp (variableDeclarations->getReference (
      getIterationCounterVariableName (1)), CUDA::getThreadBlockDimension (
      THREAD_X, subroutineScope));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      addExpresssion1);

  appendStatement (assignmentStatement1, loopBody);

  if (parallelLoop->hasIncrementedOpDats ())
  {
    SgLessThanOp * loopGuardExpression =
        buildLessThanOp (variableDeclarations->getReference (
            getIterationCounterVariableName (1)),
            variableDeclarations->getReference (nelems2));

    SgWhileStmt * loopStatement =
        buildWhileStmt (loopGuardExpression, loopBody);

    loopStatement->set_has_end_statement (true);

    appendStatement (loopStatement, subroutineScope);
  }
  else
  {
    SgLessThanOp * loopGuardExpression =
        buildLessThanOp (variableDeclarations->getReference (
            getIterationCounterVariableName (1)),
            variableDeclarations->getReference (nelems));

    SgWhileStmt * loopStatement =
        buildWhileStmt (loopGuardExpression, loopBody);

    loopStatement->set_has_end_statement (true);

    appendStatement (loopStatement, subroutineScope);
  }
}

SgBasicBlock *
FortranCUDAKernelSubroutineIndirectLoop::createInitialiseCUDASharedVariablesStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace CommonVariableNames;
  using namespace OP2::VariableNames;
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
            getCUDASharedMemoryDeclarationName (parallelLoop->getOpDatBaseType (
                i), parallelLoop->getSizeOfOpDat (i));

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
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), subtractExpression1);

        appendStatement (assignmentStatement1, block);

        /*
         * ======================================================
         * Initialise the upper bound of the while loop
         * ======================================================
         */

        Debug::getInstance ()->debugMessage (
            "Initialise the upper bound of the while loop",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        SgDotExp * dotExpression2 = buildDotExp (
            variableDeclarations->getReference (opDatDimensions),
            dimensionsDeclaration->getOpDatDimensionField (i));

        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
            variableDeclarations->getReference (getIndirectionArgumentSizeName (
                i)), dotExpression2);

        SgExprStatement * assignmentStatement2 = buildAssignStatement (
            variableDeclarations->getReference (getUpperBoundVariableName (1)),
            multiplyExpression2);

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

          SgDotExp * dotExpression3 = buildDotExp (
              variableDeclarations->getReference (opDatDimensions),
              dimensionsDeclaration->getOpDatDimensionField (i));

          SgExprListExp * modActualParameters = buildExprListExp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), dotExpression3);

          SgFunctionCallExp * modFunctionCall = buildFunctionCallExp (
              modFunctionSymbol, modActualParameters);

          SgExprStatement * assignmentStatement3 = buildAssignStatement (
              variableDeclarations->getReference (moduloResult),
              modFunctionCall);

          appendStatement (assignmentStatement3, loopBody);

          /*
           * ======================================================
           * Statement to index shared memory array
           * ======================================================
           */
          Debug::getInstance ()->debugMessage (
              "Statement to index shared memory", Debug::HIGHEST_DEBUG_LEVEL,
              __FILE__, __LINE__);

          SgAddOp * addExpression4a = buildAddOp (
              variableDeclarations->getReference (getNumberOfBytesVariableName (
                  i)), variableDeclarations->getReference (
                  getIterationCounterVariableName (1)));

          SgPntrArrRefExp * arrayIndexExpression4a = buildPntrArrRefExp (
              variableDeclarations->getReference (autosharedVariableName),
              addExpression4a);

          SgMultiplyOp * multiplyExpression4a = buildMultiplyOp (
              variableDeclarations->getReference (blockID), buildIntVal (
                  parallelLoop->getNumberOfDifferentIndirectOpDats ()));

          SgAddOp * addExpression4b = buildAddOp (buildIntVal (pindOffsOffset),
              multiplyExpression4a);

          SgPntrArrRefExp * arrayIndexExpression4b = buildPntrArrRefExp (
              variableDeclarations->getReference (PlanFunction::pindOffs),
              addExpression4b);

          SgDotExp * dotExpression4 = buildDotExp (
              variableDeclarations->getReference (opDatDimensions),
              dimensionsDeclaration->getOpDatDimensionField (i));

          SgDivideOp * divideExpression4 = buildDivideOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), dotExpression4);

          SgAddOp * addExpression4c = buildAddOp (arrayIndexExpression4b,
              divideExpression4);

          SgAddOp * addExpression4d = buildAddOp (buildIntVal (0),
              addExpression4c);

          SgPntrArrRefExp * arrayIndexExpression4c = buildPntrArrRefExp (
              variableDeclarations->getReference (getLocalToGlobalMappingName (
                  i)), addExpression4d);

          SgMultiplyOp * multiplyExpression4b = buildMultiplyOp (
              arrayIndexExpression4c, dotExpression4);

          SgAddOp * addExpression4e = buildAddOp (
              variableDeclarations->getReference (moduloResult),
              multiplyExpression4b);

          SgPntrArrRefExp * arrayIndexExpression4d = buildPntrArrRefExp (
              variableDeclarations->getReference (getOpDatName (i)),
              addExpression4e);

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
          SgAddOp * addExpression4 = buildAddOp (
              variableDeclarations->getReference (getNumberOfBytesVariableName (
                  i)), variableDeclarations->getReference (
                  getIterationCounterVariableName (1)));

          SgPntrArrRefExp * arrayIndexExpression4 = buildPntrArrRefExp (
              variableDeclarations->getReference (autosharedVariableName),
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
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)),
            CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

        SgExprStatement * assignmentStatement5 = buildAssignStatement (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), addExpression5);

        appendStatement (assignmentStatement5, loopBody);

        /*
         * ======================================================
         * While loop guard
         * ======================================================
         */
        SgExpression * loopGuard = buildLessThanOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)),
            variableDeclarations->getReference (getUpperBoundVariableName (1)));

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
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace CommonVariableNames;
  using namespace OP2::VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating thread zero statements for incremented OP_DATs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgSubtractOp * subtractExpression1 = buildSubtractOp (
      variableDeclarations->getReference (nelems), buildIntVal (1));

  SgDivideOp * divideExpression1 = buildDivideOp (subtractExpression1,
      CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

  SgExpression * addExpression1 = buildAddOp (buildIntVal (1),
      divideExpression1);

  SgMultiplyOp * multiplyExpression1 =
      buildMultiplyOp (
          CUDA::getThreadBlockDimension (THREAD_X, subroutineScope),
          addExpression1);

  SgStatement * statement1 = buildAssignStatement (buildOpaqueVarRefExp (
      nelems2, subroutineScope), multiplyExpression1);

  appendStatement (statement1, block);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (PlanFunction::pnthrcol, subroutineScope),
      variableDeclarations->getReference (blockID));

  SgStatement * statement2 = buildAssignStatement (buildOpaqueVarRefExp (
      numberOfColours, subroutineScope), arrayExpression2);

  appendStatement (statement2, block);

  return block;
}

SgBasicBlock *
FortranCUDAKernelSubroutineIndirectLoop::createInitialiseBytesPerOpDatStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace CommonVariableNames;
  using namespace OP2::VariableNames;

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

        SgDotExp * dotExpression = buildDotExp (
            variableDeclarations->getReference (opDatDimensions),
            dimensionsDeclaration->getOpDatDimensionField (i));

        SgMultiplyOp * multiplyExpression = buildMultiplyOp (
            variableDeclarations->getReference (getIndirectionArgumentSizeName (
                i)), dotExpression);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            variableDeclarations->getReference (getRoundUpVariableName (i)),
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
              variableDeclarations->getReference (getNumberOfBytesVariableName (
                  i)), buildIntVal (0));

          appendStatement (assignmentStatement, block);
        }
        else
        {
          SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
              variableDeclarations->getReference (getNumberOfBytesVariableName (
                  lasti)), buildIntVal (parallelLoop->getSizeOfOpDat (lasti)));

          SgDivideOp * divideExpression1 = buildDivideOp (multiplyExpression1,
              buildIntVal (parallelLoop->getSizeOfOpDat (i)));

          SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
              variableDeclarations->getReference (
                  getRoundUpVariableName (lasti)), buildIntVal (
                  parallelLoop->getSizeOfOpDat (lasti)));

          SgDivideOp * divideExpression2 = buildDivideOp (multiplyExpression2,
              buildIntVal (parallelLoop->getSizeOfOpDat (i)));

          SgAddOp * addExpression = buildAddOp (divideExpression1,
              divideExpression2);

          SgExprStatement * assignmentStatement = buildAssignStatement (
              variableDeclarations->getReference (getNumberOfBytesVariableName (
                  i)), addExpression);

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
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace CommonVariableNames;
  using namespace OP2::VariableNames;

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
      variableDeclarations->getReference (PlanFunction::blockOffset));

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (PlanFunction::pblkMap),
      arrayIndexExpression1);

  SgStatement * statement1 = buildAssignStatement (
      variableDeclarations->getReference (blockID), arrayExpression1);

  appendStatement (statement1, ifBlock);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (PlanFunction::pnelems, subroutineScope),
      variableDeclarations->getReference (blockID));

  SgStatement * statement2 = buildAssignStatement (buildOpaqueVarRefExp (
      nelems, subroutineScope), arrayExpression2);

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
      buildOpaqueVarRefExp (PlanFunction::poffset, subroutineScope),
      variableDeclarations->getReference (blockID));

  SgStatement * statement3 = buildAssignStatement (buildOpaqueVarRefExp (
      sharedMemoryOffset, subroutineScope), arrayExpression3);

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
        SgMultiplyOp * multiplyExpression = buildMultiplyOp (
            variableDeclarations->getReference (blockID), buildIntVal (
                parallelLoop->getNumberOfDistinctIndirectOpDatArguments ()));

        SgAddOp * addExpression = buildAddOp (buildIntVal (offset),
            multiplyExpression);

        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (PlanFunction::pindSizes),
            addExpression);

        SgStatement * statement = buildAssignStatement (
            variableDeclarations->getReference (getIndirectionArgumentSizeName (
                i)), arrayExpression);

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
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace CommonVariableNames;

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
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      subtractExpression1);

  appendStatement (assignmentStatement1, subroutineScope);

  createExecutionLoopStatements ();

  appendStatement (createIncrementAndWriteAccessEpilogueStatements (),
      subroutineScope);
}

void
FortranCUDAKernelSubroutineIndirectLoop::createIncrementAccessLocalVariableDeclarations ()
{
  using namespace CommonVariableNames;
  using namespace OP2::VariableNames;
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

      string const variableName = getIncrementAccessMapName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));
    }
  }

  variableDeclarations->add (numberOfColours,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfColours, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, 1, CUDA_SHARED));

  variableDeclarations->add (nelems2,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          nelems2, FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          1, CUDA_SHARED));

  variableDeclarations ->add (colour1,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          colour1, FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations ->add (colour2,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          colour2, FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations ->add (getIterationCounterVariableName (2),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (2),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}

void
FortranCUDAKernelSubroutineIndirectLoop::createExecutionLocalVariableDeclarations ()
{
  using namespace CommonVariableNames;
  using namespace OP2::VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed to execute kernel",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (sharedMemoryOffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          sharedMemoryOffset, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, 1, CUDA_SHARED));

  variableDeclarations->add (blockID,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          blockID, FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          1, CUDA_SHARED));

  variableDeclarations->add (nelems,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          nelems, FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          1, CUDA_SHARED));

  variableDeclarations ->add (moduloResult,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          moduloResult, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations ->add (nbytes,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          nbytes, FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations ->add (getIterationCounterVariableName (1),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations ->add (getUpperBoundVariableName (1),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getUpperBoundVariableName (1),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}

void
FortranCUDAKernelSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  using namespace OP2::VariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createCUDAStageInVariablesVariableDeclarations ();

  createCUDASharedVariableDeclarations ();

  createExecutionLocalVariableDeclarations ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & variableName = getNumberOfBytesVariableName (i);

    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      Debug::getInstance ()->debugMessage (
          "Creating number of bytes declaration for OP_DAT " + lexical_cast <
              string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

      SgVariableDeclaration * variableDeclaration =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope);

      variableDeclarations->add (variableName, variableDeclaration);

      numberOfBytesDeclarations[parallelLoop->getOpDatVariableName (i)]
          = variableDeclaration;
    }
    else
    {
      Debug::getInstance ()->debugMessage (
          "Number of bytes declaration NOT needed for OP_DAT " + lexical_cast <
              string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

      variableDeclarations ->add (variableName,
          numberOfBytesDeclarations[parallelLoop->getOpDatVariableName (i)]);
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      Debug::getInstance ()->debugMessage ("Creating round up declaration "
          + lexical_cast <string> (i), Debug::INNER_LOOP_LEVEL, __FILE__,
          __LINE__);

      string const & variableName = getRoundUpVariableName (i);

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

        string const variableName = getIndirectionArgumentSizeName (i);

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
  using namespace SageBuilder;
  using namespace CommonVariableNames;
  using namespace OP2::VariableNames;
  using std::string;
  using std::map;

  Debug::getInstance ()->debugMessage (
      "Creating plan formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  map <string, string> variableToSizeMap;

  variableToSizeMap[PlanFunction::pindSizes] = PlanFunction::pindSizesSize;

  variableToSizeMap[PlanFunction::pindOffs] = PlanFunction::pindOffsSize;

  variableToSizeMap[PlanFunction::pblkMap] = PlanFunction::pblkMapSize;

  variableToSizeMap[PlanFunction::poffset] = PlanFunction::poffsetSize;

  variableToSizeMap[PlanFunction::pnelems] = PlanFunction::pnelemsSize;

  variableToSizeMap[PlanFunction::pnthrcol] = PlanFunction::pnthrcolSize;

  variableToSizeMap[PlanFunction::pthrcol] = PlanFunction::pthrcolSize;

  for (map <string, string>::const_iterator it = variableToSizeMap.begin (); it
      != variableToSizeMap.end (); ++it)
  {
    SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
        opDatCardinalities),
        cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
            it->second));

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
      PlanFunction::blockOffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          PlanFunction::blockOffset,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));
}

void
FortranCUDAKernelSubroutineIndirectLoop::createOpDatFormalParameterDeclarations ()
{
  using namespace CommonVariableNames;
  using namespace OP2::VariableNames;
  using namespace SageBuilder;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i))
      {
        string const & variableName = getReductionArrayDeviceName (i);

        variableDeclarations->add (
            variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, FortranTypesBuilder::getArray_RankOne (
                    parallelLoop->getOpDatBaseType (i)), subroutineScope,
                formalParameters, 1, CUDA_DEVICE));
      }
      else if (parallelLoop->isIndirect (i) || parallelLoop->isDirect (i))
      {
        string const variableName = getOpDatName (i);

        SgDotExp * dotExpression = buildDotExp (
            variableDeclarations->getReference (opDatCardinalities),
            cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
                getOpDatCardinalityName (i)));

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
        string const & variableName = getOpDatName (i);

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
        string const & variableName = getLocalToGlobalMappingName (i);

        SgDotExp * dotExpression2 = buildDotExp (
            variableDeclarations->getReference (opDatCardinalities),
            cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
                getLocalToGlobalMappingSizeName (i)));

        SgSubtractOp * upperBoundExpression2 = buildSubtractOp (dotExpression2,
            buildIntVal (1));

        variableDeclarations ->add (
            variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName,
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
      string const & variableName = getGlobalToLocalMappingName (i);

      SgIntVal * lowerBoundExpression = buildIntVal (0);

      SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (opDatCardinalities),
          cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
              getGlobalToLocalMappingSizeName (i)));

      SgSubtractOp * upperBoundExpression = buildSubtractOp (dotExpression,
          buildIntVal (1));

      variableDeclarations->add (
          variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              variableName,
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
  using namespace OP2::VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (
      opDatDimensions,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          opDatDimensions, dimensionsDeclaration->getType (), subroutineScope,
          formalParameters, 1, CUDA_DEVICE));

  variableDeclarations->add (
      opDatCardinalities,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          opDatCardinalities, cardinalitiesDeclaration->getType (),
          subroutineScope, formalParameters, 1, CUDA_DEVICE));

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
