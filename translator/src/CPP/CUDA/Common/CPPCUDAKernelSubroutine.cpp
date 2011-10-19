#include <CPPCUDAKernelSubroutine.h>
#include <CPPCUDAUserSubroutine.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>

void
CPPCUDAKernelSubroutine::createInitialiseCUDAStageInVariablesStatements ()
{
}

void
CPPCUDAKernelSubroutine::createReductionEpilogueStatements ()
{
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
      if (parallelLoop->isGlobal (i) && parallelLoop->isArray (i) == false)
      {
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
