#include "CPPCUDASubroutinesGeneration.h"
#include "CPPParallelLoop.h"
#include "CPPProgramDeclarationsAndDefinitions.h"
#include "CPPCUDAKernelSubroutine.h"
#include "CPPCUDAKernelSubroutineDirectLoop.h"
#include "CPPCUDAHostSubroutineDirectLoop.h"
#include "CPPCUDAKernelSubroutineIndirectLoop.h"
#include "CPPCUDAHostSubroutineIndirectLoop.h"
#include "CPPCUDAUserSubroutine.h"
#include "CPPCUDAReductionSubroutine.h"
#include "CPPReductionSubroutines.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "CUDA.h"
#include "OP2.h"
#include "OP2Definitions.h"

void
CPPCUDASubroutinesGeneration::addFreeVariableDeclarations ()
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

    variableDeclaration->get_declarationModifier ().get_storageModifier ().setCudaConstant ();
  }
}

void
CPPCUDASubroutinesGeneration::createReductionSubroutines ()
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
    CPPCUDAReductionSubroutine * subroutine = new CPPCUDAReductionSubroutine (
        moduleScope, *it);

    reductionSubroutines->addSubroutine (*it,
        subroutine->getSubroutineHeaderStatement ());
  }
}

void
CPPCUDASubroutinesGeneration::addHeaderIncludes ()
{
  using namespace SageInterface;

  addTextForUnparser (moduleScope, "#include \""
      + OP2::Libraries::CPP::mainLibrary + "\"\n",
      AstUnparseAttribute::e_before);

  addTextForUnparser (moduleScope, "#include \"" + CUDA::CPP::OP2RuntimeSupport
      + "\"\n", AstUnparseAttribute::e_before);
}

void
CPPCUDASubroutinesGeneration::createSubroutines ()
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

    CPPCUDAUserSubroutine * userDeviceSubroutine = new CPPCUDAUserSubroutine (
        moduleScope, parallelLoop, declarations);

    userSubroutines[userSubroutineName] = userDeviceSubroutine;

    CPPCUDAKernelSubroutine * kernelSubroutine;

    if (parallelLoop->isDirectLoop ())
    {
      kernelSubroutine = new CPPCUDAKernelSubroutineDirectLoop (moduleScope,
          userDeviceSubroutine, parallelLoop, reductionSubroutines);

      hostSubroutines[userSubroutineName]
          = new CPPCUDAHostSubroutineDirectLoop (moduleScope, kernelSubroutine,
              parallelLoop, moduleDeclarations);
    }
    else
    {
      kernelSubroutine = new CPPCUDAKernelSubroutineIndirectLoop (moduleScope,
          userDeviceSubroutine, parallelLoop, reductionSubroutines);

      hostSubroutines[userSubroutineName]
          = new CPPCUDAHostSubroutineIndirectLoop (moduleScope,
              kernelSubroutine, parallelLoop, moduleDeclarations);
    }
  }
}

CPPCUDASubroutinesGeneration::CPPCUDASubroutinesGeneration (
    SgProject * project, CPPProgramDeclarationsAndDefinitions * declarations) :
  CPPSubroutinesGeneration (project, declarations, "rose_cuda_code.cpp")
{
  generate ();
}
