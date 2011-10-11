#include <CPPOpenCLModuleDeclarations.h>
#include <CPPOpenCLReductionSubroutine.h>
#include <CPPParallelLoop.h>
#include <CPPOpDatDimensionsDeclaration.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>

void
CPPOpenCLModuleDeclarations::createReductionDeclarations ()
{
}

SgVariableDeclaration *
CPPOpenCLModuleDeclarations::getReductionArrayHostVariableDeclaration ()
{
  return NULL;
}

SgVariableDeclaration *
CPPOpenCLModuleDeclarations::getReductionArrayDeviceVariableDeclaration ()
{
  return NULL;
}

CPPOpenCLModuleDeclarations::CPPOpenCLModuleDeclarations (
    std::string const & userSubroutineName, CPPParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  CPPModuleDeclarations (userSubroutineName, parallelLoop, moduleScope)
{
  Debug::getInstance ()->debugMessage (
      "Generating OpenCL module scope declarations", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }
}
