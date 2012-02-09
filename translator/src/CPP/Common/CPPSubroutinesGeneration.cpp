


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


#include "CPPSubroutinesGeneration.h"
#include "CPPParallelLoop.h"
#include "CPPHostSubroutine.h"
#include "CPPProgramDeclarationsAndDefinitions.h"
#include "CPPReductionSubroutines.h"
#include "CPPModuleDeclarations.h"
#include "Exceptions.h"
#include "CPPUserSubroutine.h"
#include "OP2.h"
#include "Globals.h"

void
CPPSubroutinesGeneration::addFreeVariableDeclarations ()
{
}

void
CPPSubroutinesGeneration::addOP2IncludeDirective ()
{
  using namespace SageInterface;
  using std::vector;
  using std::string;

  Debug::getInstance ()->debugMessage ("Adding OP2 include directive",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (vector <string>::iterator it = dirtyFiles.begin (); it
      != dirtyFiles.end (); ++it)
  {
    std::string const & fileName = *it;

    SgSourceFile * sourceFile = declarations->getSourceFile (fileName);

    SgStatement * lastDeclarationStatement = findLastDeclarationStatement (
        sourceFile->get_globalScope ());

    addTextForUnparser (lastDeclarationStatement, "#include \""
        + OP2::Libraries::CPP::mainLibrary + "\"\n",
        AstUnparseAttribute::e_before);

    addTextForUnparser (lastDeclarationStatement, "#include \"" + newFileName
        + "\"\n", AstUnparseAttribute::e_before);
  }
}

void
CPPSubroutinesGeneration::patchCallsToParallelLoops ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using std::map;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Patching calls to OP_PAR_LOOPs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  vector <string> processedFiles;

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    CPPParallelLoop * parallelLoop =
        static_cast <CPPParallelLoop *> (it->second);

    Debug::getInstance ()->debugMessage (
        "Analysing OP_PAR_LOOP for user subroutine '" + userSubroutineName
            + "'", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    CPPHostSubroutine * hostSubroutine = hostSubroutines[userSubroutineName];

    for (vector <SgFunctionCallExp *>::const_iterator it =
        parallelLoop->getFirstFunctionCall (); it
        != parallelLoop->getLastFunctionCall (); ++it)
    {
      SgFunctionCallExp * functionCallExpression = *it;

      SgScopeStatement * scope = isSgExprStatement (
          functionCallExpression->get_parent ())->get_scope ();

      SgFunctionRefExp * hostSubroutineReference = buildFunctionRefExp (
          hostSubroutine->getSubroutineHeaderStatement ());

      functionCallExpression->set_function (hostSubroutineReference);

      /*
       * ==================================================
       * Remove the first parameter (kernel reference)
       * ==================================================
       */

      SgExpressionPtrList & arguments =
          functionCallExpression->get_args ()->get_expressions ();

      arguments.erase (arguments.begin ());

      arguments.erase (arguments.begin ());

      arguments.insert (arguments.begin (), buildStringVal (
          userSubroutines[userSubroutineName]->getSubroutineName ()));

      for (vector <string>::const_iterator it =
          parallelLoop->getFirstFileName (); it
          != parallelLoop->getLastFileName (); ++it)
      {
        string fileName = *it;

        Debug::getInstance ()->debugMessage ("Analysing file '" + fileName
            + "'", Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

        SgSourceFile * sourceFile = declarations->getSourceFile (fileName);

        SgScopeStatement * fileScope = sourceFile->get_globalScope ();

        SgFunctionDeclaration * nonDefininingDeclaration =
            buildNondefiningFunctionDeclaration (
                hostSubroutine->getSubroutineName (), buildVoidType (),
                hostSubroutine->getCopyOfFormalParameters (), fileScope);

        SgStatement * declarationStatement = findLastDeclarationStatement (
            fileScope);

        insertStatementBefore (declarationStatement, nonDefininingDeclaration);
      }
    }
  }
}

void
CPPSubroutinesGeneration::generate ()
{
  moduleScope = sourceFile->get_globalScope ();

  addHeaderIncludes ();

  addFreeVariableDeclarations ();

  moduleDeclarations = new CPPModuleDeclarations (moduleScope, declarations);

  createSubroutines ();

  patchCallsToParallelLoops ();

  addOP2IncludeDirective ();

  determineWhichInputFilesToBeUnparsed ();
}

CPPSubroutinesGeneration::CPPSubroutinesGeneration (SgProject * project,
    CPPProgramDeclarationsAndDefinitions * declarations,
    std::string const & newFileName) :
  SubroutinesGeneration <CPPProgramDeclarationsAndDefinitions,
      CPPHostSubroutine> (project, declarations, newFileName)
{
  reductionSubroutines = new CPPReductionSubroutines ();
}

