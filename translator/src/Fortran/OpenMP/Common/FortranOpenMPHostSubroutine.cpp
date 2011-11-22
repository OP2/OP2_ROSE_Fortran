#include "FortranOpenMPHostSubroutine.h"
#include "FortranKernelSubroutine.h"
#include "FortranParallelLoop.h"
#include "FortranOpenMPModuleDeclarations.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "FortranTypesBuilder.h"
#include "RoseHelper.h"
#include "CompilerGeneratedNames.h"
#include "OP2.h"
#include "OpenMP.h"

void
FortranOpenMPHostSubroutine::createReductionEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace ReductionVariableNames;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating reduction epilogue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Create the inner loop
   * ======================================================
   */

  SgBasicBlock * outerLoopBody = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i))
    {
      SgAssignOp * innerLoopInitializationExpression = buildAssignOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (0));

      SgSubtractOp * innerLoopUpperBoundExpression = buildSubtractOp (
          buildIntVal (parallelLoop->getOpDatDimension (i)), buildIntVal (1));

      SgBasicBlock * innerLoopBody = buildBasicBlock ();

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              1)), buildIntVal (64));

      SgAddOp * addExpression1 = buildAddOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), multiplyExpression1);

      SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (
          variableDeclarations->getReference (getReductionArrayHostName (i)),
          addExpression1);

      SgAddOp * addExpression2 = buildAddOp (buildIntVal (
          parallelLoop->getOpDatDimension (i)),
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgPntrArrRefExp * arrayIndexExpression2 = buildPntrArrRefExp (
          variableDeclarations->getReference (getOpDatLocalName (i)),
          addExpression2);

      SgAddOp * addExpression3 = buildAddOp (buildIntVal (
          parallelLoop->getOpDatDimension (i)),
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgPntrArrRefExp * arrayIndexExpression3 = buildPntrArrRefExp (
          variableDeclarations->getReference (getOpDatLocalName (i)),
          addExpression3);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayIndexExpression3, buildAddOp (arrayIndexExpression1,
              arrayIndexExpression2));

      innerLoopBody->append_statement (assignmentStatement);

      SgFortranDo * innerLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              innerLoopInitializationExpression, innerLoopUpperBoundExpression,
              buildIntVal (1), innerLoopBody);

      outerLoopBody->append_statement (innerLoopStatement);
    }
  }

  /*
   * ======================================================
   * Create the outer loop
   * ======================================================
   */

  SgAssignOp * outerLoopInitializationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (0));

  SgSubtractOp * outerLoopUpperBoundExpression = buildSubtractOp (
      variableDeclarations->getReference (OpenMP::numberOfThreads),
      buildIntVal (1));

  SgFortranDo * outerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          outerLoopInitializationExpression, outerLoopUpperBoundExpression,
          buildIntVal (1), outerLoopBody);

  appendStatement (outerLoopStatement, subroutineScope);
}

void
FortranOpenMPHostSubroutine::createReductionPrologueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace ReductionVariableNames;
  using namespace LoopVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating reduction prologue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Create the inner loop
   * ======================================================
   */

  SgBasicBlock * outerLoopBody = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i))
    {
      SgAssignOp * innerLoopInitializationExpression = buildAssignOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (0));

      SgSubtractOp * innerLoopUpperBoundExpression = buildSubtractOp (
          buildIntVal (parallelLoop->getOpDatDimension (i)), buildIntVal (1));

      SgBasicBlock * innerLoopBody = buildBasicBlock ();

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              1)), buildIntVal (64));

      SgAddOp * addExpression = buildAddOp (variableDeclarations->getReference (
          getIterationCounterVariableName (2)), multiplyExpression);

      SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (getReductionArrayHostName (i)),
          addExpression);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayIndexExpression, buildIntVal (0));

      innerLoopBody->append_statement (assignmentStatement);

      SgFortranDo * innerLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              innerLoopInitializationExpression, innerLoopUpperBoundExpression,
              buildIntVal (1), innerLoopBody);

      outerLoopBody->append_statement (innerLoopStatement);
    }
  }

  /*
   * ======================================================
   * Create the outer loop
   * ======================================================
   */

  SgAssignOp * outerLoopInitializationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (0));

  SgSubtractOp * outerLoopUpperBoundExpression = buildSubtractOp (
      variableDeclarations->getReference (OpenMP::numberOfThreads),
      buildIntVal (1));

  SgFortranDo * outerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          outerLoopInitializationExpression, outerLoopUpperBoundExpression,
          buildIntVal (1), outerLoopBody);

  appendStatement (outerLoopStatement, subroutineScope);
}

void
FortranOpenMPHostSubroutine::createReductionDeclarations ()
{
  using namespace SageBuilder;
  using namespace ReductionVariableNames;
  using namespace LoopVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating reduction local variable declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (getIterationCounterVariableName (2),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (2),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (getIterationCounterVariableName (3),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (3),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i))
    {
      string const & variableName = getReductionArrayHostName (i);

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (buildIntVal (64),
          buildIntVal (64));

      SgAddOp * addExpression = buildAddOp (buildIntVal (
          parallelLoop->getOpDatDimension (i)), multiplyExpression);

      SgSubtractOp * subtractExpression = buildSubtractOp (addExpression,
          buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgType * newArrayType =
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              parallelLoop->getOpDatBaseType (i), buildIntVal (0),
              subtractExpression);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, newArrayType, subroutineScope));
    }
  }
}

SgBasicBlock *
FortranOpenMPHostSubroutine::createInitialiseNumberOfThreadsStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise the number of threads",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OpenMP::numberOfThreads),
      OpenMP::createGetMaximumNumberOfThreadsCallStatement (subroutineScope));

  appendStatement (assignmentStatement1, subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (OpenMP::numberOfThreads),
      buildIntVal (1));

  appendStatement (assignmentStatement2, subroutineScope);

  addTextForUnparser (assignmentStatement1, OpenMP::getIfDirectiveString (),
      AstUnparseAttribute::e_before);

  addTextForUnparser (assignmentStatement2, OpenMP::getElseDirectiveString (),
      AstUnparseAttribute::e_before);

  addTextForUnparser (assignmentStatement2, OpenMP::getEndIfDirectiveString (),
      AstUnparseAttribute::e_after);

  return block;
}

SgBasicBlock *
FortranOpenMPHostSubroutine::createTransferOpDatStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2::RunTimeVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to transfer OP_DAT", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  if (parallelLoop->isDirectLoop () == false)
  {
    SgDotExp * dotExpression1 = buildDotExp (
        variableDeclarations->getReference (getOpSetName ()),
        buildOpaqueVarRefExp (Fortran::setPtr, block));

    SgPointerAssignOp * assignExpression1 = new SgPointerAssignOp (
        RoseHelper::getFileInfo (), variableDeclarations->getReference (
            getOpSetCoreName ()), dotExpression1, buildVoidType ());

    assignExpression1->set_endOfConstruct (RoseHelper::getFileInfo ());

    appendStatement (buildExprStatement (assignExpression1), block);

    for (unsigned int i = 1; i
        <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
    {
      SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (getOpMapName (i)),
          buildOpaqueVarRefExp (Fortran::mapPtr, block));

      SgPointerAssignOp * assignExpression = new SgPointerAssignOp (
          RoseHelper::getFileInfo (), variableDeclarations->getReference (
              getOpMapCoreName (i)), dotExpression, buildVoidType ());

      assignExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

      appendStatement (buildExprStatement (assignExpression), block);
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
        getOpDatName (i)), buildOpaqueVarRefExp (Fortran::dataPtr, block));

    SgPointerAssignOp * assignExpression = new SgPointerAssignOp (
        RoseHelper::getFileInfo (), variableDeclarations->getReference (
            getOpDatCoreName (i)), dotExpression, buildVoidType ());

    assignExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

    appendStatement (buildExprStatement (assignExpression), block);
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i) == false)
      {
        SgDotExp * dotExpression = buildDotExp (
            variableDeclarations->getReference (getOpDatCoreName (i)),
            buildOpaqueVarRefExp (set, block));

        SgStatement
            * callStatement =
                FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                    subroutineScope, dotExpression,
                    variableDeclarations->getReference (getOpSetCoreName (i)));

        appendStatement (callStatement, block);
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgDotExp * dotExpression1 = buildDotExp (
          variableDeclarations->getReference (getOpDatCoreName (i)),
          buildOpaqueVarRefExp (dimension, block));

      if (parallelLoop->isReductionRequired (i))
      {
        SgExprStatement * assignmentStatement = buildAssignStatement (
            variableDeclarations->getReference (getOpDatCardinalityName (i)),
            dotExpression1);

        appendStatement (assignmentStatement, block);
      }
      else
      {
        SgDotExp * dotExpression2 = buildDotExp (
            variableDeclarations->getReference (getOpSetCoreName (i)),
            buildOpaqueVarRefExp (size, block));

        SgMultiplyOp * multiplyExpression = buildMultiplyOp (dotExpression1,
            dotExpression2);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            variableDeclarations->getReference (getOpDatCardinalityName (i)),
            multiplyExpression);

        appendStatement (assignmentStatement, block);
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (getOpDatCoreName (i)),
          buildOpaqueVarRefExp (dataOnHost, block));

      SgAggregateInitializer * shapeExpression =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              variableDeclarations->getReference (getOpDatCardinalityName (i)));

      SgStatement
          * callStatement =
              FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                  subroutineScope, dotExpression,
                  variableDeclarations->getReference (getOpDatLocalName (i)),
                  shapeExpression);

      appendStatement (callStatement, block);
    }
  }

  return block;
}

void
FortranOpenMPHostSubroutine::createOpDatLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating variables needed to initialise OP_DAT data",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  if (parallelLoop->isDirectLoop () == false)
  {
    variableDeclarations->add (getOpSetCoreName (),
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            getOpSetCoreName (), buildPointerType (
                FortranTypesBuilder::buildClassDeclaration (OP2::OP_SET_CORE,
                    subroutineScope)->get_type ()), subroutineScope));
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & variableName1 = getOpDatCoreName (i);

    variableDeclarations->add (variableName1,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            variableName1, buildPointerType (
                FortranTypesBuilder::buildClassDeclaration (OP2::OP_DAT_CORE,
                    subroutineScope)->get_type ()), subroutineScope));

    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName2 = getOpDatLocalName (i);

      variableDeclarations->add (variableName2,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName2, buildPointerType (
                  FortranTypesBuilder::getArray_RankOne (
                      parallelLoop->getOpDatBaseType (i))), subroutineScope));

      string const & variableName3 = getOpDatCardinalityName (i);

      variableDeclarations->add (variableName3,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName3, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));

      if (parallelLoop->isReductionRequired (i) == false)
      {
        string const & variableName4 = getOpSetCoreName (i);

        variableDeclarations->add (variableName4,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName4, buildPointerType (
                    FortranTypesBuilder::buildClassDeclaration (
                        OP2::OP_SET_CORE, subroutineScope)->get_type ()),
                subroutineScope));
      }
    }

    if (parallelLoop->isDirectLoop () == false)
    {
      string const & variableName5 = getOpMapCoreName (i);

      variableDeclarations->add (variableName5,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName5, buildPointerType (
                  FortranTypesBuilder::buildClassDeclaration (OP2::OP_MAP_CORE,
                      subroutineScope)->get_type ()), subroutineScope));
    }
  }
}

void
FortranOpenMPHostSubroutine::createOpenMPLocalVariableDeclarations ()
{
  using namespace LoopVariableNames;

  variableDeclarations->add (getIterationCounterVariableName (1),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (OpenMP::numberOfThreads,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::numberOfThreads, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (OpenMP::sliceStart,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::sliceStart, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (OpenMP::sliceEnd,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::sliceEnd, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));
}

FortranOpenMPHostSubroutine::FortranOpenMPHostSubroutine (
    SgScopeStatement * moduleScope, FortranKernelSubroutine * kernelSubroutine,
    FortranParallelLoop * parallelLoop) :
  FortranHostSubroutine (moduleScope, kernelSubroutine, parallelLoop)
{
}
