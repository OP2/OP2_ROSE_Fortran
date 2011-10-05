#include <CPPOpenCLUserSubroutine.h>
#include <CPPProgramDeclarationsAndDefinitions.h>
#include <CPPParallelLoop.h>
#include <CPPOpenCLStatementsAndExpressionsBuilder.h>
#include <boost/algorithm/string/predicate.hpp>
#include <Debug.h>
#include <algorithm>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

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

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

CPPOpenCLUserSubroutine::CPPOpenCLUserSubroutine (
    std::string const & subroutineName, SgScopeStatement * moduleScope,
    CPPProgramDeclarationsAndDefinitions * declarations,
    CPPParallelLoop * parallelLoop) :
      UserSubroutine <SgFunctionDeclaration,
          CPPProgramDeclarationsAndDefinitions> (subroutineName, declarations,
          parallelLoop)
//initialiseConstantsSubroutine( initialiseConstantsSubroutine )
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
