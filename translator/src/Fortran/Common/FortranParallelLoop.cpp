#include <FortranParallelLoop.h>

unsigned int
FortranParallelLoop::getNumberOfOpDatArgumentGroups ()
{
  return (getActualArguments ().size () - NUMBER_OF_NON_OP_DAT_ARGUMENTS)
      / NUMBER_OF_ARGUMENTS_PER_OP_DAT;
}
