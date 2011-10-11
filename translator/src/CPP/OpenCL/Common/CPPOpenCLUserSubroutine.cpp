#include <CPPOpenCLUserSubroutine.h>
#include <CPPProgramDeclarationsAndDefinitions.h>
#include <CPPParallelLoop.h>

void
CPPOpenCLUserSubroutine::patchReferencesToConstants ()
{
}

void
CPPOpenCLUserSubroutine::createStatements ()
{
}

void
CPPOpenCLUserSubroutine::createLocalVariableDeclarations ()
{
}

void
CPPOpenCLUserSubroutine::createFormalParameterDeclarations ()
{
}

CPPOpenCLUserSubroutine::CPPOpenCLUserSubroutine (
    SgScopeStatement * moduleScope, CPPParallelLoop * parallelLoop,
    CPPProgramDeclarationsAndDefinitions * declarations) :
      UserSubroutine <SgFunctionDeclaration,
          CPPProgramDeclarationsAndDefinitions> (parallelLoop, declarations)
{
  using SageBuilder::buildDefiningFunctionDeclaration;
  using SageBuilder::buildVoidType;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  subroutineHeaderStatement = buildDefiningFunctionDeclaration (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      moduleScope);

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineHeaderStatement->get_functionModifier ().setInline ();

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  createStatements ();

  patchReferencesToConstants ();
}
