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
#include "RoseStatementsAndExpressionsBuilder.h"
#include "OP2Definitions.h"

void
CPPOpenCLSubroutinesGeneration::addFreeVariableDeclarations ()
{
  using std::map;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Adding variables with constant access specifiers to module",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (map <string, OpConstDefinition *>::const_iterator it =
      declarations->firstOpConstDefinition (); it
      != declarations->lastOpConstDefinition (); ++it)
  {
    std::string const & variableName = it->first;

    OpConstDefinition * constDefinition = it->second;

    SgType * type = constDefinition->getType ();

    Debug::getInstance ()->debugMessage ("Analysing OP_DECL_CONST with name '"
        + variableName + "'", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    SgVariableDeclaration * variableDeclaration =
        RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
            variableName, type, moduleScope);

    variableDeclaration->get_declarationModifier ().get_storageModifier ().setExtern ();
  }
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
          userDeviceSubroutine, parallelLoop, reductionSubroutines);

      hostSubroutines[userSubroutineName]
          = new CPPOpenCLHostSubroutineDirectLoop (moduleScope,
              kernelSubroutine, parallelLoop, moduleDeclarations);
    }
    else
    {
      kernelSubroutine
          = new CPPOpenCLKernelSubroutineIndirectLoop (moduleScope,
              userDeviceSubroutine, parallelLoop, reductionSubroutines);

      hostSubroutines[userSubroutineName]
          = new CPPOpenCLHostSubroutineIndirectLoop (moduleScope,
              kernelSubroutine, parallelLoop, moduleDeclarations);
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
