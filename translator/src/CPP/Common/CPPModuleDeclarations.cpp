#include <CPPModuleDeclarations.h>
#include <ScopedVariableDeclarations.h>
#include <Debug.h>
#include <CommonNamespaces.h>
#include <rose.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

CPPModuleDeclarations::CPPModuleDeclarations (
    std::string const & userSubroutineName, CPPParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  userSubroutineName (userSubroutineName), parallelLoop (parallelLoop),
      moduleScope (moduleScope)
{
  variableDeclarations = new ScopedVariableDeclarations ();
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

SgVariableDeclaration *
CPPModuleDeclarations::getGlobalOpDatDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  return variableDeclarations->get (OP2::VariableNames::getOpDatGlobalName (
      OP_DAT_ArgumentGroup));
}

ScopedVariableDeclarations *
CPPModuleDeclarations::getAllDeclarations ()
{
  return variableDeclarations;
}
