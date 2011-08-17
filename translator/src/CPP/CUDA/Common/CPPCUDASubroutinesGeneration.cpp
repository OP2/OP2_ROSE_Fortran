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
