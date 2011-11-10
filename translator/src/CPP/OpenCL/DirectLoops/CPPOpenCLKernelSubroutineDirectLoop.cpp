#include <CPPOpenCLKernelSubroutineDirectLoop.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <Debug.h>
#include <OpenCL.h>
#include <CompilerGeneratedNames.h>

SgStatement *
CPPOpenCLKernelSubroutineDirectLoop::createUserSubroutineCallStatement ()
{
}

void
CPPOpenCLKernelSubroutineDirectLoop::createExecutionLoopStatements ()
{
}

void
CPPOpenCLKernelSubroutineDirectLoop::createStatements ()
{
  Debug::getInstance ()->debugMessage ("Creating Statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

void
CPPOpenCLKernelSubroutineDirectLoop::createLocalVariableDeclarations ()
{
}

void
CPPOpenCLKernelSubroutineDirectLoop::createOpDatFormalParameterDeclarations ()
{
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace SageBuilder;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating OP_DAT formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i))
      {
        string const & variableName = getReductionArrayDeviceName (i);

        Debug::getInstance ()->debugMessage ("Reduction type",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        variableDeclarations->add (
            variableName,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, buildPointerType (parallelLoop->getOpDatBaseType (
                    i)), subroutineScope, formalParameters));

      }
      else if (parallelLoop->isDirect (i))
      {
        Debug::getInstance ()->debugMessage ("OP_DAT: direct",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        string const & variableName = getOpDatName (i);

        variableDeclarations->add (
            variableName,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, buildPointerType (parallelLoop->getOpDatBaseType (
                    i)), subroutineScope, formalParameters));
      }
      else if (parallelLoop->isRead (i))
      {
        Debug::getInstance ()->debugMessage ("Read",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        string const & variableName = getOpDatName (i);
      }
    }
  }
}

void
CPPOpenCLKernelSubroutineDirectLoop::createFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;

  createOpDatFormalParameterDeclarations ();

  /*
   * ======================================================
   * Offset into shared memory
   * ======================================================
   */

  variableDeclarations->add (
      sharedMemoryOffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          sharedMemoryOffset, buildIntType (), subroutineScope,
          formalParameters));

  /*
   * ======================================================
   * OP_SET size
   * ======================================================
   */

  variableDeclarations->add (
      setSize,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          setSize, buildIntType (), subroutineScope, formalParameters));
}

CPPOpenCLKernelSubroutineDirectLoop::CPPOpenCLKernelSubroutineDirectLoop (
    SgScopeStatement * moduleScope, CPPOpenCLUserSubroutine * calleeSubroutine,
    CPPParallelLoop * parallelLoop,
    CPPReductionSubroutines * reductionSubroutines) :
  CPPOpenCLKernelSubroutine (moduleScope, calleeSubroutine, parallelLoop,
      reductionSubroutines)
{
  Debug::getInstance ()->debugMessage (
      "Creating OpenCL kernel for direct loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
