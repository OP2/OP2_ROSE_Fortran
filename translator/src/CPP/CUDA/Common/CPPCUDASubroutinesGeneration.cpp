#include <CPPCUDASubroutinesGeneration.h>
#include <CPPParallelLoop.h>
#include <CPPProgramDeclarationsAndDefinitions.h>
#include <CPPCUDAKernelSubroutine.h>
#include <CPPCUDAKernelSubroutineDirectLoop.h>
#include <CPPCUDAHostSubroutineDirectLoop.h>
#include <CPPCUDAUserSubroutine.h>

void
CPPCUDASubroutinesGeneration::createSubroutines ()
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

    CPPParallelLoop * parallelLoop =
        static_cast <CPPParallelLoop *> (it->second);

    CPPCUDAUserSubroutine * userDeviceSubroutine = new CPPCUDAUserSubroutine (
        moduleScope, parallelLoop, declarations);

    CPPCUDAKernelSubroutine * kernelSubroutine;

    if (parallelLoop->isDirectLoop ())
    {
      kernelSubroutine = new CPPCUDAKernelSubroutineDirectLoop (moduleScope,
          userDeviceSubroutine, parallelLoop);

      hostSubroutines[userSubroutineName]
          = new CPPCUDAHostSubroutineDirectLoop (moduleScope, kernelSubroutine,
              parallelLoop);
    }
    else
    {
    }
  }
}

CPPCUDASubroutinesGeneration::CPPCUDASubroutinesGeneration (
    SgProject * project, CPPProgramDeclarationsAndDefinitions * declarations) :
  CPPSubroutinesGeneration (project, declarations, "rose_cuda_code.cpp")
{
  generate ();
}
