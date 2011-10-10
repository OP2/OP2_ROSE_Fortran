#include <FortranCUDAKernelSubroutine.h>
#include <FortranParallelLoop.h>
#include <FortranCUDAModuleDeclarations.h>
#include <FortranReductionSubroutines.h>
#include <FortranCUDAReductionSubroutine.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <Reduction.h>
#include <Debug.h>
#include <CommonNamespaces.h>
#include <CUDA.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

SgExpression *
FortranCUDAKernelSubroutine::createUserKernelOpGlobalActualParameterExpression (
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

      string const variableName = OP2::VariableNames::getOpDatCardinalityName (
          OP_DAT_ArgumentGroup);

      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::opDatCardinalities)),
          buildOpaqueVarRefExp (variableName, subroutineScope));

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
        OP2::VariableNames::getOpDatLocalName (OP_DAT_ArgumentGroup)));
  }
  else if (parallelLoop->isMaximised (OP_DAT_ArgumentGroup))
  {
    Debug::getInstance ()->debugMessage ("OP_GBL with maximum access",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parameterExpression = buildVarRefExp (variableDeclarations->get (
        OP2::VariableNames::getOpDatLocalName (OP_DAT_ArgumentGroup)));
  }
  else if (parallelLoop->isMinimised (OP_DAT_ArgumentGroup))
  {
    Debug::getInstance ()->debugMessage ("OP_GBL with minimum access",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parameterExpression = buildVarRefExp (variableDeclarations->get (
        OP2::VariableNames::getOpDatLocalName (OP_DAT_ArgumentGroup)));
  }

  return parameterExpression;
}

void
FortranCUDAKernelSubroutine::createInitialiseCUDAStageInVariablesStatements ()
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
      "Creating statements to initialise stage in variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isCUDAStageInVariableDeclarationNeeded (i))
    {
      if (parallelLoop->isReductionRequired (i))
      {
        SgExprStatement * assignmentStatement = buildAssignStatement (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getOpDatLocalName (i))), buildIntVal (0));

        appendStatement (assignmentStatement, subroutineScope);
      }
      else
      {
        SgBasicBlock * loopBody = buildBasicBlock ();

        SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getOpDatLocalName (i))), buildVarRefExp (
                variableDeclarations->get (
                    CommonVariableNames::iterationCounter1)));

        if (parallelLoop->isIncremented (i))
        {
          SgExprStatement * assignmentStatement = buildAssignStatement (
              arrayIndexExpression1, buildIntVal (0));

          appendStatement (assignmentStatement, loopBody);
        }
        else
        {
          SgMultiplyOp * multiplyExpression = buildMultiplyOp (
              CUDA::getBlockId (BLOCK_X, subroutineScope), buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgAddOp * addExpression = buildAddOp (
              buildVarRefExp (variableDeclarations->get (
                  CommonVariableNames::iterationCounter1)), multiplyExpression);

          SgPntrArrRefExp * arrayIndexExpression2 = buildPntrArrRefExp (
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::getOpDatLocalName (i))), addExpression);

          SgExprStatement * assignmentStatement = buildAssignStatement (
              arrayIndexExpression1, arrayIndexExpression2);

          appendStatement (assignmentStatement, loopBody);
        }

        SgExpression * initializationExpression = buildAssignOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), buildIntVal (0));

        SgExpression * upperBoundExpression = buildIntVal (
            parallelLoop->getOpDatDimension (i) - 1);

        SgFortranDo * loopStatement =
            FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
                initializationExpression, upperBoundExpression,
                buildIntVal (1), loopBody);

        appendStatement (loopStatement, subroutineScope);
      }
    }
  }
}

void
FortranCUDAKernelSubroutine::createReductionEpilogueStatements ()
{
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
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
       * Index into the reduction array on the device
       * ======================================================
       */

      SgSubtractOp * subtractExpression1 = buildSubtractOp (CUDA::getBlockId (
          BLOCK_X, subroutineScope), buildIntVal (1));

      SgSubtractOp * subtractExpression2 = buildSubtractOp (CUDA::getBlockId (
          BLOCK_X, subroutineScope), buildIntVal (1));

      SgSubscriptExpression * subscriptExpression = new SgSubscriptExpression (
          RoseHelper::getFileInfo (), subtractExpression1, subtractExpression2,
          buildIntVal (1));
      subscriptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

      SgPntrArrRefExp * parameterExpression1 = buildPntrArrRefExp (
          buildVarRefExp (
              moduleDeclarations->getReductionArrayDeviceDeclaration (i)),
          subscriptExpression);

      /*
       * ======================================================
       * The locally computed value to be reduced
       * ======================================================
       */

      SgExpression * parameterExpression2 =
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getOpDatLocalName (i)));

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
        reductionType = buildIntVal (MAXIMUM);
      }
      else if (parallelLoop->isMinimised (i))
      {
        reductionType = buildIntVal (MINIMUM);
      }
      else
      {
        Debug::getInstance ()->errorMessage ("Unhandled type of reduction",
            __FILE__, __LINE__);
      }

      SgExprListExp * actualParameters = buildExprListExp (
          parameterExpression1, parameterExpression2, buildVarRefExp (
              variableDeclarations->get (OP2::VariableNames::warpSize)),
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

      appendStatement (buildExprStatement (reductionFunctionCall),
          subroutineScope);
    }
  }
}

void
FortranCUDAKernelSubroutine::createCUDAStageInVariablesVariableDeclarations ()
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

    if (parallelLoop->isCUDAStageInVariableDeclarationNeeded (i))
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
FortranCUDAKernelSubroutine::createCUDASharedVariableDeclarations ()
{
  using std::find;
  using std::vector;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA shared variable declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

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
              "Creating declaration with name '" + autosharedVariableName
                  + "' for OP_DAT '" + parallelLoop->getOpDatVariableName (i)
                  + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

          SgExpression * upperBound = new SgAsteriskShapeExp (
              RoseHelper::getFileInfo ());

          variableDeclarations->add (
              autosharedVariableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  autosharedVariableName,
                  FortranTypesBuilder::getArray_RankOne (
                      parallelLoop->getOpDatBaseType (i), 0, upperBound),
                  subroutineScope, 1, CUDA_SHARED));

          autosharedNames.push_back (autosharedVariableName);

          string const autosharedOffsetVariableName =
              OP2::VariableNames::getAutosharedOffsetDeclarationName (
                  parallelLoop->getOpDatBaseType (i),
                  parallelLoop->getSizeOfOpDat (i));

          Debug::getInstance ()->debugMessage (
              "Creating offset declaration with name '"
                  + autosharedOffsetVariableName + "' for OP_DAT '"
                  + parallelLoop->getOpDatVariableName (i) + "'",
              Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

          variableDeclarations->add (
              autosharedOffsetVariableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  autosharedOffsetVariableName,
                  FortranTypesBuilder::getFourByteInteger (), subroutineScope));
        }
      }
    }
  }
}

FortranCUDAKernelSubroutine::FortranCUDAKernelSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    FortranReductionSubroutines * reductionSubroutines,
    FortranCUDAOpDatCardinalitiesDeclaration * dataSizesDeclaration,
    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration,
    FortranCUDAModuleDeclarations * moduleDeclarations) :
  FortranKernelSubroutine (subroutineName, userSubroutineName, parallelLoop,
      moduleScope), dataSizesDeclaration (dataSizesDeclaration),
      opDatDimensionsDeclaration (opDatDimensionsDeclaration),
      moduleDeclarations (moduleDeclarations)
{
  this->reductionSubroutines = reductionSubroutines;

  subroutineHeaderStatement->get_functionModifier ().setCudaKernel ();
}
