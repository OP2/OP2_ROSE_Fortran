#include <CPPOpenCLModuleDeclarations.h>
#include <CPPOpenCLReductionSubroutine.h>
#include <CPPParallelLoop.h>
#include <CPPOpenCLDataSizesDeclaration.h>
#include <CPPOpDatDimensionsDeclaration.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

void
CPPOpenCLModuleDeclarations::createReductionDeclarations ()
{
  using std::vector;

  vector <Reduction *> reductions;

  parallelLoop->getReductionsNeeded (reductions);

  for (vector <Reduction *>::iterator it = reductions.begin (); it
      != reductions.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration1 =
        RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
            ReductionSubroutine::reductionArrayHost, (*it)->getBaseType (),
            moduleScope);

    variableDeclarations->add (ReductionSubroutine::reductionArrayHost,
        variableDeclaration1);

    SgVariableDeclaration * variableDeclaration2 =
        RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
            ReductionSubroutine::reductionArrayDevice, (*it)->getBaseType (),
            moduleScope);

    variableDeclarations->add (ReductionSubroutine::reductionArrayDevice,
        variableDeclaration2);
  }
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

SgVariableDeclaration *
CPPOpenCLModuleDeclarations::getReductionArrayHostVariableDeclaration ()
{
  return variableDeclarations->get (ReductionSubroutine::reductionArrayHost);
}

SgVariableDeclaration *
CPPOpenCLModuleDeclarations::getReductionArrayDeviceVariableDeclaration ()
{
  return variableDeclarations->get (ReductionSubroutine::reductionArrayDevice);
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
