#include <CPPSubroutinesGeneration.h>
#include <CPPParallelLoop.h>
#include <CPPHostSubroutine.h>
#include <CPPProgramDeclarationsAndDefinitions.h>
#include <CPPReductionSubroutines.h>
#include <CPPModuleDeclarations.h>
#include <Exceptions.h>
#include "CPPUserSubroutine.h"

void
CPPSubroutinesGeneration::patchCallsToParallelLoops ()
{
  using namespace SageBuilder;
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
            + "'. Call is in file '" + parallelLoop->getFileName () + "'",
        Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

    CPPHostSubroutine * hostSubroutine = hostSubroutines[userSubroutineName];

    SgFunctionCallExp * functionCallExpression =
        parallelLoop->getFunctionCall ();

    SgScopeStatement * scope = isSgExprStatement (
        functionCallExpression->get_parent ())->get_scope ();

    SgFunctionRefExp * hostSubroutineReference = buildFunctionRefExp (
        hostSubroutine->getSubroutineHeaderStatement ());

    functionCallExpression->set_function (hostSubroutineReference);

    if (find (dirtyFiles.begin (), dirtyFiles.end (),
        parallelLoop->getFileName ()) == dirtyFiles.end ())
    {
      dirtyFiles.push_back (parallelLoop->getFileName ());
    }

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
  }
}

void
CPPSubroutinesGeneration::generate ()
{
  moduleScope = sourceFile->get_globalScope ();

  addHeaderIncludes ();

  moduleDeclarations = new CPPModuleDeclarations (moduleScope, declarations);

  createSubroutines ();

  patchCallsToParallelLoops ();
}

CPPSubroutinesGeneration::CPPSubroutinesGeneration (SgProject * project,
    CPPProgramDeclarationsAndDefinitions * declarations,
    std::string const & newFileName) :
  SubroutinesGeneration <CPPProgramDeclarationsAndDefinitions,
      CPPHostSubroutine> (project, declarations, newFileName)
{
  reductionSubroutines = new CPPReductionSubroutines ();
}

