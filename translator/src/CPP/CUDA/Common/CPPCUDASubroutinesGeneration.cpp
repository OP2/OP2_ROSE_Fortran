#include <CPPCUDASubroutinesGeneration.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

CPPHostSubroutine *
CPPCUDASubroutinesGeneration::createSubroutines (
    CPPParallelLoop * parallelLoop, std::string const & userSubroutineName)
{
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
