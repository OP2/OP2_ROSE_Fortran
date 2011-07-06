#include <FortranCUDASubroutinesGeneration.h>
#include <FortranOpDatDimensionsDeclaration.h>
#include <FortranCUDAInitialiseConstantsSubroutine.h>
#include <FortranCUDAKernelSubroutineDirectLoop.h>
#include <FortranCUDAKernelSubroutineIndirectLoop.h>
#include <FortranCUDAHostSubroutineDirectLoop.h>
#include <FortranCUDAHostSubroutineIndirectLoop.h>
#include <FortranCUDAUserSubroutine.h>
#include <RoseHelper.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

FortranHostSubroutine *
FortranCUDASubroutinesGeneration::createSubroutines (
    FortranParallelLoop * parallelLoop, std::string const & userSubroutineName,
    SgScopeStatement * moduleScope)
{
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
            moduleScope, declarations);

    addContains (moduleScope);

    initialiseConstantsSubroutine->generateSubroutine ();

    userDeviceSubroutine = new FortranCUDAUserSubroutine (userSubroutineName,
        moduleScope, initialiseConstantsSubroutine, declarations, parallelLoop);

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
            moduleScope, declarations);

    addContains (moduleScope);

    initialiseConstantsSubroutine->generateSubroutine ();

    userDeviceSubroutine = new FortranCUDAUserSubroutine (userSubroutineName,
        moduleScope, initialiseConstantsSubroutine, declarations, parallelLoop);

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

    useStatement->set_definingDeclaration (moduleStatement);

    appendStatement (useStatement, moduleStatement->get_definition ());
  }
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

FortranCUDASubroutinesGeneration::FortranCUDASubroutinesGeneration (
    SgProject * project,
    FortranProgramDeclarationsAndDefinitions * declarations) :
  FortranSubroutinesGeneration (declarations, ".CUF")
{
  traverseInputFiles (project, preorder);

  unparse ();
}
