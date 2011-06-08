#include <FortranOpenMPHostSubroutine.h>
#include <CommonNamespaces.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

void
FortranOpenMPHostSubroutine::initialiseNumberOfThreads ()
{
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("omp_get_max_threads",
          subroutineScope);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      buildExprListExp ());

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (variableDeclarations[OpenMP::numberOfThreads]),
      functionCall);

  appendStatement (assignmentStatement1, subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (variableDeclarations[OpenMP::numberOfThreads]),
      buildIntVal (1));

  appendStatement (assignmentStatement2, subroutineScope);

  addTextForUnparser (assignmentStatement1, "#ifdef _OPENMP\n",
      AstUnparseAttribute::e_after);

  addTextForUnparser (assignmentStatement2, "#else\n",
      AstUnparseAttribute::e_before);

  addTextForUnparser (assignmentStatement2, "#endif\n",
      AstUnparseAttribute::e_after);
}

FortranOpenMPHostSubroutine::FortranOpenMPHostSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    std::string const & kernelSubroutineName, ParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  FortranHostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop, moduleScope)
{
}
