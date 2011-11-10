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
#include <CompilerGeneratedNames.h>
#include <CUDA.h>

void
FortranCUDAKernelSubroutine::createReductionEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
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
          variableDeclarations->getReference (getReductionArrayDeviceName (i)),
          subscriptExpression);

      /*
       * ======================================================
       * The locally computed value to be reduced
       * ======================================================
       */

      SgExpression * parameterExpression2 = variableDeclarations->getReference (
          getOpDatLocalName (i));

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

      /*
       * ======================================================
       * Create reduction function call
       * ======================================================
       */

      SgExprListExp * actualParameters = buildExprListExp (
          parameterExpression1, parameterExpression2,
          variableDeclarations->getReference (warpSize), reductionType);

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
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local thread variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & variableName = getOpDatLocalName (i);

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
  using namespace OP2VariableNames;
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
      if (parallelLoop->isIndirect (i) || (parallelLoop->isDirect (i)
          && parallelLoop->getOpDatDimension (i) > 1))
      {
        string const autosharedVariableName =
            getCUDASharedMemoryDeclarationName (parallelLoop->getOpDatBaseType (
                i), parallelLoop->getSizeOfOpDat (i));

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
              getCUDASharedMemoryOffsetDeclarationName (
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
