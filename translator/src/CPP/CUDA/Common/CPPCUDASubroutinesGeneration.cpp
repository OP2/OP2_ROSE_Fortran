#include <CPPCUDASubroutinesGeneration.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

CPPHostSubroutine *
CPPCUDASubroutinesGeneration::createSubroutines ()
{
  return NULL;
}

void
CPPCUDASubroutinesGeneration::addLibraries ()
{
  return;
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

CPPCUDASubroutinesGeneration::CPPCUDASubroutinesGeneration (
    SgProject * project, CPPProgramDeclarationsAndDefinitions * declarations) :
  CPPSubroutinesGeneration (declarations, ".CUF")
{
  unparse ();
}
