#include <FortranParallelLoop.h>
#include <rose.h>

bool
FortranParallelLoop::isCardinalityDeclarationNeeded (
    unsigned int OP_DAT_ArgumentGroup)
{
  /*
   * ======================================================
   * The only time we do NOT need this declaration is when:
   * a) The data is OP_GBL
   * b) The data is scalar
   * c) The data is read
   *
   * This is because, in this case, there is no transferal of
   * OP_GBL data into device memory. The data is instead
   * passed by value directly to the CUDA kernel
   * ======================================================
   */

  return isDirect (OP_DAT_ArgumentGroup) || isIndirect (OP_DAT_ArgumentGroup)
      || isReductionRequired (OP_DAT_ArgumentGroup) || (isGlobalArray (
      OP_DAT_ArgumentGroup) && isRead (OP_DAT_ArgumentGroup));
}

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
