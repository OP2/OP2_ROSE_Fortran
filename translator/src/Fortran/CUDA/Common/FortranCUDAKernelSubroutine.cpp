#include <FortranCUDAKernelSubroutine.h>
#include <CommonNamespaces.h>
#include <FortranCUDAReductionSubroutine.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <Debug.h>
#include <CUDA.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

SgExpression *
FortranCUDAKernelSubroutine::buildOpGlobalActualParameterExpression (
    unsigned int OP_DAT_ArgumentGroup)
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using std::string;

  SgExpression * parameterExpression;

  if (parallelLoop->isRead (OP_DAT_ArgumentGroup))
  {
    if (parallelLoop->isGlobalScalar (OP_DAT_ArgumentGroup))
    {
      Debug::getInstance ()->debugMessage ("OP_GBL with read access (Scalar)",
          Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

      parameterExpression = buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::getOpDatName (OP_DAT_ArgumentGroup)));
    }
    else
    {
      Debug::getInstance ()->debugMessage ("OP_GBL with read access (Array)",
          Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

      string const variableName = OP2::VariableNames::getOpDatSizeName (
          OP_DAT_ArgumentGroup);

      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getDataSizesVariableDeclarationName (
                  userSubroutineName))), buildOpaqueVarRefExp (variableName,
          subroutineScope));

      SgSubtractOp * subtractExpression = buildSubtractOp (dotExpression,
          buildIntVal (1));

      SgSubscriptExpression * subscriptExpression = new SgSubscriptExpression (
          RoseHelper::getFileInfo (), buildIntVal (0), subtractExpression,
          buildIntVal (1));

      subscriptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

      parameterExpression = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::getOpDatName (
              OP_DAT_ArgumentGroup))), subscriptExpression);
    }
  }
  else if (parallelLoop->isIncremented (OP_DAT_ArgumentGroup))
  {
    Debug::getInstance ()->debugMessage ("OP_GBL with increment access",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parameterExpression = buildVarRefExp (variableDeclarations->get (
        OP2::VariableNames::getOpDatName (OP_DAT_ArgumentGroup)));
  }

  return parameterExpression;
}

void
FortranCUDAKernelSubroutine::createInitialiseLocalThreadVariablesStatements ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAddOp;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating initialise local thread variable statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->localThreadVariableDeclarationNeeded (i))
    {
      SgBasicBlock * loopBody = buildBasicBlock ();

      SgExpression * LHSOfAssigment;

      if (parallelLoop->isGlobalScalar (i))
      {
        LHSOfAssigment = buildVarRefExp (variableDeclarations->get (
            OP2::VariableNames::getOpDatLocalName (i)));
      }
      else
      {
        LHSOfAssigment
            = buildPntrArrRefExp (buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getOpDatLocalName (i))), buildVarRefExp (
                variableDeclarations->get (
                    CommonVariableNames::iterationCounter1)));
      }

      if (parallelLoop->isIncremented (i))
      {
        SgExprStatement * assignmentStatement = buildAssignStatement (
            LHSOfAssigment, buildIntVal (0));

        appendStatement (assignmentStatement, loopBody);
      }
      else
      {
        SgMultiplyOp * multiplyExpression = buildMultiplyOp (CUDA::getBlockId (
            BLOCK_X, subroutineScope), buildIntVal (
            parallelLoop->getOpDatDimension (i)));

        SgAddOp * addExpression = buildAddOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), multiplyExpression);

        SgPntrArrRefExp * arrayIndexExpression2 = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getOpDatLocalName (i))), addExpression);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            LHSOfAssigment, arrayIndexExpression2);

        appendStatement (assignmentStatement, loopBody);
      }

      SgExpression * initializationExpression = buildAssignOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter1)),
          buildIntVal (0));

      SgExpression * upperBoundExpression = buildIntVal (
          parallelLoop->getOpDatDimension (i) - 1);

      SgFortranDo * loopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              initializationExpression, upperBoundExpression, buildIntVal (1),
              loopBody);

      appendStatement (loopStatement, subroutineScope);
    }
  }
}

void
FortranCUDAKernelSubroutine::createReductionLoopStatements ()
{
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildVarRefExp;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage ("Adding reduction subroutine call",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      /*
       * ======================================================
       * Create reduction call parameters
       * ======================================================
       */
      SgSubtractOp * subtractExpression1 = buildSubtractOp (CUDA::getBlockId (
          BLOCK_X, subroutineScope), buildIntVal (1));

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
          subtractExpression1, buildIntVal (1));

      SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter1)),
          multiplyExpression1);

      SgAddOp * addExpression2 = buildAddOp (addExpression1, buildIntVal (
          parallelLoop->getOpDatDimension (i) - 1));

      SgSubscriptExpression * subscriptExpression1 = new SgSubscriptExpression (
          RoseHelper::getFileInfo (), addExpression1, addExpression2,
          buildIntVal (1));

      subscriptExpression1->set_endOfConstruct (RoseHelper::getFileInfo ());

      /*
       * ======================================================
       * Index into OP_DAT array
       * ======================================================
       */

      SgPntrArrRefExp * parameterExpression1 = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getOpDatName (i))), subscriptExpression1);

      /*
       * ======================================================
       * Index into local OP_DAT array
       * ======================================================
       */

      SgExpression * parameterExpression2;

      if (parallelLoop->isGlobalScalar (i))
      {
        parameterExpression2 = buildVarRefExp (variableDeclarations->get (
            OP2::VariableNames::getOpDatLocalName (i)));
      }
      else
      {
        parameterExpression2
            = buildPntrArrRefExp (buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getOpDatLocalName (i))), buildVarRefExp (
                variableDeclarations->get (
                    CommonVariableNames::iterationCounter1)));
      }

      /*
       * ======================================================
       * Reduction operation parameter
       * ======================================================
       */

      SgIntVal * reductionType;

      if (parallelLoop->isIncremented (i))
      {
        reductionType = buildIntVal (INCREMENT);
      }
      else if (parallelLoop->isMaximised (i))
      {
        reductionType = buildIntVal (MAX_ACCESS);
      }
      else if (parallelLoop->isMinimised (i))
      {
        reductionType = buildIntVal (MIN_ACCESS);
      }
      else
      {
        Debug::getInstance ()->errorMessage ("Unhandled type of reduction",
            __FILE__, __LINE__);
      }

      SgExprListExp * actualParameters = buildExprListExp (
          parameterExpression1, parameterExpression2, buildVarRefExp (
              variableDeclarations->get (OP2::VariableNames::warpSize)),
          buildVarRefExp (
              variableDeclarations->get (OP2::VariableNames::offset)),
          reductionType);

      /*
       * ======================================================
       * Create reduction function call
       * ======================================================
       */

      SgFunctionSymbol * reductionFunctionSymbol =
          isSgFunctionSymbol (
              reductionSubroutines->getHeader (parallelLoop->getReductionTuple (
                  i))->get_symbol_from_symbol_table ());

      ROSE_ASSERT (reductionFunctionSymbol != NULL);

      SgFunctionCallExp * reductionFunctionCall = buildFunctionCallExp (
          reductionFunctionSymbol, actualParameters);

      /*
       * ======================================================
       * Create loop to repeatedly call reduction subroutine
       * up to the number of dimensions of OP_DAT
       * ======================================================
       */

      SgBasicBlock * loopBody = buildBasicBlock ();

      appendStatement (buildExprStatement (reductionFunctionCall), loopBody);

      SgAssignOp * initializationExpression = buildAssignOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter1)),
          buildIntVal (0));

      SgIntVal * upperBoundExpression = buildIntVal (
          parallelLoop->getOpDatDimension (i) - 1);

      SgFortranDo * loopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              initializationExpression, upperBoundExpression, buildIntVal (1),
              loopBody);

      appendStatement (loopStatement, subroutineScope);
    }
  }
}

void
FortranCUDAKernelSubroutine::createLocalThreadDeclarations ()
{
  using SageBuilder::buildIntVal;
  using SageBuilder::buildArrayType;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildExprListExp;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local thread variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & variableName = OP2::VariableNames::getOpDatLocalName (i);

    if (parallelLoop->localThreadVariableDeclarationNeeded (i))
    {
      if (parallelLoop->isGlobalScalar (i))
      {
        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, parallelLoop->getOpDatBaseType (i),
                subroutineScope));
      }
      else
      {
        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, FortranTypesBuilder::getArray_RankOne (
                    parallelLoop->getOpDatBaseType (i), 0,
                    parallelLoop->getOpDatDimension (i) - 1), subroutineScope));
      }
    }
  }
}

void
FortranCUDAKernelSubroutine::createAutoSharedDeclarations ()
{
  using std::find;
  using std::vector;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating autoshared declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  vector <string> autosharedNames;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isGlobal (i) == false)
      {
        string const autosharedVariableName =
            OP2::VariableNames::getAutosharedDeclarationName (
                parallelLoop->getOpDatBaseType (i),
                parallelLoop->getSizeOfOpDat (i));

        if (find (autosharedNames.begin (), autosharedNames.end (),
            autosharedVariableName) == autosharedNames.end ())
        {
          Debug::getInstance ()->debugMessage (
              "Creating autoshared declaration with name '"
                  + autosharedVariableName + "' for OP_DAT '"
                  + parallelLoop->getOpDatVariableName (i) + "'",
              Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

          SgExpression * upperBound = new SgAsteriskShapeExp (
              RoseHelper::getFileInfo ());

          variableDeclarations->add (
              autosharedVariableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  autosharedVariableName,
                  FortranTypesBuilder::getArray_RankOne (
                      parallelLoop->getOpDatBaseType (i), 0, upperBound),
                  subroutineScope, 1, SHARED));

          autosharedNames.push_back (autosharedVariableName);
        }
      }
    }
  }
}

FortranCUDAKernelSubroutine::FortranCUDAKernelSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    FortranReductionSubroutines * reductionSubroutines,
    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration) :
  FortranKernelSubroutine (subroutineName, userSubroutineName, parallelLoop,
      moduleScope), opDatDimensionsDeclaration (opDatDimensionsDeclaration)
{
  using SageInterface::addTextForUnparser;

  this->reductionSubroutines = reductionSubroutines;

  addTextForUnparser (subroutineHeaderStatement, "attributes(global) ",
      AstUnparseAttribute::e_before);
}
