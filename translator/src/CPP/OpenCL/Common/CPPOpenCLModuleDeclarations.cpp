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

void
CPPOpenCLModuleDeclarations::createDataSizesDeclaration ()
{
  Debug::getInstance ()->debugMessage (
      "Generating data sizes declaration at module scope",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  std::string const & variableName =
      OP2::VariableNames::getDataSizesVariableDeclarationName (
          userSubroutineName);

  SgVariableDeclaration * variableDeclaration =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName, dataSizesDeclaration->getType (), moduleScope);

  variableDeclarations->add (variableName, variableDeclaration);
}

void
CPPOpenCLModuleDeclarations::createDimensionsDeclaration ()
{
  Debug::getInstance ()->debugMessage (
      "Generating OP_DAT dimensions declaration at module scope",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  std::string const & variableName =
      OP2::VariableNames::getDimensionsVariableDeclarationName (
          userSubroutineName);

  SgVariableDeclaration * variableDeclaration =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName, dimensionsDeclaration->getType (), moduleScope);

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
      OP2::VariableNames::getDataSizesVariableDeclarationName (
          userSubroutineName);

  return variableDeclarations->get (variableName);
}

SgVariableDeclaration *
CPPOpenCLModuleDeclarations::getDimensionsVariableDeclaration ()
{
  std::string const & variableName =
      OP2::VariableNames::getDimensionsVariableDeclarationName (
          userSubroutineName);

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
