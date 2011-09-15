#include <FortranCUDASubroutinesGeneration.h>
#include <FortranCUDAModuleDeclarationsIndirectLoop.h>
#include <FortranCUDAKernelSubroutineDirectLoop.h>
#include <FortranCUDAKernelSubroutineIndirectLoop.h>
#include <FortranCUDAHostSubroutineDirectLoop.h>
#include <FortranCUDAHostSubroutineIndirectLoop.h>
#include <FortranCUDAUserSubroutine.h>
#include <FortranCUDAReductionSubroutine.h>
#include <FortranTypesBuilder.h>
#include <RoseHelper.h>
#include <Reduction.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranCUDASubroutinesGeneration::createConstantDeclarations ()
{
  using boost::lexical_cast;
  using boost::iequals;
  using SageInterface::removeStatement;
  using std::map;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Creating constant declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (map <string, OpConstDefinition *>::const_iterator it =
      declarations->firstOpConstDefinition (); it
      != declarations->lastOpConstDefinition (); ++it)
  {
    string const constantName = it->first;

    Debug::getInstance ()->debugMessage ("Creating declaration for variable '"
        + constantName + "'", Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

    SgVariableDeclaration * originalDeclaration =
        declarations->getOriginalDeclaration (constantName);

    SgVariableDeclaration * newDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            constantName,
            originalDeclaration ->get_decl_item (constantName)->get_type (),
            moduleScope, 1, CONSTANT);

    SgInitializedNamePtrList & variables =
        originalDeclaration->get_variables ();

    if (variables.size () == 1)
    {
      Debug::getInstance ()->debugMessage (
          "Single variable in declaration. Removing entire statement",
          Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

      removeStatement (originalDeclaration);
    }
    else
    {
      Debug::getInstance ()->errorMessage (
          "Multiple variables in original declaration. Currently not supported",
          __FILE__, __LINE__);
    }
  }
}

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

    FortranCUDAUserSubroutine * userDeviceSubroutine =
        new FortranCUDAUserSubroutine (userSubroutineName, moduleScope,
            declarations, parallelLoop);

    FortranCUDAKernelSubroutine * kernelSubroutine;

    if (parallelLoop->isDirectLoop ())
    {
      kernelSubroutine
          = new FortranCUDAKernelSubroutineDirectLoop (
              userSubroutineName,
              userDeviceSubroutine->getSubroutineName (),
              parallelLoop,
              moduleScope,
              reductionSubroutines,
              dataSizesDeclarations[userSubroutineName],
              dimensionsDeclarations[userSubroutineName],
              static_cast <FortranCUDAModuleDeclarations *> (moduleDeclarations[userSubroutineName]));

      hostSubroutines[userSubroutineName]
          = new FortranCUDAHostSubroutineDirectLoop (
              userSubroutineName,
              userSubroutineName,
              kernelSubroutine->getSubroutineName (),
              parallelLoop,
              moduleScope,
              dataSizesDeclarations[userSubroutineName],
              dimensionsDeclarations[userSubroutineName],
              static_cast <FortranCUDAModuleDeclarations *> (moduleDeclarations[userSubroutineName]));
    }
    else
    {
      kernelSubroutine
          = new FortranCUDAKernelSubroutineIndirectLoop (
              userSubroutineName,
              userDeviceSubroutine->getSubroutineName (),
              parallelLoop,
              moduleScope,
              reductionSubroutines,
              static_cast <FortranCUDADataSizesDeclarationIndirectLoop *> (dataSizesDeclarations[userSubroutineName]),
              dimensionsDeclarations[userSubroutineName],
              static_cast <FortranCUDAModuleDeclarations *> (moduleDeclarations[userSubroutineName]));

      hostSubroutines[userSubroutineName]
          = new FortranCUDAHostSubroutineIndirectLoop (
              userSubroutineName,
              userSubroutineName,
              kernelSubroutine->getSubroutineName (),
              parallelLoop,
              moduleScope,
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

    Debug::getInstance ()->debugMessage ("Analysing '" + userSubroutineName
        + "'", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    FortranParallelLoop * parallelLoop = it->second;

    dimensionsDeclarations[userSubroutineName]
        = new FortranOpDatDimensionsDeclaration (userSubroutineName,
            parallelLoop, moduleScope);

    if (parallelLoop->isDirectLoop ())
    {
      dataSizesDeclarations[userSubroutineName]
          = new FortranCUDADataSizesDeclaration (userSubroutineName,
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

    Debug::getInstance ()->debugMessage ("Analysing '" + userSubroutineName
        + "'", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

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

  /*
   * ======================================================
   * Now declare constants
   * ======================================================
   */
  createConstantDeclarations ();
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
