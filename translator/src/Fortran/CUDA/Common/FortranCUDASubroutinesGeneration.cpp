#include "FortranCUDASubroutinesGeneration.h"
#include "FortranCUDAKernelSubroutineDirectLoop.h"
#include "FortranCUDAKernelSubroutineIndirectLoop.h"
#include "FortranCUDAHostSubroutineDirectLoop.h"
#include "FortranCUDAHostSubroutineIndirectLoop.h"
#include "FortranCUDAUserSubroutine.h"
#include "FortranCUDAReductionSubroutine.h"
#include "FortranReductionSubroutines.h"
#include "FortranCUDAModuleDeclarations.h"
#include "FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop.h"
#include "FortranOpDatDimensionsDeclaration.h"
#include "FortranParallelLoop.h"
#include "FortranProgramDeclarationsAndDefinitions.h"
#include "FortranTypesBuilder.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "FortranCUDAConstantDeclarations.h"
#include "OP2Definitions.h"
#include "RoseHelper.h"
#include "Reduction.h"
#include "Exceptions.h"
#include "OP2.h"
#include "CUDA.h"
#include <boost/algorithm/string.hpp>

void
FortranCUDASubroutinesGeneration::createReductionSubroutines ()
{
  using boost::lexical_cast;
  using std::string;
  using std::map;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Creating reduction subroutines",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  vector <Reduction *> reductionsNeeded;

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    ParallelLoop * parallelLoop = it->second;

    parallelLoop->getReductionsNeeded (reductionsNeeded);
  }

  for (vector <Reduction *>::const_iterator it = reductionsNeeded.begin (); it
      != reductionsNeeded.end (); ++it)
  {
    FortranCUDAReductionSubroutine * subroutine =
        new FortranCUDAReductionSubroutine (moduleScope, *it);

    reductionSubroutines->addSubroutine (*it,
        subroutine->getSubroutineHeaderStatement ());
  }
}

void
FortranCUDASubroutinesGeneration::createSubroutines ()
{
  using std::string;
  using std::map;

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    Debug::getInstance ()->debugMessage ("Analysing user subroutine '"
        + userSubroutineName + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    FortranParallelLoop * parallelLoop =
        static_cast <FortranParallelLoop *> (it->second);

    FortranCUDAUserSubroutine * userDeviceSubroutine =
        new FortranCUDAUserSubroutine (moduleScope, parallelLoop, declarations,
            CUDAconstants);

    FortranCUDAKernelSubroutine * kernelSubroutine;

    if (parallelLoop->isDirectLoop ())
    {
      kernelSubroutine
          = new FortranCUDAKernelSubroutineDirectLoop (
              moduleScope,
              userDeviceSubroutine,
              parallelLoop,
              reductionSubroutines,
              cardinalitiesDeclarations[userSubroutineName],
              dimensionsDeclarations[userSubroutineName],
              static_cast <FortranCUDAModuleDeclarations *> (moduleDeclarations[userSubroutineName]));

      hostSubroutines[userSubroutineName]
          = new FortranCUDAHostSubroutineDirectLoop (
              moduleScope,
              kernelSubroutine,
              parallelLoop,
              cardinalitiesDeclarations[userSubroutineName],
              dimensionsDeclarations[userSubroutineName],
              static_cast <FortranCUDAModuleDeclarations *> (moduleDeclarations[userSubroutineName]));
    }
    else
    {
      kernelSubroutine
          = new FortranCUDAKernelSubroutineIndirectLoop (
              moduleScope,
              userDeviceSubroutine,
              parallelLoop,
              reductionSubroutines,
              static_cast <FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop *> (cardinalitiesDeclarations[userSubroutineName]),
              dimensionsDeclarations[userSubroutineName],
              static_cast <FortranCUDAModuleDeclarations *> (moduleDeclarations[userSubroutineName]));

      hostSubroutines[userSubroutineName]
          = new FortranCUDAHostSubroutineIndirectLoop (
              moduleScope,
              kernelSubroutine,
              parallelLoop,
              static_cast <FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop *> (cardinalitiesDeclarations[userSubroutineName]),
              dimensionsDeclarations[userSubroutineName],
              static_cast <FortranCUDAModuleDeclarations *> (moduleDeclarations[userSubroutineName]));
    }
  }
}

void
FortranCUDASubroutinesGeneration::createModuleDeclarations ()
{
  using std::map;
  using std::string;

  CUDAconstants = new FortranCUDAConstantDeclarations (declarations,
      moduleScope);

  /*
   * ======================================================
   * First declare the type declarations
   * ======================================================
   */

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    Debug::getInstance ()->debugMessage ("Analysing '" + userSubroutineName
        + "'", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    FortranParallelLoop * parallelLoop =
        static_cast <FortranParallelLoop *> (it->second);

    dimensionsDeclarations[userSubroutineName]
        = new FortranOpDatDimensionsDeclaration (parallelLoop, moduleScope);

    if (parallelLoop->isDirectLoop ())
    {
      cardinalitiesDeclarations[userSubroutineName]
          = new FortranCUDAOpDatCardinalitiesDeclaration (parallelLoop,
              moduleScope);
    }
    else
    {
      cardinalitiesDeclarations[userSubroutineName]
          = new FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop (
              parallelLoop, moduleScope);
    }
  }

  /*
   * ======================================================
   * Now declare the variables
   * ======================================================
   */

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    Debug::getInstance ()->debugMessage ("Analysing '" + userSubroutineName
        + "'", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    FortranParallelLoop * parallelLoop =
        static_cast <FortranParallelLoop *> (it->second);

    moduleDeclarations[userSubroutineName] = new FortranCUDAModuleDeclarations (
        parallelLoop, moduleScope,
        cardinalitiesDeclarations[userSubroutineName],
        dimensionsDeclarations[userSubroutineName]);
  }
}

void
FortranCUDASubroutinesGeneration::addLibraries ()
{
  using namespace SageInterface;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Adding 'use' statements to CUDA module", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  vector <string> libs;

  libs.push_back (OP2::Libraries::Fortran::declarations);
  libs.push_back (OP2::Libraries::Fortran::CBindings);
  libs.push_back (CUDA::Libraries::Fortran::CUDARuntimeSupport);
  libs.push_back (CUDA::Libraries::Fortran::CUDALaunchParameters);

  for (vector <string>::const_iterator it = libs.begin (); it != libs.end (); ++it)
  {
    SgUseStatement* useStatement = new SgUseStatement (
        RoseHelper::getFileInfo (), *it, false);

    useStatement->set_definingDeclaration (useStatement);

    appendStatement (useStatement, moduleScope);
  }
}
