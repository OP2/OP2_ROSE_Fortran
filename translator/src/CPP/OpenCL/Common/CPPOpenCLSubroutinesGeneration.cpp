#include <CPPOpenCLSubroutinesGeneration.h>
#include <CPPOpenCLKernelSubroutineDirectLoop.h>
#include <CPPOpenCLKernelSubroutineIndirectLoop.h>
#include <CPPOpenCLHostSubroutineDirectLoop.h>
#include <CPPOpenCLHostSubroutineIndirectLoop.h>
#include <CPPOpenCLUserSubroutine.h>
#include <CPPParallelLoop.h>
#include <CPPProgramDeclarationsAndDefinitions.h>

namespace Libraries
{
  std::string const OPENCL = "CL/cl.h";
}

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

    Debug::getInstance ()->debugMessage ("Analysing parallel loop "
        + userSubroutineName, Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    CPPParallelLoop * parallelLoop =
        static_cast <CPPParallelLoop *> (it->second);

    CPPOpenCLUserSubroutine * userDeviceSubroutine =
        new CPPOpenCLUserSubroutine (moduleScope, parallelLoop, declarations);

    CPPOpenCLKernelSubroutine * kernelSubroutine;

    if (parallelLoop->isDirectLoop ())
    {
      kernelSubroutine = new CPPOpenCLKernelSubroutineDirectLoop (moduleScope,
          userDeviceSubroutine, parallelLoop, NULL);

      hostSubroutines[userSubroutineName]
          = new CPPOpenCLHostSubroutineDirectLoop (moduleScope,
              kernelSubroutine, parallelLoop);
    }
    else
    {
      kernelSubroutine = new CPPOpenCLKernelSubroutineIndirectLoop (
          moduleScope, userDeviceSubroutine, parallelLoop, NULL);

      hostSubroutines[userSubroutineName]
          = new CPPOpenCLHostSubroutineIndirectLoop (moduleScope,
              kernelSubroutine, parallelLoop);
    }
  }
}

void
CPPOpenCLSubroutinesGeneration::addLibraries ()
{
  using std::string;
  using std::vector;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Adding '#include' statements to main file", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  vector <string> libs;
  libs.push_back (Libraries::OPENCL);

  for (vector <string>::const_iterator it = libs.begin (); it != libs.end (); ++it)
  {
  }

  SgVariableDeclaration *test = SageBuilder::buildVariableDeclaration ("test",
      SageBuilder::buildIntType (), NULL, moduleScope);

  appendStatement (test, moduleScope);
}

CPPOpenCLSubroutinesGeneration::CPPOpenCLSubroutinesGeneration (
    SgProject * project, CPPProgramDeclarationsAndDefinitions * declarations) :
  CPPSubroutinesGeneration (project, declarations, "rose_opencl_code.cpp")
{
  generate ();
}
