#include <CPPKernelSubroutine.h>
#include <CPPParallelLoop.h>

CPPKernelSubroutine::CPPKernelSubroutine (SgScopeStatement * moduleScope,
    Subroutine <SgFunctionDeclaration> * calleeSubroutine,
    CPPParallelLoop * parallelLoop) :
  KernelSubroutine <SgFunctionDeclaration> (calleeSubroutine, parallelLoop)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  subroutineHeaderStatement = buildDefiningFunctionDeclaration (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  appendStatement (subroutineHeaderStatement, moduleScope);
}
