#include <CPPOpenCLReductionSubroutine.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <Debug.h>
#include <CommonNamespaces.h>

void
CPPOpenCLReductionSubroutine::createLocalVariableDeclarations ()
{
}

void
CPPOpenCLReductionSubroutine::createFormalParameterDeclarations ()
{
  using SageBuilder::buildOpaqueType;
  using SageBuilder::buildIntType;
  using SageBuilder::buildPointerType;
  using SageBuilder::buildVolatileType;

  Debug::getInstance ()->debugMessage (
      "Creating reduction procedure formal parameter", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

CPPOpenCLReductionSubroutine::CPPOpenCLReductionSubroutine (
    std::string const & subroutineAndVariableName,
    SgScopeStatement * moduleScope, Reduction * reduction) :
  Subroutine <SgProcedureHeaderStatement> (subroutineAndVariableName),
      reduction (reduction)
{
  using SageBuilder::buildProcedureHeaderStatement;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildFunctionParameterList;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  Debug::getInstance ()->debugMessage ("Creating reduction subroutine",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  addTextForUnparser (subroutineHeaderStatement, "attributes(device) ",
      AstUnparseAttribute::e_before);

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
