#include <CPPCUDAKernelSubroutine.h>
#include <CPPCUDAUserSubroutine.h>
#include <CPPCUDAReductionSubroutines.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CUDA.h>
#include <CommonNamespaces.h>

void
CPPCUDAKernelSubroutine::createInitialiseCUDAStageInVariablesStatements ()
{
}

void
CPPCUDAKernelSubroutine::createReductionPrologueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating reduction prologue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      SgBasicBlock * loopBody = buildBasicBlock ();

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (
              OP2::VariableNames::getOpDatLocalName (i)),
          variableDeclarations->getReference (
              CommonVariableNames::iterationCounter1));

      SgExpression * rhsExpression;

      if (isSgTypeInt (parallelLoop->getOpDatBaseType (i)))
      {
        rhsExpression = buildIntVal (0);
      }
      else
      {
        ROSE_ASSERT (isSgTypeFloat(parallelLoop->getOpDatBaseType(i)));

        rhsExpression = buildFloatVal (0);
      }

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression, rhsExpression);

      appendStatement (assignmentStatement, loopBody);

      SgAssignOp * initializationExpression = buildAssignOp (
          variableDeclarations->getReference (
              CommonVariableNames::iterationCounter1), buildIntVal (0));

      SgLessThanOp * upperBoundExpression = buildLessThanOp (
          variableDeclarations->getReference (
              CommonVariableNames::iterationCounter1), buildIntVal (
              parallelLoop->getOpDatDimension (i)));

      SgPlusPlusOp * strideExpression = buildPlusPlusOp (
          variableDeclarations->getReference (
              CommonVariableNames::iterationCounter1));

      SgForStatement * forStatement = buildForStatement (buildExprStatement (
          initializationExpression), buildExprStatement (upperBoundExpression),
          strideExpression, loopBody);

      appendStatement (forStatement, subroutineScope);
    }
  }
}

void
CPPCUDAKernelSubroutine::createReductionEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating reduction epilogue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  Debug::getInstance ()->debugMessage (
      "Creating reduction prologue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      SgBasicBlock * loopBody = buildBasicBlock ();

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
          variableDeclarations->getReference (
              OP2::VariableNames::getOpDatLocalName (i)),
          variableDeclarations->getReference (
              CommonVariableNames::iterationCounter1));

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (CUDA::getBlockId (
          BLOCK_X, subroutineScope), buildIntVal (
          parallelLoop->getOpDatDimension (i)));

      SgAddOp * addExpression = buildAddOp (variableDeclarations->getReference (
          CommonVariableNames::iterationCounter1), multiplyExpression);

      SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
          variableDeclarations->getReference (
              OP2::VariableNames::getReductionArrayDeviceName (i)),
          addExpression);

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

      SgFunctionSymbol * reductionFunctionSymbol =
          isSgFunctionSymbol (
              reductionSubroutines->getHeader (parallelLoop->getReductionTuple (
                  i))->get_symbol_from_symbol_table ());

      ROSE_ASSERT (reductionFunctionSymbol != NULL);

      SgFunctionCallExp * reductionFunctionCall = buildFunctionCallExp (
          reductionFunctionSymbol, actualParameters);

      appendStatement (buildExprStatement (reductionFunctionCall), loopBody);

      SgAssignOp * initializationExpression = buildAssignOp (
          variableDeclarations->getReference (
              CommonVariableNames::iterationCounter1), buildIntVal (0));

      SgLessThanOp * upperBoundExpression = buildLessThanOp (
          variableDeclarations->getReference (
              CommonVariableNames::iterationCounter1), buildIntVal (
              parallelLoop->getOpDatDimension (i)));

      SgPlusPlusOp * strideExpression = buildPlusPlusOp (
          variableDeclarations->getReference (
              CommonVariableNames::iterationCounter1));

      SgForStatement * forStatement = buildForStatement (buildExprStatement (
          initializationExpression), buildExprStatement (upperBoundExpression),
          strideExpression, loopBody);

      appendStatement (forStatement, subroutineScope);
    }
  }
}

void
CPPCUDAKernelSubroutine::createCUDAStageInVariablesVariableDeclarations ()
{
  using namespace SageBuilder;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local thread variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & variableName = OP2::VariableNames::getOpDatLocalName (i);

    if (parallelLoop->isCUDAStageInVariableDeclarationNeeded (i))
    {
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
      else
      {
        variableDeclarations ->add (variableName,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, buildArrayType (
                    parallelLoop->getOpDatBaseType (i), buildIntVal (
                        parallelLoop->getOpDatDimension (i))), subroutineScope));
      }
    }
  }
}

void
CPPCUDAKernelSubroutine::createCUDASharedVariableDeclarations ()
{
  using namespace SageBuilder;
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

          SgVariableDeclaration * autosharedVariableDeclaration =
              RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  autosharedVariableName, buildArrayType (
                      parallelLoop->getOpDatBaseType (i)), subroutineScope);

          autosharedVariableDeclaration->get_declarationModifier ().get_storageModifier ().setCudaDynamicShared ();

          variableDeclarations->add (autosharedVariableName,
              autosharedVariableDeclaration);

          autosharedNames.push_back (autosharedVariableName);

          if (parallelLoop->isDirectLoop ())
          {
            string const autosharedOffsetVariableName =
                OP2::VariableNames::getCUDASharedMemoryOffsetDeclarationName (
                    parallelLoop->getOpDatBaseType (i),
                    parallelLoop->getSizeOfOpDat (i));

            Debug::getInstance ()->debugMessage (
                "Creating offset declaration with name '"
                    + autosharedOffsetVariableName + "' for OP_DAT '"
                    + parallelLoop->getOpDatVariableName (i) + "'",
                Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

            variableDeclarations->add (autosharedOffsetVariableName,
                RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                    autosharedOffsetVariableName, buildIntType (),
                    subroutineScope));
          }
        }
      }
    }
  }
}

CPPCUDAKernelSubroutine::CPPCUDAKernelSubroutine (
    SgScopeStatement * moduleScope, CPPCUDAUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop,
    CPPCUDAReductionSubroutines * reductionSubroutines) :
  CPPKernelSubroutine (moduleScope, userSubroutine, parallelLoop)
{
  this->reductionSubroutines = reductionSubroutines;

  subroutineHeaderStatement->get_functionModifier ().setCudaKernel ();
}
