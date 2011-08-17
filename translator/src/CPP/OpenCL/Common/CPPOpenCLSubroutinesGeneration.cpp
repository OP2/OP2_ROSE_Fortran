#include <CPPOpenCLSubroutinesGeneration.h>
//#include <CPPOpenCLModuleDeclarationsIndirectLoop.h>
#include <CPPOpenCLKernelSubroutineDirectLoop.h>
#include <CPPOpenCLKernelSubroutineIndirectLoop.h>
#include <CPPOpenCLHostSubroutineDirectLoop.h>
#include <CPPOpenCLHostSubroutineIndirectLoop.h>
#include <CPPOpenCLUserSubroutine.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

CPPHostSubroutine *
CPPOpenCLSubroutinesGeneration::createSubroutines ()
{
  using std::string;
  using std::map;

  for (map <string, CPPParallelLoop *>::const_iterator it = declarations->firstParallelLoop (); 
      it != declarations->lastParallelLoop (); 
      ++it)
  {
    string const userSubroutineName = it->first;

    CPPParallelLoop * parallelLoop = it->second;

    CPPOpenCLUserSubroutine * userDeviceSubroutine;

    CPPOpenCLKernelSubroutine * kernelSubroutine;

    if (parallelLoop->isDirectLoop ())
    {
      userDeviceSubroutine = new CPPOpenCLUserSubroutine (
          userSubroutineName,
          moduleScope,
          //initialiseConstantsSubroutine,
          declarations,
          parallelLoop);

      kernelSubroutine = new CPPOpenCLKernelSubroutineDirectLoop (
          userSubroutineName,
          userDeviceSubroutine->getSubroutineName(),
          parallelLoop,
          moduleScope,
          NULL,
          //static_cast <CPPOpenCLDataSizesDeclarationDirectLoop *> (dataSizesDeclarations[userSubroutineName]),
          dimensionsDeclarations[userSubroutineName]);

      hostSubroutines[userSubroutineName] = new CPPOpenCLHostSubroutineDirectLoop (
          userSubroutineName,
          userSubroutineName,
          kernelSubroutine->getSubroutineName(),
          parallelLoop,
          moduleScope,
          //initialiseConstantsSubroutine,
          static_cast <CPPOpenCLDataSizesDeclarationDirectLoop *> (dataSizesDeclarations[userSubroutineName]),
          dimensionsDeclarations[userSubroutineName]);
          //static_cast <CPPOpenCLModuleDeclarations *> (moduleDeclarations[userSubroutineName]));
    }
    else
    {
      userDeviceSubroutine = new CPPOpenCLUserSubroutine (
          userSubroutineName,
          moduleScope, 
          //initialiseConstantsSubroutine, 
          declarations,
          parallelLoop);

      kernelSubroutine = new CPPOpenCLKernelSubroutineIndirectLoop (
          userSubroutineName,
          userDeviceSubroutine->getSubroutineName (),
          parallelLoop,
          moduleScope,
          NULL,
          static_cast <CPPOpenCLDataSizesDeclarationIndirectLoop *> (dataSizesDeclarations[userSubroutineName]),
          dimensionsDeclarations[userSubroutineName]);

      hostSubroutines[userSubroutineName] = new CPPOpenCLHostSubroutineIndirectLoop (
          userSubroutineName,
          userSubroutineName,
          kernelSubroutine->getSubroutineName (),
          parallelLoop,
          moduleScope,
          //initialiseConstantsSubroutine,
          static_cast <CPPOpenCLDataSizesDeclarationIndirectLoop *> (dataSizesDeclarations[userSubroutineName]),
          dimensionsDeclarations[userSubroutineName]);
          //static_cast <CPPOpenCLModuleDeclarationsIndirectLoop *> (moduleDeclarations[userSubroutineName]));
    }
  }
  return NULL; //FIXME
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
  unparse ();
}
