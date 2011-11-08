#include <CPPOpenCLSubroutinesGeneration.h>
#include <CPPParallelLoop.h>
#include <CPPProgramDeclarationsAndDefinitions.h>
#include <CPPOpenCLKernelSubroutineDirectLoop.h>
#include <CPPOpenCLKernelSubroutineIndirectLoop.h>
#include <CPPOpenCLHostSubroutineDirectLoop.h>
#include <CPPOpenCLHostSubroutineIndirectLoop.h>
#include <CPPOpenCLUserSubroutine.h>
#include <CPPOpenCLReductionSubroutine.h>
#include <CPPReductionSubroutines.h>

void
CPPOpenCLSubroutinesGeneration::createSubroutines ()
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

    CPPOpenCLUserSubroutine * userDeviceSubroutine =
        new CPPOpenCLUserSubroutine (moduleScope, parallelLoop, declarations);

    CPPOpenCLKernelSubroutine * kernelSubroutine;

    if (parallelLoop->isDirectLoop ())
    {
      kernelSubroutine = new CPPOpenCLKernelSubroutineDirectLoop (moduleScope,
          userDeviceSubroutine, parallelLoop, reductionSubroutines);
    }
    else
    {
      kernelSubroutine
          = new CPPOpenCLKernelSubroutineIndirectLoop (moduleScope,
              userDeviceSubroutine, parallelLoop, reductionSubroutines);
    }
  }
}

void
CPPOpenCLSubroutinesGeneration::addHeaderIncludes ()
{
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage (
      "Adding '#include' statements to main file", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  addTextForUnparser (moduleScope, "\n#include <CL/cl.h>\n",
      AstUnparseAttribute::e_after);
}

CPPOpenCLSubroutinesGeneration::CPPOpenCLSubroutinesGeneration (
    SgProject * project, CPPProgramDeclarationsAndDefinitions * declarations) :
  CPPSubroutinesGeneration (project, declarations, "rose_opencl_code.cpp")
{
  generate ();
}
