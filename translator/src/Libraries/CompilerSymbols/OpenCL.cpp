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
    actualParameters->append_expression (buildNullExpression ());
  }
  else
  {
    actualParameters->append_expression (buildAddressOfOp (argument));
  }

  return buildFunctionCallExp ("clSetKernelArg", buildIntType (),
      actualParameters, scope);
}

SgFunctionCallExp *
OpenCL::getWorkGroupDimensionsCallStatement (SgScopeStatement * scope)
{
}

SgFunctionCallExp *
OpenCL::getLocalWorkGroupSizeCallStatement (SgScopeStatement * scope)
{
}

SgFunctionCallExp *
OpenCL::getGlobalWorkGroupSizeCallStatement (SgScopeStatement * scope)
{
}

SgFunctionCallExp *
OpenCL::getLocalWorkItemIDCallStatement (SgScopeStatement * scope)
{
}

SgFunctionCallExp *
OpenCL::getWorkGroupIDCallStatement (SgScopeStatement * scope)
{
}

SgFunctionCallExp *
OpenCL::getNumberOfWorkGroupsCallStatement (SgScopeStatement * scope)
{
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
    SgVarRefExp * successReference, SgStringVal * message)
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (successReference);

  actualParameters->append_expression (message);

  return buildFunctionCallExp ("assert_m", buildVoidType (), actualParameters,
      scope);
}
