#include <CPPCUDAUserSubroutine.h>
#include <CPPParallelLoop.h>
#include <CPPProgramDeclarationsAndDefinitions.h>

void
CPPCUDAUserSubroutine::forceOutputOfCodeToFile ()
{
}

void
CPPCUDAUserSubroutine::findOriginalSubroutine ()
{
}

void
CPPCUDAUserSubroutine::createStatements ()
{
}

void
CPPCUDAUserSubroutine::createLocalVariableDeclarations ()
{
}

void
CPPCUDAUserSubroutine::createFormalParameterDeclarations ()
{
}

CPPCUDAUserSubroutine::CPPCUDAUserSubroutine (SgScopeStatement * moduleScope,
    CPPParallelLoop * parallelLoop,
    CPPProgramDeclarationsAndDefinitions * declarations) :
      UserSubroutine <SgFunctionDeclaration,
          CPPProgramDeclarationsAndDefinitions> (parallelLoop, declarations)
{
  using SageBuilder::buildDefiningFunctionDeclaration;
  using SageBuilder::buildVoidType;
  using SageInterface::appendStatement;

  subroutineHeaderStatement = buildDefiningFunctionDeclaration (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      moduleScope);

  subroutineHeaderStatement->get_functionModifier ().setCudaDevice ();

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  findOriginalSubroutine ();

  createStatements ();

  forceOutputOfCodeToFile ();
}
