#include <CPPOpenCLKernelSubroutineDirectLoop.h>
#include <CPPOpenCLUserSubroutine.h>
#include <CPPParallelLoop.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <RoseHelper.h>
#include <Debug.h>

SgStatement *
CPPOpenCLKernelSubroutineDirectLoop::createUserSubroutineCallStatement ()
{
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprListExp;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Creating call to user device subroutine", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);
  SgExprListExp * userDeviceSubroutineParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * argN_ref = buildVarRefExp (variableDeclarations->get (
        OP2::VariableNames::getOpDatName (i)));
    SgExpression * argN_l_ref = buildVarRefExp (variableDeclarations->get (
        OP2::VariableNames::getOpDatLocalName (i)));
    SgExpression * dimN_val = buildIntVal (parallelLoop->getOpDatDimension (i));

    int dim = parallelLoop->getOpDatDimension (i);

    SgExpression * parameterExpression = buildIntVal (1);

    if (parallelLoop->isGlobal (i))
    {
      if (parallelLoop->isRead (i))
      {
        /*
         * ======================================================
         * Case of global variable accessed in read mode:
         * we directly access the device variable, by
         * passing the kernel the variable name in positions
         * 0:argSize%<devVarName>-1
         * ======================================================
         */
        parameterExpression = argN_ref;
      }
      else
      {
        /*
         * ======================================================
         * Case of global variable accessed NOT in read mode:
         * we access the corresponding local thread variable
         * ======================================================
         */
        parameterExpression = argN_l_ref;
      }
    }

    userDeviceSubroutineParameters->append_expression (parameterExpression);
  }

  //needed to have the constants accessible in the user kernel
  SgExpression * global_constants_ref = buildVarRefExp (
      variableDeclarations->get (OpenCL::CPP::globalConstants));

  userDeviceSubroutineParameters->append_expression (global_constants_ref);

  return buildFunctionCallStmt (userSubroutine->getSubroutineName (),
      buildVoidType (), userDeviceSubroutineParameters, subroutineScope);
}

SgBasicBlock *
CPPOpenCLKernelSubroutineDirectLoop::createStageInFromDeviceMemoryToLocalThreadVariablesStatements ()
{
}

SgBasicBlock *
CPPOpenCLKernelSubroutineDirectLoop::createStageOutFromLocalThreadVariablesToDeviceMemoryStatements ()
{
}

void
CPPOpenCLKernelSubroutineDirectLoop::createExecutionLoopStatements ()
{
}

void
CPPOpenCLKernelSubroutineDirectLoop::createAutoSharedDisplacementInitialisationStatement ()
{
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
    }
  }
}

void
CPPOpenCLKernelSubroutineDirectLoop::createThreadIDInitialisationStatement ()
{
}

void
CPPOpenCLKernelSubroutineDirectLoop::createOpDatFormalParameterDeclarations ()
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildArrayType;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildPointerType;
  using std::string;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      /*
       * ======================================================
       * Obtain the base type of the OP_DAT argument
       * ======================================================
       */

      string const & variableName = OP2::VariableNames::getOpDatName (i);

      SgType * opDatType = parallelLoop->getOpDatType (i);

      SgArrayType * isArrayType = isSgArrayType (opDatType);

      SgType * opDatBaseType = isArrayType->get_base_type ();

      SgPointerType * pointerType = buildPointerType (opDatBaseType);

      variableDeclarations->add (
          variableName,
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              variableName, pointerType, subroutineScope, formalParameters));
    }
  }
}

void
CPPOpenCLKernelSubroutineDirectLoop::createInitialiseLocalThreadVariablesStatements ()
{
}

void
CPPOpenCLKernelSubroutineDirectLoop::createStatements ()
{
  createThreadIDInitialisationStatement ();

  createAutoSharedDisplacementInitialisationStatement ();

  createInitialiseLocalThreadVariablesStatements ();

  createExecutionLoopStatements ();

  if (parallelLoop->isReductionRequired () == true)
  {
  }
}

void
CPPOpenCLKernelSubroutineDirectLoop::createLocalVariableDeclarations ()
{
}

void
CPPOpenCLKernelSubroutineDirectLoop::createFormalParameterDeclarations ()
{
  using SageBuilder::buildIntType;

  /*
   * ======================================================
   * OP_DAT formal parameters
   * ======================================================
   */

  createOpDatFormalParameterDeclarations ();
}

CPPOpenCLKernelSubroutineDirectLoop::CPPOpenCLKernelSubroutineDirectLoop (
    SgScopeStatement * moduleScope, CPPOpenCLUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop,
    CPPReductionSubroutines * reductionSubroutines) :
  CPPOpenCLKernelSubroutine (moduleScope, userSubroutine, parallelLoop,
      reductionSubroutines)
{
  Debug::getInstance ()->debugMessage (
      "Creating OpenCL kernel subroutine for direct loop",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
