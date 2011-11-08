#include <CPPOpenCLReductionSubroutine.h>
#include <Reduction.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <Debug.h>
#include <CommonNamespaces.h>
#include <OpenCL.h>

void
CPPOpenCLReductionSubroutine::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
}

void
CPPOpenCLReductionSubroutine::createLocalVariableDeclarations ()
{
}

void
CPPOpenCLReductionSubroutine::createFormalParameterDeclarations ()
{
  using namespace SageBuilder;
}

CPPOpenCLReductionSubroutine::CPPOpenCLReductionSubroutine (
    SgScopeStatement * moduleScope, Reduction * reduction) :
  Subroutine <SgFunctionDeclaration> (reduction->getSubroutineName ()),
      reduction (reduction)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating reduction subroutine",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  subroutineHeaderStatement = buildDefiningFunctionDeclaration (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      moduleScope);

  subroutineHeaderStatement->get_functionModifier ().setOpenclKernel ();

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
