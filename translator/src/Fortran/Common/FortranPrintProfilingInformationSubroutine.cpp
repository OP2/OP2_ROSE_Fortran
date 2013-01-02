

#include <FortranProgramDeclarationsAndDefinitions.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranPrintProfilingInformationSubroutine.h>
#include <ParallelLoop.h>
#include <OP2.h>
#include <CompilerGeneratedNames.h>
#include <Globals.h>

void
FortranPrintProfilingInformationSubroutine::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace OP2::FortranSpecific::RunTimeFunctions;
  using std::string;
  using std::map;


  for (map <string, ParallelLoop *>::const_iterator it =
      parallelLoopsDeclarations->firstParallelLoop (); it
      != parallelLoopsDeclarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    string const postfixName = getPostfixNameAsConcatOfOpArgsNames (it->second);

    string const & variableNameHost = loopTimeHost +
      userSubroutineName + postfixName;
    string const & variableNameKernel = loopTimeKernel +
      userSubroutineName + postfixName;
    string const & numberCalledVariableName = numberCalled +
      userSubroutineName + postfixName;

    string const & printStatement1 = printStar + "'" +
      variableNameHost + " = ', " +
      variableNameHost + "\n";

    string const & printStatement2 = printStar + "'" +
      variableNameKernel + " = ', " +
      variableNameKernel + "\n";

    string const & printStatement3 = printStar + "'" +
      variableNameHost + " Average = ', " +
      variableNameHost + "/" + numberCalledVariableName + "\n";

    string const & printStatement4 = printStar + "'" +
      variableNameKernel + " Average = ', " +
      variableNameKernel + "/" + numberCalledVariableName + "\n";
    
    addTextForUnparser (subroutineScope, printStatement1,
      AstUnparseAttribute::e_after);

    addTextForUnparser (subroutineScope, printStatement2,
	AstUnparseAttribute::e_after);

    addTextForUnparser (subroutineScope, printStatement3,
	AstUnparseAttribute::e_after);

    addTextForUnparser (subroutineScope, printStatement4,
	AstUnparseAttribute::e_after);
  }
}

void
FortranPrintProfilingInformationSubroutine::createFormalParameterDeclarations ()
{

}

void
FortranPrintProfilingInformationSubroutine::createLocalVariableDeclarations ()
{

}

FortranPrintProfilingInformationSubroutine::FortranPrintProfilingInformationSubroutine (
  std::string subroutineName, SgScopeStatement * moduleScope,
  std::map <std::string, FortranModuleDeclarations *> _moduleDeclarations,
  FortranProgramDeclarationsAndDefinitions * _parallelLoopsDeclarations):
  Subroutine <SgProcedureHeaderStatement> (subroutineName), moduleDeclarations (_moduleDeclarations),
  parallelLoopsDeclarations (_parallelLoopsDeclarations)
{
  using namespace SageInterface;
  using namespace SageBuilder;

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  if ( Globals::getInstance ()->getTargetBackend () == TargetLanguage::CUDA )
    subroutineHeaderStatement->get_functionModifier ().setCudaHost ();
      
  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  appendStatement (subroutineHeaderStatement, moduleScope);

    implicitStatement = FortranStatementsAndExpressionsBuilder::buildImplicitNoneStatement ();

    //appendStatement (implicitStatement, subroutineScope);

  createStatements ();
}
