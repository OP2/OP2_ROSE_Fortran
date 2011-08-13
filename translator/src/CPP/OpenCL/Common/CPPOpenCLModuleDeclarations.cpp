#include <CPPOpenCLModuleDeclarations.h>
#include <CPPStatementsAndExpressionsBuilder.h>
#include <CPPOpenCLReductionSubroutine.h>

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
        CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
            ReductionSubroutine::reductionArrayHost, (*it)->getArrayType (),
            moduleScope, 1, ALLOCATABLE);

    variableDeclarations->add (ReductionSubroutine::reductionArrayHost,
        variableDeclaration1);

    SgVariableDeclaration * variableDeclaration2 =
        CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
            ReductionSubroutine::reductionArrayDevice, (*it)->getArrayType (),
            moduleScope, 2, ALLOCATABLE, DEVICE);

    variableDeclarations->add (ReductionSubroutine::reductionArrayDevice,
        variableDeclaration2);
  }
}

void
CPPOpenCLModuleDeclarations::createDataSizesDeclaration ()
{
  Debug::getInstance ()->debugMessage (
      "Generating data sizes declaration at module scope",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  std::string const & variableName =
      VariableNames::getDataSizesVariableDeclarationName (userSubroutineName);

  SgVariableDeclaration * variableDeclaration =
      CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName, dataSizesDeclaration->getType (), moduleScope, 1,
          DEVICE);

  variableDeclarations->add (variableName, variableDeclaration);
}

void
CPPOpenCLModuleDeclarations::createDimensionsDeclaration ()
{
  Debug::getInstance ()->debugMessage (
      "Generating OP_DAT dimensions declaration at module scope",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  std::string const & variableName =
      VariableNames::getDimensionsVariableDeclarationName (userSubroutineName);

  SgVariableDeclaration * variableDeclaration =
      CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName, dimensionsDeclaration->getType (), moduleScope, 1,
          DEVICE);

  variableDeclarations->add (variableName, variableDeclaration);
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

SgVariableDeclaration *
CPPOpenCLModuleDeclarations::getDataSizesVariableDeclaration ()
{
  std::string const & variableName =
      VariableNames::getDataSizesVariableDeclarationName (userSubroutineName);

  return variableDeclarations->get (variableName);
}

SgVariableDeclaration *
CPPOpenCLModuleDeclarations::getDimensionsVariableDeclaration ()
{
  std::string const & variableName =
      VariableNames::getDimensionsVariableDeclarationName (userSubroutineName);

  return variableDeclarations->get (variableName);
}

CPPOpenCLModuleDeclarations::CPPOpenCLModuleDeclarations (
    std::string const & userSubroutineName, CPPParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope,
    CPPOpenCLDataSizesDeclaration * dataSizesDeclaration,
    CPPOpDatDimensionsDeclaration * dimensionsDeclaration) :
  CPPModuleDeclarations (userSubroutineName, parallelLoop, moduleScope),
      dataSizesDeclaration (dataSizesDeclaration), dimensionsDeclaration (
          dimensionsDeclaration)
{
  Debug::getInstance ()->debugMessage (
      "Generating OpenCL module scope declarations", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createDataSizesDeclaration ();

  createDimensionsDeclaration ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }
}
