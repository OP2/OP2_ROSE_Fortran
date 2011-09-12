#include <FortranCUDAModuleDeclarations.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <FortranCUDAReductionSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

void
FortranCUDAModuleDeclarations::createReductionDeclarations ()
{
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating reduction declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      if (parallelLoop->isRead (i) == false)
      {
        /*
         * ======================================================
         * If the OP_GBL is not read then declare both a host and
         * device array as part of the reduction will be done on
         * the GPU and its results collected on the CPU
         *
         * Note that, if the OP_GBL is read, then the data will be
         * passed to the CUDA and user kernels by reference (if
         * it is an array) or by value (if it is a scalar).
         * Therefore, nothing needs to be allocated on the host or
         * device to store intermediate and final reduction results
         * ======================================================
         */

        SgType * baseType;

        if (parallelLoop->isGlobalScalar (i))
        {
          /*
           * ======================================================
           * If the OP_GBL is a scalar then the base type of the
           * allocated arrays is the primitive type of the OP_GBL
           * ======================================================
           */

          baseType = parallelLoop->getOpDatType (i);
        }
        else
        {
          /*
           * ======================================================
           * If the OP_GBL is an array then the base type of the
           * allocated arrays is the base type of the OP_GBL array
           * ======================================================
           */

          baseType = parallelLoop->getOpDatBaseType (i);
        }

        string const reductionArrayHostName =
            OP2::VariableNames::getReductionArrayHostName (i,
                userSubroutineName);

        Debug::getInstance ()->debugMessage ("Creating host reduction array '"
            + reductionArrayHostName + "'", Debug::HIGHEST_DEBUG_LEVEL,
            __FILE__, __LINE__);

        SgVariableDeclaration * reductionArrayHost =
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                reductionArrayHostName, FortranTypesBuilder::getArray_RankOne (
                    baseType), moduleScope, 1, ALLOCATABLE);

        variableDeclarations->add (reductionArrayHostName, reductionArrayHost);

        string const reductionArrayDeviceName =
            OP2::VariableNames::getReductionArrayDeviceName (i,
                userSubroutineName);

        Debug::getInstance ()->debugMessage (
            "Creating device reduction array '" + reductionArrayDeviceName
                + "'", Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        SgVariableDeclaration * reductionArrayDevice =
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                reductionArrayDeviceName,
                FortranTypesBuilder::getArray_RankOne (baseType), moduleScope,
                2, ALLOCATABLE, DEVICE);

        variableDeclarations->add (reductionArrayDeviceName,
            reductionArrayDevice);
      }
    }
  }
}

void
FortranCUDAModuleDeclarations::createDataSizesDeclaration ()
{
  Debug::getInstance ()->debugMessage (
      "Generating data sizes declaration at module scope",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  std::string const & variableName =
      OP2::VariableNames::getDataSizesVariableDeclarationName (
          userSubroutineName);

  SgVariableDeclaration * variableDeclaration =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName, dataSizesDeclaration->getType (), moduleScope, 1,
          DEVICE);

  variableDeclarations->add (variableName, variableDeclaration);
}

void
FortranCUDAModuleDeclarations::createDimensionsDeclaration ()
{
  std::string const & variableName =
      OP2::VariableNames::getDimensionsVariableDeclarationName (
          userSubroutineName);

  Debug::getInstance ()->debugMessage (
      "Generating OP_DAT dimensions declaration '" + variableName + "'",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVariableDeclaration * variableDeclaration =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
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
FortranCUDAModuleDeclarations::getDataSizesVariableDeclaration ()
{
  std::string const & variableName =
      OP2::VariableNames::getDataSizesVariableDeclarationName (
          userSubroutineName);

  return variableDeclarations->get (variableName);
}

SgVariableDeclaration *
FortranCUDAModuleDeclarations::getDimensionsVariableDeclaration ()
{
  std::string const & variableName =
      OP2::VariableNames::getDimensionsVariableDeclarationName (
          userSubroutineName);

  return variableDeclarations->get (variableName);
}

FortranCUDAModuleDeclarations::FortranCUDAModuleDeclarations (
    std::string const & userSubroutineName, FortranParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope,
    FortranCUDADataSizesDeclaration * dataSizesDeclaration,
    FortranOpDatDimensionsDeclaration * dimensionsDeclaration) :
  FortranModuleDeclarations (userSubroutineName, parallelLoop, moduleScope),
      dataSizesDeclaration (dataSizesDeclaration), dimensionsDeclaration (
          dimensionsDeclaration)
{
  Debug::getInstance ()->debugMessage (
      "Generating CUDA module scope declarations", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createDataSizesDeclaration ();

  createDimensionsDeclaration ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }
}
