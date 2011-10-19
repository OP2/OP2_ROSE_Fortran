#include <FortranCUDAKernelSubroutine.h>
#include <FortranCUDAUserSubroutine.h>
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
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getReductionArrayDeviceName (i))),
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

      ROSE_ASSERT (reductionType != NULL);

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
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local thread variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & variableName = OP2::VariableNames::getOpDatLocalName (i);

    if (parallelLoop->isCUDAStageInVariableDeclarationNeeded (i))
    {
      if (parallelLoop->isGlobal (i) && parallelLoop->isArray (i) == false)
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
            OP2::VariableNames::getCUDASharedMemoryDeclarationName (
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
              OP2::VariableNames::getCUDASharedMemoryOffsetDeclarationName (
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
    SgScopeStatement * moduleScope, FortranCUDAUserSubroutine * userSubroutine,
    FortranParallelLoop * parallelLoop,
    FortranReductionSubroutines * reductionSubroutines,
    FortranCUDAOpDatCardinalitiesDeclaration * cardinalitiesDeclaration,
    FortranOpDatDimensionsDeclaration * dimensionsDeclaration,
    FortranCUDAModuleDeclarations * moduleDeclarations) :
  FortranKernelSubroutine (moduleScope, userSubroutine, parallelLoop),
      cardinalitiesDeclaration (cardinalitiesDeclaration),
      dimensionsDeclaration (dimensionsDeclaration), moduleDeclarations (
          moduleDeclarations)
{
  this->reductionSubroutines = reductionSubroutines;

  subroutineHeaderStatement->get_functionModifier ().setCudaKernel ();
}
