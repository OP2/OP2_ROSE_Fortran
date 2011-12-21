#include "OpenCL.h"
#include <rose.h>

SgType *
OpenCL::getKernelType (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildOpaqueType ("cl_kernel", scope);
}

SgType *
OpenCL::getEventType (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildOpaqueType ("cl_event", scope);
}

SgType *
OpenCL::getMemoryType (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildOpaqueType ("cl_mem", scope);
}

SgType *
OpenCL::getSizeType (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildOpaqueType ("size_t", scope);
}

SgType *
OpenCL::getSignedShortType (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildOpaqueType ("cl_short", scope);
}

SgType *
OpenCL::getUnsignedShortType (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildOpaqueType ("cl_ushort", scope);
}

SgType *
OpenCL::getSignedIntegerType (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildOpaqueType ("cl_int", scope);
}

SgType *
OpenCL::getUnsignedIntegerType (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildOpaqueType ("cl_uint", scope);
}

SgType *
OpenCL::getSignedLongType (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildOpaqueType ("cl_long", scope);
}

SgType *
OpenCL::getUnsignedLongType (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildOpaqueType ("cl_ulong", scope);
}

SgType *
OpenCL::getSinglePrecisionFloatType (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildOpaqueType ("cl_float", scope);
}

SgType *
OpenCL::getDoublePrecisionFloatType (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildOpaqueType ("cl_double", scope);
}

SgFunctionCallExp *
OpenCL::getSetKernelArgumentCallExpression (SgScopeStatement * scope,
    SgVarRefExp * openCLKernel, int argumentIndex, SgType * sizeOfArgument,
    SgExpression * argument)
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (openCLKernel);

  actualParameters->append_expression (buildIntVal (argumentIndex));

  actualParameters->append_expression (buildSizeOfOp (sizeOfArgument));

  if (argument == NULL)
  {
    actualParameters->append_expression (buildIntVal (0));
  }
  else
  {
    actualParameters->append_expression (buildAddressOfOp (argument));
  }

  return buildFunctionCallExp ("clSetKernelArg", buildIntType (),
      actualParameters, scope);
}

SgFunctionCallExp *
OpenCL::getEnqueueKernelCallExpression (SgScopeStatement * scope,
    SgVarRefExp * commandQueue, SgVarRefExp * openCLKernel,
    SgVarRefExp * globalWorkSize, SgVarRefExp * localWorkSize,
    SgVarRefExp * event)
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (commandQueue);

  actualParameters->append_expression (openCLKernel);

  actualParameters->append_expression (buildIntVal (1));

  actualParameters->append_expression (buildIntVal (0));

  actualParameters->append_expression (buildAddressOfOp (globalWorkSize));

  actualParameters->append_expression (buildAddressOfOp (localWorkSize));

  actualParameters->append_expression (buildIntVal (0));

  actualParameters->append_expression (buildIntVal (0));

  actualParameters->append_expression (buildAddressOfOp (event));

  return buildFunctionCallExp ("clEnqueueNDRangeKernel", buildIntType (),
      actualParameters, scope);
}

SgFunctionCallExp *
OpenCL::getFinishCommandQueueCallExpression (SgScopeStatement * scope,
    SgVarRefExp * commandQueue)
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (commandQueue);

  return buildFunctionCallExp ("clFinish", buildIntType (), actualParameters,
      scope);
}

SgFunctionCallExp *
OpenCL::getLocalWorkGroupSizeCallStatement (SgScopeStatement * scope,
    SgExpression * expression)
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp ();

  if (expression == NULL)
  {
    actualParameters->append_expression (buildIntVal (0));
  }
  else
  {
    actualParameters->append_expression (expression);
  }

  return buildFunctionCallExp ("get_local_size", buildIntType (),
      actualParameters, scope);
}

SgFunctionCallExp *
OpenCL::getGlobalWorkGroupSizeCallStatement (SgScopeStatement * scope,
    SgExpression * expression)
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp ();

  if (expression == NULL)
  {
    actualParameters->append_expression (buildIntVal (0));
  }
  else
  {
    actualParameters->append_expression (expression);
  }

  return buildFunctionCallExp ("get_global_size", buildIntType (),
      actualParameters, scope);
}

SgFunctionCallExp *
OpenCL::getLocalWorkItemIDCallStatement (SgScopeStatement * scope,
    SgExpression * expression)
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp ();

  if (expression == NULL)
  {
    actualParameters->append_expression (buildIntVal (0));
  }
  else
  {
    actualParameters->append_expression (expression);
  }

  return buildFunctionCallExp ("get_local_id", buildIntType (),
      actualParameters, scope);
}

SgFunctionCallExp *
OpenCL::getWorkGroupIDCallStatement (SgScopeStatement * scope,
    SgExpression * expression)
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp ();

  if (expression == NULL)
  {
    actualParameters->append_expression (buildIntVal (0));
  }
  else
  {
    actualParameters->append_expression (expression);
  }

  return buildFunctionCallExp ("get_global_id", buildIntType (),
      actualParameters, scope);
}

SgFunctionCallExp *
OpenCL::createWorkItemsSynchronisationCallStatement (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (buildOpaqueVarRefExp (
      CLK_LOCAL_MEM_FENCE, scope));

  return buildFunctionCallExp ("barrier", buildVoidType (), actualParameters,
      scope);
}

SgFunctionCallExp *
OpenCL::OP2RuntimeSupport::getKernel (SgScopeStatement * scope,
    std::string const & kernelName)
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (buildStringVal (kernelName));

  return buildFunctionCallExp ("getKernel", buildVoidType (), actualParameters,
      scope);
}

SgFunctionCallExp *
OpenCL::OP2RuntimeSupport::getAssertMessage (SgScopeStatement * scope,
    SgExpression * assertExpression, SgStringVal * message)
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (assertExpression);

  actualParameters->append_expression (message);

  return buildFunctionCallExp ("assert_m", buildVoidType (), actualParameters,
      scope);
}

SgFunctionCallExp *
OpenCL::OP2RuntimeSupport::getReallocateReductionArraysCallStatement (
    SgScopeStatement * scope, SgVarRefExp * reductionBytesReference)
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp (reductionBytesReference);

  return buildFunctionCallExp ("reallocReductArrays", buildVoidType (),
      actualParameters, scope);
}

SgFunctionCallExp *
OpenCL::OP2RuntimeSupport::getMoveReductionArraysFromHostToDeviceCallStatement (
    SgScopeStatement * scope, SgVarRefExp * reductionBytesReference)
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp (reductionBytesReference);

  return buildFunctionCallExp ("mvReductArraysToDevice", buildVoidType (),
      actualParameters, scope);
}

SgFunctionCallExp *
OpenCL::OP2RuntimeSupport::getMoveReductionArraysFromDeviceToHostCallStatement (
    SgScopeStatement * scope, SgVarRefExp * reductionBytesReference)
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp (reductionBytesReference);

  return buildFunctionCallExp ("mvReductArraysToHost", buildVoidType (),
      actualParameters, scope);
}

SgVarRefExp *
OpenCL::OP2RuntimeSupport::getPointerToMemoryAllocatedForHostReductionArray (
    SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildOpaqueVarRefExp ("OP_reduct_h", scope);
}

SgVarRefExp *
OpenCL::OP2RuntimeSupport::getPointerToMemoryAllocatedForDeviceReductionArray (
    SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildOpaqueVarRefExp ("OP_reduct_d", scope);
}
