#include <CPPCUDASubroutinesGeneration.h>

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

CPPCUDASubroutinesGeneration::CPPCUDASubroutinesGeneration (
    SgProject * project, CPPProgramDeclarationsAndDefinitions * declarations) :
  CPPSubroutinesGeneration (project, declarations, "rose_cuda_code.cpp")
{
}
