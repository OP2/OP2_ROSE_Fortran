#include <CPPTypesBuilder.h>
#include <RoseHelper.h>
#include <rose.h>

SgFunctionSymbol *
CPPTypesBuilder::buildFunctionDeclaration (
    std::string const & functionName, SgScopeStatement * scope)
{
  using namespace SageBuilder;

  SgFunctionType * functionType = buildFunctionType (buildVoidType (),
      new SgFunctionParameterTypeList ());

  SgFunctionRefExp * functionReference = buildFunctionRefExp (functionName,
      functionType, scope);

  return functionReference->get_symbol ();
}

SgFunctionSymbol *
CPPTypesBuilder::buildNewCPPSubroutine (
    std::string const & functionName, SgScopeStatement * scope)
{
  SgFunctionSymbol * functionSymbol = buildFunctionDeclaration (functionName,
      scope);

/*  SgProcedureHeaderStatement * procedureHeaderStatement =
      isSgProcedureHeaderStatement (functionSymbol->get_declaration ());

  procedureHeaderStatement->set_subprogram_kind (
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind);
*/
  return functionSymbol;
}           
