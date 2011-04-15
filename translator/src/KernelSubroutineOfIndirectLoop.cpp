#include <boost/lexical_cast.hpp>
#include <Debug.h>
#include <KernelSubroutineOfIndirectLoop.h>
#include <OP2CommonDefinitions.h>

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

void
KernelSubroutineOfIndirectLoop::create_OP_DAT_FormalParameters (
    ParallelLoop & parallelLoop)
{
  using boost::lexical_cast;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildPointerType;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters", 2);

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
      string const variableName = "argument" + lexical_cast <string> (i);

      SgVariableDeclaration * opDatDeclaration =
          buildVariableDeclaration (variableName, parallelLoop.get_OP_DAT_Type (
              i), NULL, subroutineScope);

      opDatDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
      opDatDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();

      appendStatement (opDatDeclaration, subroutineScope);

      formalParameters->append_arg (
          *(opDatDeclaration->get_variables ().begin ()));
    }
  }
}

void
KernelSubroutineOfIndirectLoop::createArgsSizesFormalParameter (
    DeviceDataSizesDeclaration & deviceDataSizesDeclaration)
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntType;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Creating OP_SET size formal parameter",
      2);

  formalParameter_argsSizes = buildVariableDeclaration (
      IndirectLoopVariables::argsSizes, deviceDataSizesDeclaration.getType (),
      NULL, subroutineScope);

  formalParameter_argsSizes->get_declarationModifier ().get_accessModifier ().setUndefined ();
  formalParameter_argsSizes->get_declarationModifier ().get_typeModifier ().setDevice ();

  formalParameters->append_arg (
      *(formalParameter_argsSizes->get_variables ().begin ()));

  appendStatement (formalParameter_argsSizes, subroutineScope);
}

KernelSubroutineOfIndirectLoop::KernelSubroutineOfIndirectLoop (
    std::string const & subroutineName,
    UserDeviceSubroutine & userDeviceSubroutine,
    DeviceDataSizesDeclaration & deviceDataSizesDeclaration,
    ParallelLoop & parallelLoop, SgScopeStatement * moduleScope) :
  KernelSubroutine (subroutineName)
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildProcedureHeaderStatement;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildFunctionParameterList;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;
  using std::string;
  using std::vector;

  formalParameters = buildFunctionParameterList ();

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  addTextForUnparser (subroutineHeaderStatement, "attributes(global) ",
      AstUnparseAttribute::e_before);

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  createArgsSizesFormalParameter (deviceDataSizesDeclaration);

  create_OP_DAT_FormalParameters (parallelLoop);
}
