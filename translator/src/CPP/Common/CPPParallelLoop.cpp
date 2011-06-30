#include <CPPParallelLoop.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <Debug.h>
#include <FortranCUDAReductionSubroutine.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <Globals.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
CPPParallelLoop::handleOpGblDeclaration (OpGblDeclaration * opGblDeclaration,
    std::string const & variableName, int opDatArgumentGroup)
{
}

void
CPPParallelLoop::handleOpDatDeclaration (OpDatDeclaration * opDatDeclaration,
    std::string const & variableName, int opDatArgumentGroup)
{
}

void
CPPParallelLoop::retrieveOpDatDeclarations (CPPDeclarations * declarations)
{
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

void
CPPParallelLoop::generateReductionSubroutines (SgScopeStatement * moduleScope)
{
}

unsigned int
CPPParallelLoop::getNumberOfOpDatArgumentGroups () const
{
  return 0;
}

CPPParallelLoop::CPPParallelLoop (SgExpressionPtrList & actualArguments,
    std::string userSubroutineName, CPPDeclarations * declarations) :
  ParallelLoop <SgFunctionDeclaration, CPPDeclarations> (actualArguments)
{
}
