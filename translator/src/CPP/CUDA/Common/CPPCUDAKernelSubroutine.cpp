#include <CPPCUDAKernelSubroutine.h>
#include <CPPCUDAUserSubroutine.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>

void
CPPCUDAKernelSubroutine::createInitialiseCUDAStageInVariablesStatements ()
{
}

void
CPPCUDAKernelSubroutine::createReductionPrologueStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildFloatVal;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildForStatement;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildPlusPlusOp;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAssignOp;
  using SageInterface::appendStatement;
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
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getOpDatLocalName (i))),
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter1)));

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

      SgAssignOp * initializationExpression = buildAssignOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter1)),
          buildIntVal (0));

      SgLessThanOp * upperBoundExpression = buildLessThanOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter1)),
          buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgPlusPlusOp * strideExpression = buildPlusPlusOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter1)));

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
  Debug::getInstance ()->debugMessage (
      "Creating reduction epilogue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildFloatVal;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildForStatement;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildPlusPlusOp;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAssignOp;
  using SageInterface::appendStatement;
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
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getOpDatLocalName (i))),
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter1)));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression, buildIntVal (1));

      appendStatement (assignmentStatement, loopBody);

      SgAssignOp * initializationExpression = buildAssignOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter1)),
          buildIntVal (0));

      SgLessThanOp * upperBoundExpression = buildLessThanOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter1)),
          buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgPlusPlusOp * strideExpression = buildPlusPlusOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter1)));

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
  using SageBuilder::buildIntVal;
  using SageBuilder::buildArrayType;
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
  using SageBuilder::buildIntType;
  using SageBuilder::buildArrayType;
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

          SgVariableDeclaration * autosharedVariableDeclaration =
              RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  autosharedVariableName, buildArrayType (
                      parallelLoop->getOpDatBaseType (i)), subroutineScope);

          autosharedVariableDeclaration->get_declarationModifier ().get_storageModifier ().setCudaDynamicShared ();

          variableDeclarations->add (autosharedVariableName,
              autosharedVariableDeclaration);

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

          variableDeclarations->add (autosharedOffsetVariableName,
              RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  autosharedOffsetVariableName, buildIntType (),
                  subroutineScope));
        }
      }
    }
  }
}

CPPCUDAKernelSubroutine::CPPCUDAKernelSubroutine (
    SgScopeStatement * moduleScope, CPPCUDAUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop) :
  CPPKernelSubroutine (moduleScope, userSubroutine, parallelLoop)
{
  subroutineHeaderStatement->get_functionModifier ().setCudaKernel ();
}
