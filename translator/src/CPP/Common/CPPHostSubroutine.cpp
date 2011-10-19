#include <CPPHostSubroutine.h>
#include <CPPParallelLoop.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <CommonNamespaces.h>

void
CPPHostSubroutine::createFormalParameterDeclarations ()
{
  using SageBuilder::buildClassDeclaration;
  using SageBuilder::buildCharType;
  using SageBuilder::buildPointerType;
  using SageBuilder::buildIntType;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating host subroutine formal parameters", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Add name of user kernel
   * ======================================================
   */

  string const & kernelVariableName =
      OP2::VariableNames::getUserSubroutineName ();

  variableDeclarations->add (
      kernelVariableName,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          kernelVariableName, buildPointerType (buildCharType ()),
          subroutineScope, formalParameters));

  /*
   * ======================================================
   * Add OP_SET
   * ======================================================
   */

  string const & opSetVariableName = OP2::VariableNames::getOpSetName ();

  variableDeclarations->add (
      opSetVariableName,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          opSetVariableName, buildClassDeclaration (OP2::OP_SET,
              subroutineScope)->get_type (), subroutineScope, formalParameters));

  /*
   * ======================================================
   * Add OP_DAT, indirection, OP_MAP, access arguments for
   * each OP_DAT argument group
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & opDatvariableName = OP2::VariableNames::getOpDatName (i);

    variableDeclarations->add (
        opDatvariableName,
        RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            opDatvariableName, FortranTypesBuilder::buildClassDeclaration (
                OP2::OP_DAT, subroutineScope)->get_type (), subroutineScope,
            formalParameters));
  }
}

CPPHostSubroutine::CPPHostSubroutine (SgScopeStatement * moduleScope,
    Subroutine <SgFunctionDeclaration> * calleeSubroutine,
    CPPParallelLoop * parallelLoop) :
  HostSubroutine <SgFunctionDeclaration> (calleeSubroutine, parallelLoop)
{
  using SageBuilder::buildVoidType;
  using SageBuilder::buildDefiningFunctionDeclaration;
  using SageInterface::appendStatement;

  subroutineHeaderStatement = buildDefiningFunctionDeclaration (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  appendStatement (subroutineHeaderStatement, moduleScope);
}
