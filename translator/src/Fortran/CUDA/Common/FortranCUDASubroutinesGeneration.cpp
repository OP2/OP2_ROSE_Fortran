#include <FortranCUDASubroutinesGeneration.h>
#include <FortranCUDAKernelSubroutineDirectLoop.h>
#include <FortranCUDAKernelSubroutineIndirectLoop.h>
#include <FortranCUDAHostSubroutineDirectLoop.h>
#include <FortranCUDAHostSubroutineIndirectLoop.h>
#include <FortranCUDAUserSubroutine.h>
#include <FortranCUDAReductionSubroutine.h>
#include <FortranReductionSubroutines.h>
#include <FortranCUDAModuleDeclarations.h>
#include <FortranCUDAOpDatCardinalitiesDeclarationIndirectLoop.h>
#include <FortranOpDatDimensionsDeclaration.h>
#include <FortranParallelLoop.h>
#include <FortranProgramDeclarationsAndDefinitions.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <OP2Definitions.h>
#include <RoseHelper.h>
#include <Reduction.h>
#include <Exceptions.h>
#include <boost/algorithm/string.hpp>

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
            moduleScope, 1, CUDA_CONSTANT);

    SgInitializedNamePtrList & variables =
        originalDeclaration->get_variables ();

    if (variables.size () == 1)
    {
      Debug::getInstance ()->debugMessage (
          "Single variable in declaration. Removing entire statement",
          Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

      removeStatement (originalDeclaration);
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
        new FortranCUDAUserSubroutine (moduleScope, parallelLoop, declarations);

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

  libs.push_back ("ISO_C_BINDING");
  libs.push_back ("OP2_C");
  libs.push_back ("CUDAFOR");
  libs.push_back ("CUDACONFIGURATIONPARAMS");

  for (vector <string>::const_iterator it = libs.begin (); it != libs.end (); ++it)
  {
    SgUseStatement* useStatement = new SgUseStatement (
        RoseHelper::getFileInfo (), *it, false);

    useStatement->set_definingDeclaration (useStatement);

    appendStatement (useStatement, moduleScope);
  }
}
