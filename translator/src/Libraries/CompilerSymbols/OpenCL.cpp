


/*  Open source copyright declaration based on BSD open source template:
 *  http://www.opensource.org/licenses/bsd-license.php
 * 
 * Copyright (c) 2011-2012, Adam Betts, Carlo Bertolli
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "OpenCL.h"
#include <CPPTypesBuilder.h>
#include <rose.h>
#include <ctype.h>
#include <stdlib.h>

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
OpenCL::getAllocateConstantExpression(SgScopeStatement * scope,
      std::string constantName,  
      SgVarRefExp * constant, SgType * constantType)
{
  using namespace SageBuilder;
  using std::string;

  SgExprListExp * actualParameters = buildExprListExp ();
  
  actualParameters->append_expression (buildAddressOfOp (constant));

  SgType * type = constantType;


/*
 * here we want to extract the actual type (say, float or, "float [4UL]").
 * The class_name does not give what we want, and as we haven't found an 
 * alternative yet, we have done a horrid thing of hardcoding this case. 
 * TO BE FIXED!!!!
 */
  /*string typeName = constantType->class_name ();

  long position = typeName.find ("[");
  
  int factor = 0;

  if (position != string::npos) 
  {
   
    string new_type  = typeName.substr (position + 1);
  
    string::iterator it;

    for (it = new_type.begin (); it < new_type.end (); ++it)
    {
  
      if (!isdigit (*it))
      {
  
        new_type.erase (it);
  
      }

    }
   
    //factor = strtol (new_type, NULL, 10);
    factor = atoi (new_type.c_str ());

    if (type->isIntegerType ())
    {
      new_type = "int";          

    } else if (type->isFloatType ())
    {

      new_type = "float";

    }     


    type = buildOpaqueType (new_type, scope);

   SgMultiplyOp * factorMult =
        buildMultiplyOp (buildIntVal (factor), buildSizeOfOp (type, scope));
    
    actualParameters->append_expression (factorMult);

  } else 
  {

    actualParameters->append_expression (buildSizeOfOp (type, scope));

  }
*/

  string new_type = "";

  //if (constant->get_symbol ()->get_name ().getString ().compare("qinf_d") == 0)
  //if (constant->class_name ().compare("qinf_d") == 0)
  if (constantName.compare ("qinf_d") == 0)
  {
    
    new_type = "4 * sizeof(float)";

//    type = buildOpaqueType (new_type, scope);

    actualParameters->append_expression (buildOpaqueVarRefExp (new_type, scope));
  } else
  {

    actualParameters->append_expression (buildSizeOfOp (type));

  }

  return buildFunctionCallExp ("op_allocate_constant", 
      OpenCL::getMemoryType(scope), actualParameters, scope);
}

SgFunctionCallExp *
OpenCL::getSetKernelArgumentCallBufferExpression (SgScopeStatement * scope,
    SgVarRefExp * openCLKernel, int argumentIndex, SgExpression * bufferRef) 
{
  using namespace SageBuilder;
  
  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (openCLKernel);

  actualParameters->append_expression (buildIntVal (argumentIndex));

  actualParameters->append_expression (bufferRef);

  actualParameters->append_expression (buildOpaqueVarRefExp("NULL", scope));

  return buildFunctionCallExp ("clSetKernelArg", buildIntType (),
      actualParameters, scope);
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

  actualParameters->append_expression (buildOpaqueVarRefExp("NULL", scope));

  actualParameters->append_expression (buildAddressOfOp (globalWorkSize));

  actualParameters->append_expression (buildAddressOfOp (localWorkSize));

  actualParameters->append_expression (buildIntVal (0));

  actualParameters->append_expression (buildOpaqueVarRefExp("NULL", scope));

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


/*
 * Accordig to OpenCL 1.1 spec:
 * get_global_id: returns the unique global work-item ID
 * not the WorkGroupID
 */
SgFunctionCallExp *
OpenCL::getGlobalWorkItemIDCallStatement (SgScopeStatement * scope,
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

/*
 * According to OpenCL 1.1 spec:
 * get_group_id: returns the work-group ID
 */

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
  
  return buildFunctionCallExp ("get_group_id", buildIntType (),
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
/*
SgFunctionCallExp *
OpenCL::getOpTimer (SgScopeStatement * scope,
    SgVarRefExp * cpuTime, SgVarRefExp * wallTime)
{

  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp ();
 
  actualParameters->append_expression  (buildOpaqueVarRefExp (
            CLK_LOCAL_MEM_FENCE, scope)); 
//  actualParameters->append_expression (buildAddressOfOp (cpuTime));

//  actualParameters->append_expression (buildAddressOfOp (wallTime));

  return buildFunctionCallExp ("getkernel", buildVoidType (), actualParameters,
      scope);
}*/
/*
SgFunctionCallExp *
OpenCL::getOpTimerCallStatement (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp ();
    
  actualParameters->append_expression (buildOpaqueVarRefExp (
      "cpu_t1", scope));
  
  actualParameters->append_expression (buildOpaqueVarRefExp (
      "wall_t1", scope));

//  SgFunctionDeclaration * fctDecl = new SgFunctionDeclaration( "op_timers", buildVoidType (), 
  
//  SgFunctionRefExp * fctExp = buildFunctionRefExp (

  return buildFunctionCallExp ("op_timers", buildVoidType (), 
      actualParameters, scope);
}*/

SgFunctionCallExp * 
OpenCL::OP2RuntimeSupport::getOpTimerCallStatement (SgScopeStatement * scope,
    SgVarRefExp * cpuTime, SgVarRefExp * wallTime)
{
  using namespace SageBuilder;
  
  SgFunctionSymbol * functionSymbol = 
      CPPTypesBuilder::buildNewCPPSubroutine("op_timers", scope);
  
  SgExprListExp * actualParameters = buildExprListExp ();
  
  actualParameters->append_expression (cpuTime);
  
  actualParameters->append_expression (wallTime);

  return buildFunctionCallExp (functionSymbol, actualParameters);
}

SgFunctionCallExp *
OpenCL::OP2RuntimeSupport::getOpTimingReallocCallStatement (SgScopeStatement * scope,
  SgExpression * index) 
{
  using namespace SageBuilder;
  
  SgExprListExp * actualParameters = buildExprListExp ();
    
  if (index == NULL) {
    index = buildIntVal(0);
  }

  actualParameters->append_expression (index);
  
  return buildFunctionCallExp ("op_timing_realloc", buildVoidType (), actualParameters, 
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
