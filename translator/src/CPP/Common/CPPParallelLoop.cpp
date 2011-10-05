#include <CPPParallelLoop.h>
#include <rose.h>

unsigned int
CPPParallelLoop::getNumberOfOpDatArgumentGroups ()
{
  return functionCallExpression->get_args ()->get_expressions ().size ()
      - NUMBER_OF_NON_OP_DAT_ARGUMENTS;
}

CPPParallelLoop::CPPParallelLoop (SgFunctionCallExp * functionCallExpression) :
  ParallelLoop (functionCallExpression)
{
}
