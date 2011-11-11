#include <CPPOpenMPKernelSubroutineDirectLoop.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CompilerGeneratedNames.h>
#include <OpenMP.h>
#include <Debug.h>

SgStatement *
CPPOpenMPKernelSubroutineDirectLoop::createUserSubroutineCallStatement ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;

  Debug::getInstance ()->debugMessage ("Creating call to user kernel",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDirect (i))
    {
      SgMultiplyOp * multiplyExpression = buildMultiplyOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              1)), buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgAddOp * addExpression = buildAddOp (variableDeclarations->getReference (
          getOpDatName (i)), multiplyExpression);

      actualParameters->append_expression (addExpression);
    }
    else if (parallelLoop->isReductionRequired (i))
    {
      actualParameters->append_expression (variableDeclarations->getReference (
          getOpDatName (i)));
    }
  }

  SgFunctionCallExp * functionCallExpression = buildFunctionCallExp (
      userSubroutine->getSubroutineName (), buildVoidType (), actualParameters,
      subroutineScope);

  return buildExprStatement (functionCallExpression);
}

void
CPPOpenMPKernelSubroutineDirectLoop::createExecutionLoopStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace OpenMP;

  Debug::getInstance ()->debugMessage (
      "Creating for loop statements for thread slice", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * loopBody = buildBasicBlock ();

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  appendStatement (createUserSubroutineCallStatement (), loopBody);

  /*
   * ======================================================
   * For loop statement
   * ======================================================
   */

  SgExprStatement * initialisationExpression = buildAssignStatement (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      variableDeclarations->getReference (sliceStart));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      variableDeclarations->getReference (sliceEnd));

  SgPlusPlusOp * strideExpression = buildPlusPlusOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

  SgForStatement * forLoopStatement = buildForStatement (
      initialisationExpression, buildExprStatement (upperBoundExpression),
      strideExpression, loopBody);

  appendStatement (forLoopStatement, subroutineScope);
}

void
CPPOpenMPKernelSubroutineDirectLoop::createStatements ()
{
  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createExecutionLoopStatements ();
}

void
CPPOpenMPKernelSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating host subroutine formal parameters", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (
      getIterationCounterVariableName (1),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1), buildIntType (), subroutineScope));
}

void
CPPOpenMPKernelSubroutineDirectLoop::createOpDatFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using std::string;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & opDatvariableName = getOpDatName (i);

    variableDeclarations->add (
        opDatvariableName,
        RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            opDatvariableName, buildPointerType (
                parallelLoop->getOpDatBaseType (i)), subroutineScope,
            formalParameters));
  }
}

void
CPPOpenMPKernelSubroutineDirectLoop::createFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace OpenMP;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating host subroutine formal parameters", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  createOpDatFormalParameterDeclarations ();

  variableDeclarations->add (sliceStart,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          sliceStart, buildIntType (), subroutineScope));

  variableDeclarations->add (sliceEnd,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (sliceEnd,
          buildIntType (), subroutineScope));
}

CPPOpenMPKernelSubroutineDirectLoop::CPPOpenMPKernelSubroutineDirectLoop (
    SgScopeStatement * moduleScope, CPPUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop) :
  CPPOpenMPKernelSubroutine (moduleScope, userSubroutine, parallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "Creating kernel subroutine of direct loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
