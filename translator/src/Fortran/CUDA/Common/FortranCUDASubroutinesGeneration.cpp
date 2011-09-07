#include <FortranCUDASubroutinesGeneration.h>
#include <FortranCUDAModuleDeclarationsIndirectLoop.h>
#include <FortranCUDAKernelSubroutineDirectLoop.h>
#include <FortranCUDAKernelSubroutineIndirectLoop.h>
#include <FortranCUDAHostSubroutineDirectLoop.h>
#include <FortranCUDAHostSubroutineIndirectLoop.h>
#include <FortranCUDAUserSubroutine.h>
#include <FortranCUDAReductionSubroutine.h>
#include <RoseHelper.h>
#include <Reduction.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

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

  for (map <string, FortranParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    FortranParallelLoop * parallelLoop = it->second;

    parallelLoop->getReductionsNeeded (reductionsNeeded);
  }

  for (vector <Reduction *>::const_iterator it = reductionsNeeded.begin (); it
      != reductionsNeeded.end (); ++it)
  {
    FortranCUDAReductionSubroutine * subroutine =
        new FortranCUDAReductionSubroutine ((*it)->getSubroutineName (),
            moduleScope, *it);

    reductionSubroutines->addSubroutine (*it,
        subroutine->getSubroutineHeaderStatement ());
  }
}

void
FortranCUDASubroutinesGeneration::createSubroutines ()
{
  using std::string;
  using std::map;

  for (map <string, FortranParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    Debug::getInstance ()->debugMessage ("Analysing user subroutine '"
        + userSubroutineName + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    FortranParallelLoop * parallelLoop = it->second;

    FortranCUDAUserSubroutine * userDeviceSubroutine;

    FortranCUDAKernelSubroutine * kernelSubroutine;

    if (parallelLoop->isDirectLoop ())
    {
      userDeviceSubroutine = new FortranCUDAUserSubroutine (userSubroutineName,
          moduleScope, initialiseConstantsSubroutine, declarations,
          parallelLoop);

      kernelSubroutine
          = new FortranCUDAKernelSubroutineDirectLoop (
              userSubroutineName,
              userDeviceSubroutine->getSubroutineName (),
              parallelLoop,
              moduleScope,
              reductionSubroutines,
              static_cast <FortranCUDADataSizesDeclarationDirectLoop *> (dataSizesDeclarations[userSubroutineName]),
              dimensionsDeclarations[userSubroutineName]);

      hostSubroutines[userSubroutineName]
          = new FortranCUDAHostSubroutineDirectLoop (
              userSubroutineName,
              userSubroutineName,
              kernelSubroutine->getSubroutineName (),
              parallelLoop,
              moduleScope,
              initialiseConstantsSubroutine,
              static_cast <FortranCUDADataSizesDeclarationDirectLoop *> (dataSizesDeclarations[userSubroutineName]),
              dimensionsDeclarations[userSubroutineName],
              static_cast <FortranCUDAModuleDeclarations *> (moduleDeclarations[userSubroutineName]));
    }
    else
    {
      userDeviceSubroutine = new FortranCUDAUserSubroutine (userSubroutineName,
          moduleScope, initialiseConstantsSubroutine, declarations,
          parallelLoop);

      kernelSubroutine
          = new FortranCUDAKernelSubroutineIndirectLoop (
              userSubroutineName,
              userDeviceSubroutine->getSubroutineName (),
              parallelLoop,
              moduleScope,
              reductionSubroutines,
              static_cast <FortranCUDADataSizesDeclarationIndirectLoop *> (dataSizesDeclarations[userSubroutineName]),
              dimensionsDeclarations[userSubroutineName]);

      hostSubroutines[userSubroutineName]
          = new FortranCUDAHostSubroutineIndirectLoop (
              userSubroutineName,
              userSubroutineName,
              kernelSubroutine->getSubroutineName (),
              parallelLoop,
              moduleScope,
              initialiseConstantsSubroutine,
              static_cast <FortranCUDADataSizesDeclarationIndirectLoop *> (dataSizesDeclarations[userSubroutineName]),
              dimensionsDeclarations[userSubroutineName],
              static_cast <FortranCUDAModuleDeclarationsIndirectLoop *> (moduleDeclarations[userSubroutineName]));
    }
  }
}

void
FortranCUDASubroutinesGeneration::createModuleDeclarations ()
{
  using std::map;
  using std::string;

  /*
   * ======================================================
   * First declare the type declarations
   * ======================================================
   */

  for (map <string, FortranParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    FortranParallelLoop * parallelLoop = it->second;

    dimensionsDeclarations[userSubroutineName]
        = new FortranOpDatDimensionsDeclaration (userSubroutineName,
            parallelLoop, moduleScope);

    if (parallelLoop->isDirectLoop ())
    {
      dataSizesDeclarations[userSubroutineName]
          = new FortranCUDADataSizesDeclarationDirectLoop (userSubroutineName,
              parallelLoop, moduleScope);
    }
    else
    {
      dataSizesDeclarations[userSubroutineName]
          = new FortranCUDADataSizesDeclarationIndirectLoop (
              userSubroutineName, parallelLoop, moduleScope);
    }
  }

  /*
   * ======================================================
   * Now declare the variables
   * ======================================================
   */

  for (map <string, FortranParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    FortranParallelLoop * parallelLoop = it->second;

    if (parallelLoop->isDirectLoop ())
    {
      moduleDeclarations[userSubroutineName]
          = new FortranCUDAModuleDeclarations (userSubroutineName,
              parallelLoop, moduleScope,
              dataSizesDeclarations[userSubroutineName],
              dimensionsDeclarations[userSubroutineName]);
    }
    else
    {
      moduleDeclarations[userSubroutineName]
          = new FortranCUDAModuleDeclarationsIndirectLoop (
              userSubroutineName,
              parallelLoop,
              moduleScope,
              static_cast <FortranCUDADataSizesDeclarationIndirectLoop *> (dataSizesDeclarations[userSubroutineName]),
              dimensionsDeclarations[userSubroutineName]);
    }
  }
}

void
FortranCUDASubroutinesGeneration::addLibraries ()
{
  using std::string;
  using std::vector;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Adding 'use' statements to CUDA module", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  vector <string> libs;
  libs.push_back (Libraries::ISO_C_BINDING);
  libs.push_back (Libraries::OP2_C);
  libs.push_back (Libraries::cudaConfigurationParams);
  libs.push_back (Libraries::CUDAFOR);

  for (vector <string>::const_iterator it = libs.begin (); it != libs.end (); ++it)
  {
    SgUseStatement* useStatement = new SgUseStatement (
        RoseHelper::getFileInfo (), *it, false);

    useStatement->set_definingDeclaration (useStatement);

    appendStatement (useStatement, moduleScope);
  }
}
