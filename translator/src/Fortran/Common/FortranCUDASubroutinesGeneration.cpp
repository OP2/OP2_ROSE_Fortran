/*
 * Written by Adam Betts and Carlo Bertolli
 */

#include <FortranCUDASubroutinesGeneration.h>
#include <FortranOpDatDimensionsDeclaration.h>
#include <FortranCUDAInitialiseConstantsSubroutine.h>
#include <FortranCUDAKernelSubroutineDirectLoop.h>
#include <FortranCUDAKernelSubroutineIndirectLoop.h>
#include <FortranCUDAHostSubroutineDirectLoop.h>
#include <FortranCUDAHostSubroutineIndirectLoop.h>
#include <FortranCUDAUserSubroutine.h>
#include <ROSEHelper.h>

FortranHostSubroutine *
FortranCUDASubroutinesGeneration::createSubroutines (
    FortranParallelLoop * parallelLoop, std::string const & userSubroutineName,
    SgModuleStatement * moduleStatement)
{
  SgScopeStatement * moduleScope = moduleStatement->get_definition ();

  /*
   * ======================================================
   * Create the type representing the dimensions of each
   * OP_DAT
   * ======================================================
   */

  FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration =
      new FortranOpDatDimensionsDeclaration (userSubroutineName, parallelLoop,
          moduleScope);

  if (parallelLoop->isReductionRequired ())
  {
    parallelLoop->generateReductionSubroutines (moduleScope);
  }

  FortranCUDAUserSubroutine * userDeviceSubroutine;

  FortranCUDAKernelSubroutine * kernelSubroutine;

  FortranCUDAHostSubroutine * hostSubroutine;

  if (parallelLoop->isDirectLoop ())
  {
    /*
     * ======================================================
     * Direct loop
     * ======================================================
     */
    FortranCUDADataSizesDeclarationDirectLoop * dataSizesDeclaration =
        new FortranCUDADataSizesDeclarationDirectLoop (userSubroutineName,
            parallelLoop, moduleScope);

    FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine =
        new FortranCUDAInitialiseConstantsSubroutine (userSubroutineName,
            moduleScope);

    addContains (moduleStatement);

    initialiseConstantsSubroutine->generateSubroutine ();

    userDeviceSubroutine = new FortranCUDAUserSubroutine (userSubroutineName,
        initialiseConstantsSubroutine, declarations, parallelLoop, moduleScope);

    kernelSubroutine = new FortranCUDAKernelSubroutineDirectLoop (
        userSubroutineName, userDeviceSubroutine->getSubroutineName (),
        parallelLoop, moduleScope, dataSizesDeclaration,
        opDatDimensionsDeclaration);

    hostSubroutine = new FortranCUDAHostSubroutineDirectLoop (
        userSubroutineName, userDeviceSubroutine->getSubroutineName (),
        kernelSubroutine->getSubroutineName (), parallelLoop, moduleScope,
        initialiseConstantsSubroutine, dataSizesDeclaration,
        opDatDimensionsDeclaration);
  }
  else
  {
    /*
     * ======================================================
     * Indirect loop
     * ======================================================
     */

    FortranCUDADataSizesDeclarationIndirectLoop * dataSizesDeclaration =
        new FortranCUDADataSizesDeclarationIndirectLoop (userSubroutineName,
            parallelLoop, moduleScope);

    FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine =
        new FortranCUDAInitialiseConstantsSubroutine (userSubroutineName,
            moduleScope);

    addContains (moduleStatement);

    initialiseConstantsSubroutine->generateSubroutine ();

    userDeviceSubroutine = new FortranCUDAUserSubroutine (userSubroutineName,
        initialiseConstantsSubroutine, declarations, parallelLoop, moduleScope);

    kernelSubroutine = new FortranCUDAKernelSubroutineIndirectLoop (
        userSubroutineName, userDeviceSubroutine->getSubroutineName (),
        parallelLoop, moduleScope, dataSizesDeclaration,
        opDatDimensionsDeclaration);

    hostSubroutine = new FortranCUDAHostSubroutineIndirectLoop (
        userSubroutineName, userDeviceSubroutine->getSubroutineName (),
        kernelSubroutine->getSubroutineName (), parallelLoop, moduleScope,
        initialiseConstantsSubroutine, dataSizesDeclaration,
        opDatDimensionsDeclaration);
  }

  return hostSubroutine;
}

void
FortranCUDASubroutinesGeneration::addLibraries (
    SgModuleStatement * moduleStatement)
{
  using std::string;
  using std::vector;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Adding 'use' statements to CUDA module", 2);

  vector <string> libs;
  libs.push_back (Libraries::ISO_C_BINDING);
  libs.push_back (Libraries::OP2_C);
  libs.push_back (Libraries::cudaConfigurationParams);
  libs.push_back (Libraries::CUDAFOR);

  for (vector <string>::const_iterator it = libs.begin (); it != libs.end (); ++it)
  {
    SgUseStatement* useStatement = new SgUseStatement (
        ROSEHelper::getFileInfo (), *it, false);

    useStatement->set_definingDeclaration (moduleStatement);

    appendStatement (useStatement, moduleStatement->get_definition ());
  }
}