#include <CPPCUDAKernelSubroutine.h>
#include <CPPCUDAUserSubroutine.h>
#include <CPPReductionSubroutines.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CUDA.h>
#include <CompilerGeneratedNames.h>

void
CPPCUDAKernelSubroutine::createReductionPrologueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating reduction prologue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      if (parallelLoop->isArray (i) || parallelLoop->isPointer (i))
      {
        SgBasicBlock * loopBody = buildBasicBlock ();

        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (getOpDatLocalName (i)),
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)));

        SgExpression * rhsExpression;

        if (isSgTypeInt (parallelLoop->getOpDatBaseType (i)))
        {
          rhsExpression = buildIntVal (0);
        }
        else if (isSgTypeFloat (parallelLoop->getOpDatBaseType (i)))
        {
          rhsExpression = buildFloatVal (0);
        }
        else if (isSgTypeDouble (parallelLoop->getOpDatBaseType (i)))
        {
          rhsExpression = buildDoubleVal (0);
        }
        else
        {
          throw Exceptions::ParallelLoop::UnsupportedBaseTypeException (
              "Reduction type not supported");
        }

        SgExprStatement * assignmentStatement = buildAssignStatement (
            arrayExpression, rhsExpression);

        appendStatement (assignmentStatement, loopBody);

        SgAssignOp * initializationExpression = buildAssignOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (0));

        SgLessThanOp * upperBoundExpression = buildLessThanOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (
                parallelLoop->getOpDatDimension (i)));

        SgPlusPlusOp * strideExpression = buildPlusPlusOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)));

        SgForStatement * forStatement = buildForStatement (buildExprStatement (
            initializationExpression),
            buildExprStatement (upperBoundExpression), strideExpression,
            loopBody);

        appendStatement (forStatement, subroutineScope);
      }
      else
      {
        SgExpression * rhsExpression;

        if (isSgTypeInt (parallelLoop->getOpDatBaseType (i)))
        {
          rhsExpression = buildIntVal (0);
        }
        else if (isSgTypeFloat (parallelLoop->getOpDatBaseType (i)))
        {
          rhsExpression = buildFloatVal (0);
        }
        else if (isSgTypeDouble (parallelLoop->getOpDatBaseType (i)))
        {
          rhsExpression = buildDoubleVal (0);
        }
        else
        {
          throw Exceptions::ParallelLoop::UnsupportedBaseTypeException (
              "Reduction type not supported");
        }

        SgExprStatement * assignmentStatement = buildAssignStatement (
            variableDeclarations->getReference (getOpDatLocalName (i)),
            rhsExpression);

        appendStatement (assignmentStatement, subroutineScope);
      }
    }
  }
}

void
CPPCUDAKernelSubroutine::createReductionEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating reduction epilogue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      if (parallelLoop->isArray (i) || parallelLoop->isPointer (i))
      {
        SgBasicBlock * loopBody = buildBasicBlock ();

        SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
            variableDeclarations->getReference (getOpDatLocalName (i)),
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)));

        SgMultiplyOp * multiplyExpression = buildMultiplyOp (CUDA::getBlockId (
            BLOCK_X, subroutineScope), buildIntVal (
            parallelLoop->getOpDatDimension (i)));

        SgAddOp * addExpression = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), multiplyExpression);

        SgPntrArrRefExp * arrayExpression2 =
            buildPntrArrRefExp (variableDeclarations->getReference (
                getReductionArrayDeviceName (i)), addExpression);

        SgAddressOfOp * addressExpression = buildAddressOfOp (arrayExpression2);

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

        SgExprListExp * actualParameters = buildExprListExp (addressExpression,
            arrayExpression1, reductionType);

        SgFunctionSymbol
            * reductionFunctionSymbol =
                isSgFunctionSymbol (
                    reductionSubroutines->getHeader (
                        parallelLoop->getReductionTuple (i))->get_symbol_from_symbol_table ());

        ROSE_ASSERT (reductionFunctionSymbol != NULL);

        SgFunctionCallExp * reductionFunctionCall = buildFunctionCallExp (
            reductionFunctionSymbol, actualParameters);

        appendStatement (buildExprStatement (reductionFunctionCall), loopBody);

        SgAssignOp * initializationExpression = buildAssignOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (0));

        SgLessThanOp * upperBoundExpression = buildLessThanOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (
                parallelLoop->getOpDatDimension (i)));

        SgPlusPlusOp * strideExpression = buildPlusPlusOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)));

        SgForStatement * forStatement = buildForStatement (buildExprStatement (
            initializationExpression),
            buildExprStatement (upperBoundExpression), strideExpression,
            loopBody);

        appendStatement (forStatement, subroutineScope);
      }
      else
      {
        SgMultiplyOp * multiplyExpression = buildMultiplyOp (CUDA::getBlockId (
            BLOCK_X, subroutineScope), buildIntVal (
            parallelLoop->getOpDatDimension (i)));

        SgPntrArrRefExp * arrayExpression2 =
            buildPntrArrRefExp (variableDeclarations->getReference (
                getReductionArrayDeviceName (i)), multiplyExpression);

        SgAddressOfOp * addressExpression = buildAddressOfOp (arrayExpression2);

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

        SgExprListExp * actualParameters = buildExprListExp (addressExpression,
            variableDeclarations->getReference (getOpDatLocalName (i)),
            reductionType);

        SgFunctionSymbol
            * reductionFunctionSymbol =
                isSgFunctionSymbol (
                    reductionSubroutines->getHeader (
                        parallelLoop->getReductionTuple (i))->get_symbol_from_symbol_table ());

        ROSE_ASSERT (reductionFunctionSymbol != NULL);

        SgFunctionCallExp * reductionFunctionCall = buildFunctionCallExp (
            reductionFunctionSymbol, actualParameters);

        appendStatement (buildExprStatement (reductionFunctionCall),
            subroutineScope);
      }
    }
  }
}

void
CPPCUDAKernelSubroutine::createReductionVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating declarations needed for reduction", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = getOpDatLocalName (i);

      if (parallelLoop->isReductionRequired (i))
      {
        if (parallelLoop->isArray (i) || parallelLoop->isPointer (i))
        {
          variableDeclarations ->add (variableName,
              RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  variableName, buildArrayType (parallelLoop->getOpDatBaseType (
                      i), buildIntVal (parallelLoop->getOpDatDimension (i))),
                  subroutineScope));
        }
        else
        {
          variableDeclarations ->add (variableName,
              RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  variableName, parallelLoop->getOpDatBaseType (i),
                  subroutineScope));
        }
      }
    }
  }
}

CPPCUDAKernelSubroutine::CPPCUDAKernelSubroutine (
    SgScopeStatement * moduleScope, CPPCUDAUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop,
    CPPReductionSubroutines * reductionSubroutines) :
  CPPKernelSubroutine (moduleScope, userSubroutine, parallelLoop)
{
  this->reductionSubroutines = reductionSubroutines;

  subroutineHeaderStatement->get_functionModifier ().setCudaKernel ();
}
