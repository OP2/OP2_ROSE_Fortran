#include "CPPOpenCLSubroutinesGeneration.h"
#include "CPPParallelLoop.h"
#include "CPPProgramDeclarationsAndDefinitions.h"
#include "CPPOpenCLKernelSubroutineDirectLoop.h"
#include "CPPOpenCLKernelSubroutineIndirectLoop.h"
#include "CPPOpenCLHostSubroutineDirectLoop.h"
#include "CPPOpenCLHostSubroutineIndirectLoop.h"
#include "CPPOpenCLUserSubroutine.h"
#include "CPPOpenCLReductionSubroutine.h"
#include "CPPReductionSubroutines.h"
#include "CPPOpenCLConstantDeclarations.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "OP2Definitions.h"
#include "OP2.h"

void
CPPOpenCLSubroutinesGeneration::addFreeVariableDeclarations ()
{
  constantDeclarations = new CPPOpenCLConstantDeclarations (declarations,
      moduleScope);
}

void
CPPOpenCLSubroutinesGeneration::createReductionSubroutines ()
{
  using boost::lexical_cast;
  using std::string;
  using std::map;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Creating reduction subroutines",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  vector <Reduction *> reductionsNeeded;

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    ParallelLoop * parallelLoop = it->second;

    parallelLoop->getReductionsNeeded (reductionsNeeded);
  }

  for (vector <Reduction *>::const_iterator it = reductionsNeeded.begin (); it
      != reductionsNeeded.end (); ++it)
  {
    CPPOpenCLReductionSubroutine * subroutine =
        new CPPOpenCLReductionSubroutine (moduleScope, *it);

    reductionSubroutines->addSubroutine (*it,
        subroutine->getSubroutineHeaderStatement ());
  }
}

void
CPPOpenCLSubroutinesGeneration::createSubroutines ()
{
  using std::string;
  using std::map;

  createReductionSubroutines ();

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

    userSubroutines[userSubroutineName] = userDeviceSubroutine;

    CPPOpenCLKernelSubroutine * kernelSubroutine;

    if (parallelLoop->isDirectLoop ())
    {
      kernelSubroutine = new CPPOpenCLKernelSubroutineDirectLoop (moduleScope,
          userDeviceSubroutine, parallelLoop, reductionSubroutines,
          declarations);

      hostSubroutines[userSubroutineName]
          = new CPPOpenCLHostSubroutineDirectLoop (moduleScope,
              kernelSubroutine, parallelLoop, moduleDeclarations,
              userDeviceSubroutine, constantDeclarations);
    }
    else
    {
      kernelSubroutine = new CPPOpenCLKernelSubroutineIndirectLoop (
          moduleScope, userDeviceSubroutine, parallelLoop,
          reductionSubroutines, declarations);

      hostSubroutines[userSubroutineName]
          = new CPPOpenCLHostSubroutineIndirectLoop (moduleScope,
              kernelSubroutine, parallelLoop, moduleDeclarations,
              userDeviceSubroutine, constantDeclarations);
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

  addTextForUnparser (moduleScope, "#include \""
      + OP2::Libraries::CPP::mainLibrary + "\"\n",
      AstUnparseAttribute::e_before);
}

CPPOpenCLSubroutinesGeneration::CPPOpenCLSubroutinesGeneration (
    SgProject * project, CPPProgramDeclarationsAndDefinitions * declarations) :
  CPPSubroutinesGeneration (project, declarations, "rose_opencl_code.cpp")
{
  generate ();
}
