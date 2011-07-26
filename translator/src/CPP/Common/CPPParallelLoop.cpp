#include <CPPParallelLoop.h>

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
CPPParallelLoop::getNumberOfOpDatArgumentGroups ()
{
  return getActualArguments ().size () - NUMBER_OF_NON_OP_DAT_ARGUMENTS;
}

CPPParallelLoop::CPPParallelLoop (SgFunctionCallExp * functionCallExpression) :
  ParallelLoop <SgFunctionDeclaration> (functionCallExpression)
{
}
