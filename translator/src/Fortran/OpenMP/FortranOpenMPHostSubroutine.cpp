#include <FortranOpenMPHostSubroutine.h>
#include <CommonNamespaces.h>

FortranOpenMPHostSubroutine::FortranOpenMPHostSubroutine (
    std::string const & subroutineName, Subroutine * userSubroutine,
    Subroutine * kernelSubroutine, ParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  FortranHostSubroutine (subroutineName + SubroutineNameSuffixes::hostSuffix,
      userSubroutine, parallelLoop)
{
  using SageBuilder::buildFunctionParameterList;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildProcedureHeaderStatement;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  this->kernelSubroutine = kernelSubroutine;

  formalParameters = buildFunctionParameterList ();

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  appendStatement (subroutineHeaderStatement, moduleScope);

  addTextForUnparser (subroutineHeaderStatement, "attributes(host) ",
      AstUnparseAttribute::e_before);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();
}
