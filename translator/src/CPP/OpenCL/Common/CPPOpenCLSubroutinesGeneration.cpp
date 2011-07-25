#include <CPPOpenCLSubroutinesGeneration.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

CPPHostSubroutine *
CPPOpenCLSubroutinesGeneration::createSubroutines (
    CPPParallelLoop * parallelLoop, std::string const & userSubroutineName)
{

}

void
CPPOpenCLSubroutinesGeneration::addLibraries (
    SgModuleStatement * moduleStatement)
{

}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

CPPOpenCLSubroutinesGeneration::CPPOpenCLSubroutinesGeneration (
    SgProject * project, CPPProgramDeclarationsAndDefinitions * declarations) :
  CPPSubroutinesGeneration (declarations, ".cpp")
{
  traverseInputFiles (project, preorder);

  patchCallsToParallelLoops ();

  unparse ();
}
