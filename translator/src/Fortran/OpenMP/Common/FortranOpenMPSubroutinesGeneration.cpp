


/*  Open source copyright declaration based on BSD open source template:
 *  http://www.opensource.org/licenses/bsd-license.php
 * 
 * Copyright (c) 2011-2012, Adam Betts, Carlo Bertolli
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


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
#include "FortranConstantDeclarations.h"
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

  Debug::getInstance ()->debugMessage ("Creating subroutines: appending constants initialisation subroutine",
    Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
  
  OP2constants->appendConstantInitialisationToModule ( moduleScope, declarations, /* isCuda = */ false );

  Debug::getInstance ()->debugMessage ("Creating subroutines: scanning all parallel loops",
    Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  
  /*
   * ======================================================
   * This vector contains all subroutines called by user
   * kernels, to avoid their duplication in the output
   * file
   * ======================================================
   */
  vector < SgProcedureHeaderStatement * > allCalledRoutines;

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    Debug::getInstance ()->debugMessage ("Analysing user subroutine '"
        + userSubroutineName + "' with subroutines already defined in previous kernels, number = "
        + boost::lexical_cast<string> (allCalledRoutines.size()), Debug::FUNCTION_LEVEL, __FILE__, __LINE__);    
    
    vector < SgProcedureHeaderStatement * > :: iterator finder;
    for ( finder = allCalledRoutines.begin (); finder != allCalledRoutines.end (); finder++ )
    {
      Debug::getInstance ()->debugMessage ("Routine: '" + (*finder)->get_name ().getString () + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
    }

    FortranParallelLoop * parallelLoop =
        static_cast <FortranParallelLoop *> (it->second);

    Debug::getInstance ()->debugMessage ("Creating userSubroutine object",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);            
        
    FortranUserSubroutine * userSubroutine = new FortranUserSubroutine (
        moduleScope, parallelLoop, declarations);

    userSubroutine->createFormalParameterDeclarations ();
    userSubroutine->createStatements ();
        
    Debug::getInstance ()->debugMessage ("Creating OP2Constants",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);                    
        
    OP2constants->patchReferencesToConstants (
       userSubroutine->getSubroutineHeaderStatement ());

    /* This is not done in CUDA (done in the constructor of the user sub.?)*/
    //    userSubroutine->createFormalParameterDeclarations ();
    //userSubroutine->createLocalVariableDeclarations ();
    //userSubroutine->createStatements ();

    /*
     * ======================================================
     * We have to set each node in the AST representation of
     * this subroutine as compiler generated, otherwise chunks
     * of the user kernel are missing in the output
     * ======================================================
     */

    RoseHelper::forceOutputOfCodeToFile (
        userSubroutine->getSubroutineHeaderStatement ());

    /*
     * ======================================================
     * When the user subroutine has calls to other user
     * subroutines we need to add them to the generated file
     * allCalledRoutines is not used in the openMP case
     * (need to eliminate it from these calls)
     * ======================================================
     */

    userSubroutine->appendAdditionalSubroutines (moduleScope, parallelLoop, declarations, OP2constants, &allCalledRoutines);

    // This is recursively called in the previous routine...erase it
    //    vector < FortranUserSubroutine * > additionalSubroutines = userSubroutine->getAdditionalSubroutines ();
    
    // same of above...
    //    vector < FortranUserSubroutine * > :: iterator it;
    //    for ( it = additionalSubroutines.begin(); it != additionalSubroutines.end(); it++ )
    //    {          
    //      RoseHelper::forceOutputOfCodeToFile (
    //        (*it)->getSubroutineHeaderStatement ());
    //    }        

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

  OP2constants = new FortranConstantDeclarations (declarations,
      moduleScope, false);
  
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
