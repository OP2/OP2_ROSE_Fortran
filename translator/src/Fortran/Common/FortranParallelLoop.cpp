#include <FortranParallelLoop.h>
#include <rose.h>

unsigned int
FortranParallelLoop::getNumberOfOpDatArgumentGroups ()
{
  return (functionCallExpression->get_args ()->get_expressions ().size ()
      - NUMBER_OF_NON_OP_DAT_ARGUMENTS) / NUMBER_OF_ARGUMENTS_PER_OP_DAT;
}

FortranParallelLoop::FortranParallelLoop (
    SgFunctionCallExp * functionCallExpression, std::string fileName) :
  ParallelLoop (functionCallExpression, fileName)
{
}
