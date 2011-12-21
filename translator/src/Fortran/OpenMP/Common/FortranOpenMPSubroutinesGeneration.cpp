#include "FortranOpenMPSubroutinesGeneration.h"
#include "FortranParallelLoop.h"
#include "FortranUserSubroutine.h"
#include "FortranOpenMPModuleDeclarationsIndirectLoop.h"
#include "FortranOpenMPKernelSubroutineDirectLoop.h"
#include "FortranOpenMPKernelSubroutineIndirectLoop.h"
#include "FortranOpenMPHostSubroutineDirectLoop.h"
#include "FortranOpenMPHostSubroutineIndirectLoop.h"
#include "FortranProgramDeclarationsAndDefinitions.h"
#include "FortranOpDatDimensionsDeclaration.h"
#include "RoseHelper.h"
#include "Globals.h"
#include "OpenMP.h"
#include "OP2.h"
#include <boost/algorithm/string.hpp>

void
FortranOpenMPSubroutinesGeneration::createReductionSubroutines ()
{
}

void
FortranOpenMPSubroutinesGeneration::createSubroutines ()
{
  using std::string;
  using std::map;

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    FortranParallelLoop * parallelLoop =
        static_cast <FortranParallelLoop *> (it->second);

    FortranUserSubroutine * userSubroutine = new FortranUserSubroutine (
        moduleScope, parallelLoop, declarations);

    userSubroutine->createFormalParameterDeclarations ();
    userSubroutine->createLocalVariableDeclarations ();
    userSubroutine->createStatements ();
    RoseHelper::forceOutputOfCodeToFile (
        userSubroutine->getSubroutineHeaderStatement ());

    /*
     * ======================================================
     * When the user subroutine has calls to other user
     * subroutines we need to add them to the generated file
     * ======================================================
     */
        
    userSubroutine->appendAdditionalSubroutines (moduleScope, parallelLoop, declarations);
    
    vector < FortranUserSubroutine * > additionalSubroutines = userSubroutine->getAdditionalSubroutines ();
    
    vector < FortranUserSubroutine * > :: iterator it;
    for ( it = additionalSubroutines.begin(); it != additionalSubroutines.end(); it++ )
    {          
      RoseHelper::forceOutputOfCodeToFile (
        (*it)->getSubroutineHeaderStatement ());
    }        
        
        
    FortranOpenMPKernelSubroutine * kernelSubroutine;

    if (parallelLoop->isDirectLoop ())
    {
      kernelSubroutine = new FortranOpenMPKernelSubroutineDirectLoop (
          moduleScope, userSubroutine, parallelLoop);

      hostSubroutines[userSubroutineName]
          = new FortranOpenMPHostSubroutineDirectLoop (moduleScope,
              kernelSubroutine, parallelLoop);
    }
    else
    {
      kernelSubroutine = new FortranOpenMPKernelSubroutineIndirectLoop (
          moduleScope, userSubroutine, parallelLoop);

      hostSubroutines[userSubroutineName]
          = new FortranOpenMPHostSubroutineIndirectLoop (
              moduleScope,
              kernelSubroutine,
              parallelLoop,
              static_cast <FortranOpenMPModuleDeclarationsIndirectLoop *> (moduleDeclarations[userSubroutineName]));
    }
  }
}

void
FortranOpenMPSubroutinesGeneration::createModuleDeclarations ()
{
  using std::map;
  using std::string;

  /*
   * ======================================================
   * Declare module-wide variables for indirect loops
   * ======================================================
   */

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    FortranParallelLoop * parallelLoop =
        static_cast <FortranParallelLoop *> (it->second);

    if (parallelLoop->isDirectLoop () == false)
    {
      moduleDeclarations[userSubroutineName]
          = new FortranOpenMPModuleDeclarationsIndirectLoop (parallelLoop,
              moduleScope);
    }
  }
}

void
FortranOpenMPSubroutinesGeneration::addLibraries ()
{
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage (
      "Adding 'use' statements to OpenMP module", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Module including OP2 declarations
   * ======================================================
   */

  SgUseStatement* useStatement1 = new SgUseStatement (
      RoseHelper::getFileInfo (), OP2::Libraries::Fortran::declarations, false);

  useStatement1->set_definingDeclaration (useStatement1);

  appendStatement (useStatement1, moduleScope);

  /*
   * ======================================================
   * Module including OP2 run-time support
   * ======================================================
   */

  SgUseStatement* useStatement2 = new SgUseStatement (
      RoseHelper::getFileInfo (), OP2::Libraries::Fortran::runtimeSupport,
      false);

  useStatement2->set_definingDeclaration (useStatement2);

  appendStatement (useStatement2, moduleScope);

  /*
   * ======================================================
   * Module including free variables referenced in
   * user kernels
   * ======================================================
   */

  SgUseStatement* useStatement3 = new SgUseStatement (
      RoseHelper::getFileInfo (),
      Globals::getInstance ()->getFreeVariablesModuleName (), false);

  useStatement3->set_definingDeclaration (useStatement3);

  appendStatement (useStatement3, moduleScope);

  /*
   * ======================================================
   * Module including OpenMP run-time support
   * ======================================================
   */

  SgUseStatement* useStatement4 = new SgUseStatement (
      RoseHelper::getFileInfo (), OpenMP::Fortran::libraryName, false);

  useStatement4->set_definingDeclaration (useStatement4);

  appendStatement (useStatement4, moduleScope);

  addTextForUnparser (useStatement4, OpenMP::getIfDirectiveString (),
      AstUnparseAttribute::e_before);

  addTextForUnparser (useStatement4, OpenMP::getEndIfDirectiveString (),
      AstUnparseAttribute::e_after);
}
