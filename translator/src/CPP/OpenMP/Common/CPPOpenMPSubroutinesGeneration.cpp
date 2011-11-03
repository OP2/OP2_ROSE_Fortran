#include <CPPOpenMPSubroutinesGeneration.h>
#include <CPPParallelLoop.h>
#include <CPPUserSubroutine.h>
#include <CPPProgramDeclarationsAndDefinitions.h>
#include <CPPOpenMPHostSubroutineDirectLoop.h>
#include <CPPOpenMPHostSubroutineIndirectLoop.h>
#include <RoseStatementsAndExpressionsBuilder.h>

void
CPPOpenMPSubroutinesGeneration::createSubroutines ()
{
  using namespace SageInterface;
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

    CPPUserSubroutine * userSubroutine = new CPPUserSubroutine (moduleScope,
        parallelLoop, declarations);

    if (parallelLoop->isDirectLoop ())
    {
      hostSubroutines[userSubroutineName]
          = new CPPOpenMPHostSubroutineDirectLoop (moduleScope, userSubroutine,
              parallelLoop);
    }
    else
    {
      hostSubroutines[userSubroutineName]
          = new CPPOpenMPHostSubroutineIndirectLoop (moduleScope,
              userSubroutine, parallelLoop);
    }
  }
}

CPPOpenMPSubroutinesGeneration::CPPOpenMPSubroutinesGeneration (
    SgProject * project, CPPProgramDeclarationsAndDefinitions * declarations) :
  CPPSubroutinesGeneration (project, declarations, "rose_openmp_code.cpp")
{
  generate ();
}
