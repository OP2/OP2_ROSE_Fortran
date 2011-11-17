#include <CPPOpenMPKernelSubroutineIndirectLoop.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CompilerGeneratedNames.h>

SgStatement *
CPPOpenMPKernelSubroutineIndirectLoop::createUserSubroutineCallStatement ()
{

}

void
CPPOpenMPKernelSubroutineIndirectLoop::createExecutionLoopStatements ()
{
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createStatements ()
{
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createIncrementAccessLocalVariableDeclarations ()
{

}

void
CPPOpenMPKernelSubroutineIndirectLoop::createExecutionLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed to execute kernel",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (sharedMemoryOffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          sharedMemoryOffset, buildIntType (), subroutineScope));

  variableDeclarations->add (nelems,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (nelems,
          buildIntType (), subroutineScope));

  variableDeclarations ->add (nbytes,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (nbytes,
          buildIntType (), subroutineScope));

  variableDeclarations ->add (
      getIterationCounterVariableName (1),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1), buildIntType (), subroutineScope));

  variableDeclarations ->add (getUpperBoundVariableName (1),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getUpperBoundVariableName (1), buildIntType (), subroutineScope));
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createCUDAStageInVariablesVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local thread variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & variableName = getOpDatLocalName (i);

    if (parallelLoop->isCUDAStageInVariableDeclarationNeeded (i))
    {
      if (parallelLoop->isReductionRequired (i))
      {
        if (parallelLoop->isArray (i) || parallelLoop->isPointer (i))
        {
          variableDeclarations ->add (variableName,
              RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  variableName, buildArrayType (parallelLoop->getOpDatBaseType (
                      i), buildIntVal (parallelLoop->getOpDatDimension (i))),
                  subroutineScope));
        }
        else
        {
          variableDeclarations ->add (variableName,
              RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  variableName, parallelLoop->getOpDatBaseType (i),
                  subroutineScope));
        }
      }
      else
      {
        variableDeclarations ->add (variableName,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, buildArrayType (
                    parallelLoop->getOpDatBaseType (i), buildIntVal (
                        parallelLoop->getOpDatDimension (i))), subroutineScope));
      }
    }
  }
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createCUDAStageInVariablesVariableDeclarations ();

  createExecutionLocalVariableDeclarations ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating shared indirection mapping for OP_DAT " + lexical_cast <
                string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

        string const variableName = getIndirectionMapName (i);

        variableDeclarations->add (variableName,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, buildPointerType (buildIntType ()),
                subroutineScope));
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating indirection size argument for OP_DAT " + lexical_cast <
                string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

        string const variableName = getIndirectionArgumentSizeName (i);

        variableDeclarations->add (variableName,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, buildIntType (), subroutineScope));
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating shared memory pointer for OP_DAT "
                + lexical_cast <string> (i), Debug::INNER_LOOP_LEVEL, __FILE__,
            __LINE__);

        string const variableName = getIndirectionCUDASharedMemoryName (i);

        SgVariableDeclaration * variableDeclaration =
            RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, buildPointerType (parallelLoop->getOpDatBaseType (
                    i)), subroutineScope);

        variableDeclarations->add (variableName, variableDeclaration);

        indirectOpDatSharedMemoryDeclarations[parallelLoop->getOpDatVariableName (
            i)] = variableDeclaration;
      }
    }
    else
    {
      Debug::getInstance ()->debugMessage (
          "New shared memory declaration NOT needed for OP_DAT "
              + lexical_cast <string> (i), Debug::INNER_LOOP_LEVEL, __FILE__,
          __LINE__);

      string const variableName = getIndirectionCUDASharedMemoryName (i);

      variableDeclarations ->add (
          variableName,
          indirectOpDatSharedMemoryDeclarations[parallelLoop->getOpDatVariableName (
              i)]);
    }
  }

  if (parallelLoop->hasIncrementedOpDats ())
  {
    createIncrementAccessLocalVariableDeclarations ();
  }
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createPlanFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating plan formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (
      pindSizes,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          pindSizes, buildPointerType (buildIntType ()), subroutineScope,
          formalParameters));

  variableDeclarations->add (
      pindOffs,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          pindOffs, buildPointerType (buildIntType ()), subroutineScope,
          formalParameters));

  variableDeclarations->add (
      pblkMap,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          pblkMap, buildPointerType (buildIntType ()), subroutineScope,
          formalParameters));

  variableDeclarations->add (
      poffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          poffset, buildPointerType (buildIntType ()), subroutineScope,
          formalParameters));

  variableDeclarations->add (
      pnelems,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          pnelems, buildPointerType (buildIntType ()), subroutineScope,
          formalParameters));

  variableDeclarations->add (
      pnthrcol,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          pnthrcol, buildPointerType (buildIntType ()), subroutineScope,
          formalParameters));

  variableDeclarations->add (
      pthrcol,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          pthrcol, buildPointerType (buildIntType ()), subroutineScope,
          formalParameters));

  variableDeclarations->add (
      blockOffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          blockOffset, buildIntType (), subroutineScope, formalParameters));

  variableDeclarations->add (
      blockID,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          blockID, buildIntType (), subroutineScope, formalParameters));
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createOpDatFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const variableName = getOpDatName (i);

      variableDeclarations->add (
          variableName,
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              variableName, buildPointerType (
                  parallelLoop->getOpDatBaseType (i)), subroutineScope,
              formalParameters));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        string const & variableName = getLocalToGlobalMappingName (i);

        variableDeclarations->add (
            variableName,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, buildPointerType (buildIntType ()),
                subroutineScope, formalParameters));
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      string const & variableName = getGlobalToLocalMappingName (i);

      variableDeclarations->add (
          variableName,
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              variableName, buildPointerType (buildShortType ()),
              subroutineScope, formalParameters));
    }
  }
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createFormalParameterDeclarations ()
{
  Debug::getInstance ()->debugMessage (
      "Creating formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  createOpDatFormalParameterDeclarations ();

  createPlanFormalParameterDeclarations ();
}

CPPOpenMPKernelSubroutineIndirectLoop::CPPOpenMPKernelSubroutineIndirectLoop (
    SgScopeStatement * moduleScope, CPPUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop) :
  CPPOpenMPKernelSubroutine (moduleScope, userSubroutine, parallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "Creating kernel subroutine of indirect loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
