#include <CPPOpenCLKernelSubroutine.h>
#include <CPPOpenCLStatementsAndExpressionsBuilder.h>


void
CPPOpenCLKernelSubroutine::createReductionLoopStatements ()
{
  //FIXME
}

void
CPPOpenCLKernelSubroutine::createLocalThreadDeclarations ()
{
    using std::string;
    
    Debug::getInstance ()->debugMessage ( "creating Local Thread Declarations", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
    for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
    {
        string const & variableName = VariableNames::getOpDatLocalName (i);
        SgType * opDatType = parallelLoop->getOpDatType(i);
        variableDeclarations->add(
                                  variableName,
                                  CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
                                                                                                 variableName, 
                                                                                                 opDatType, 
                                                                                                 subroutineScope, 
                                                                                                 0, NULL));

    }
}

void
CPPOpenCLKernelSubroutine::createAutoSharedDeclaration ()
{
  //FIXME
}

CPPOpenCLKernelSubroutine::CPPOpenCLKernelSubroutine (
    std::string const & subroutineName, 
    std::string const & userSubroutineName,
    CPPParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope, 
    CPPReductionSubroutines * reductionSubroutines,
    CPPOpDatDimensionsDeclaration * opDatDimensionsDeclaration) :
  CPPKernelSubroutine (
      subroutineName, 
      userSubroutineName, 
      parallelLoop,
      moduleScope,
      reductionSubroutines),
  opDatDimensionsDeclaration( opDatDimensionsDeclaration )
{
  using SageInterface::addTextForUnparser;

  addTextForUnparser (subroutineHeaderStatement, "__kernel ", AstUnparseAttribute::e_before);
}
