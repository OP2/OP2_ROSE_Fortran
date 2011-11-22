#include "CPPHostSubroutine.h"
#include "CPPParallelLoop.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "CompilerGeneratedNames.h"
#include "PlanFunctionNames.h"
#include "OP2.h"

SgBasicBlock *
CPPHostSubroutine::createInitialisePlanFunctionArrayStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;
  using std::map;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise plan function variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (opDatArray), buildIntVal (i - 1));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, variableDeclarations->getReference (getOpDatName (
            i)));

    appendStatement (assignmentStatement, block);
  }

  map <string, unsigned int> indirectOpDatsToIndirection;
  unsigned int indirection = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (indirectionDescriptorArray),
        buildIntVal (i - 1));

    SgExprStatement * assignmentStatement;

    if (parallelLoop->isDirect (i))
    {
      assignmentStatement = buildAssignStatement (arrayIndexExpression,
          buildIntVal (-1));
    }
    else if (parallelLoop->isIndirect (i))
    {
      if (parallelLoop->isDuplicateOpDat (i) == false)
      {
        assignmentStatement = buildAssignStatement (arrayIndexExpression,
            buildIntVal (indirection));

        indirectOpDatsToIndirection[parallelLoop->getOpDatVariableName (i)]
            = indirection;

        indirection++;
      }
      else
      {
        assignmentStatement = buildAssignStatement (arrayIndexExpression,
            buildIntVal (
                indirectOpDatsToIndirection[parallelLoop->getOpDatVariableName (
                    i)]));
      }
    }

    appendStatement (assignmentStatement, block);
  }

  return block;
}

void
CPPHostSubroutine::createFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating host subroutine formal parameters", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Add name of user kernel
   * ======================================================
   */

  string const & kernelVariableName = getUserSubroutineName ();

  SgVariableDeclaration
      * kernelVariableNameDeclaration =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              kernelVariableName, buildPointerType (buildConstType (
                  buildCharType ())), subroutineScope, formalParameters);

  variableDeclarations->add (kernelVariableName, kernelVariableNameDeclaration);

  /*
   * ======================================================
   * Add OP_SET
   * ======================================================
   */

  string const & opSetVariableName = getOpSetName ();

  variableDeclarations->add (
      opSetVariableName,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          opSetVariableName, buildOpaqueType (OP2::OP_SET, subroutineScope),
          subroutineScope, formalParameters));

  /*
   * ======================================================
   * Add OP_DAT, indirection, OP_MAP, access arguments for
   * each OP_DAT argument group
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & opDatvariableName = getOpDatName (i);

    variableDeclarations->add (
        opDatvariableName,
        RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            opDatvariableName, buildOpaqueType (OP2::OP_ARG, subroutineScope),
            subroutineScope, formalParameters));
  }
}

CPPHostSubroutine::CPPHostSubroutine (SgScopeStatement * moduleScope,
    Subroutine <SgFunctionDeclaration> * calleeSubroutine,
    CPPParallelLoop * parallelLoop) :
  HostSubroutine <SgFunctionDeclaration> (calleeSubroutine, parallelLoop)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  subroutineHeaderStatement = buildDefiningFunctionDeclaration (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  appendStatement (subroutineHeaderStatement, moduleScope);
}
