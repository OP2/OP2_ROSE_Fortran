#include <CPPOpenCLHostSubroutineDirectLoop.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <Debug.h>
#include <CPPOpenCLReductionSubroutine.h>
#include <CPPOpenCLStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>

using namespace SageBuilder;
/*
 * ======================================================
 * Private functions
 * ======================================================
 */


void
CPPOpenCLHostSubroutineDirectLoop::createVariableSizesInitialisationStatements ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating OpenCL kernel prologue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);
  //FIXME: needed?
#if 0
  /*
   * ======================================================
   * In direct loops, sizes are only related to OP_DAT
   * variables
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * argN_size = buildVarRefExp(variableDeclarations->get(VariableNames::getOpDatName(i)));
    
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = VariableNames::getOpDatSizeName (i);

      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          moduleDeclarations->getDataSizesVariableDeclaration ()),
          buildOpaqueVarRefExp (variableName, subroutineScope));

      /*
       * ======================================================
       * The size value changes for reduction variables
       * ======================================================
       */

      if (parallelLoop->isReductionRequired (i) == false)
      {
        SgExprStatement * assignmentStatement = buildAssignStatement (
            dotExpression, buildVarRefExp (
                dataSizesDeclaration->getFieldDeclarations ()->get (
                    variableName)));

        appendStatement (assignmentStatement, subroutineScope);
      }
      else
      {
        SgExprStatement * assignmentStatement = buildAssignStatement (
            dotExpression, buildVarRefExp (variableDeclarations->get (
                ReductionSubroutine::numberOfThreadItems)));

        appendStatement (assignmentStatement, subroutineScope);
      }
    }
  }
#endif
}

void
CPPOpenCLHostSubroutineDirectLoop::createOpenCLKernelInitialisationStatements ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageInterface::appendStatement;
  using std::string;

  string const OPWarpSizeVariableName = "OP_WARP_SIZE";
  string const maxFunctionName = "MAX";
  
  
  SgExpression * nblocks_ref = buildVarRefExp ( variableDeclarations->get (OpenCL::CPP::blocksPerGrid) );
  SgExpression * nthreads_ref = buildVarRefExp ( variableDeclarations->get (OpenCL::CPP::threadsPerBlock));
  SgExpression * nthreadstot_ref = buildVarRefExp ( variableDeclarations->get (OpenCL::CPP::totalThreads));
  SgExpression * nshared_ref = buildVarRefExp (variableDeclarations->get (OpenCL::CPP::sharedMemorySize));
  SgExpression * reduct_size_ref = buildVarRefExp (variableDeclarations->get (ReductionSubroutine::reductionArraySize));
  SgExpression * offset_s_ref = buildVarRefExp (variableDeclarations->get (DirectLoop::CPP::KernelSubroutine::warpScratchpadSize));

  /*
   * ======================================================
   * The following values are copied from Mike Giles'
   * implementation and may be subject to future changes
   * ======================================================
   */
  int const nblocks = 200;
  int const nthreads = 128;
  
  SgExprStatement * tempStatement = NULL;

  /*
   * ======================================================
   * nblocks = 200
   * ======================================================
   */
  tempStatement = buildAssignStatement (
      nblocks_ref, 
      buildIntVal (nblocks) );

  appendStatement ( tempStatement, subroutineScope );

  /*
   * ======================================================
   * nthread = 128 //TODO: change in order to wrap with OP_BLOCK_SIZE (how?)
   * ======================================================
   */
  
  tempStatement = buildAssignStatement (
      nthreads_ref, 
      buildIntVal (nthreads) );

  appendStatement ( tempStatement, subroutineScope );
  
  /*
   * ======================================================
   * ntotthread = nblocks * nthread
   * ======================================================
   */
  
  tempStatement = buildAssignStatement (
      nthreadstot_ref, 
      buildMultiplyOp(
          nblocks_ref,
          nthreads_ref ) );

  appendStatement ( tempStatement, subroutineScope );
  

  /* ======================================================
   * nshared = 0
   * ======================================================
   */
  tempStatement = buildAssignStatement (
      nshared_ref, 
      buildIntVal (0) );

  appendStatement (tempStatement, subroutineScope);

  /*
   * ======================================================
   * Computing value for nshared: an input OP_DAT is copied
   * to shared memory only if its dimension is larger than
   * 1 or it is not encapsulating global data.
   * Therefore, the maximum size of shared memory is
   * equal to maximum size * dimension OP_DAT copied on
   * shared memory, multiplied by the number of threads
   * ======================================================
   */
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * dimN_val = buildIntVal ( parallelLoop->getOpDatDimension (i));
    
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->getOpDatDimension (i) > 1
          && parallelLoop->getOpMapValue (i) != GLOBAL)
      {
        /*
         * ======================================================
         * nshared = MAX(nshared, size * dim)
         * ======================================================
         */
        
        tempStatement = buildAssignStatement (
            nshared_ref, 
            buildFunctionCallExp(
                maxFunctionName,
                buildIntType(),
                buildExprListExp(
                    nshared_ref,
                    buildMultiplyOp(
                        buildSizeOfOp(
                            parallelLoop->getOpDatType(i) ),
                        dimN_val ) ) ) );
        
        appendStatement (tempStatement, subroutineScope);
      }
    }
  }

  /*
   * ======================================================
   * offset_s = nshared * OP_WARP_SIZE
   * ======================================================
   */
  
  tempStatement = buildAssignStatement (
      offset_s_ref,
      buildMultiplyOp(
          nshared_ref,
          buildOpaqueVarRefExp (OPWarpSizeVariableName, subroutineScope) ) );

  appendStatement (tempStatement, subroutineScope);



  if ( parallelLoop->isReductionRequired() ) 
  {
    /*
     * ======================================================
     * nshared = MAX(nshared*nthreads, reduct_size*nthreads)
     * ======================================================
     */
    tempStatement = buildAssignStatement (
        nshared_ref,
        buildFunctionCallExp(
            maxFunctionName,
            buildIntType(),
            buildExprListExp(
                buildMultiplyOp(
                    nshared_ref,
                    nthreads_ref ),
                buildMultiplyOp(
                    reduct_size_ref,
                    nthreads_ref ) ) ) );
    
    appendStatement (tempStatement, subroutineScope);
  } 
  else 
  {
    /*
     * ======================================================
     * nshared = nshared * nthreads
     * ======================================================
     */
    tempStatement = buildAssignStatement (
        nshared_ref,
        buildMultiplyOp(
            nshared_ref,
            nthreads_ref ) );
    
    appendStatement (tempStatement, subroutineScope);
  }  
}

void
CPPOpenCLHostSubroutineDirectLoop::createOpenCLKernelLocalVariableDeclarationsForDirectLoop ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildIntType;
  using SageBuilder::buildAssignInitializer;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating OpenCL configuration parameters", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgVariableDeclaration * variableDeclaration1 = buildVariableDeclaration (
      DirectLoop::CPP::KernelSubroutine::offsetInThreadBlock,
      buildIntType(), buildAssignInitializer (
          buildIntVal (0), buildIntType ()), subroutineScope);

  variableDeclarations->add (
      DirectLoop::CPP::KernelSubroutine::offsetInThreadBlock,
      variableDeclaration1);

  variableDeclaration1->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variableDeclaration1, subroutineScope);

  SgVariableDeclaration * variableDeclaration2 = buildVariableDeclaration (
      DirectLoop::CPP::KernelSubroutine::warpSize,
      buildIntType(), buildAssignInitializer (
          buildIntVal (0), buildIntType ()), subroutineScope);

  variableDeclarations->add (DirectLoop::CPP::KernelSubroutine::warpSize,
      variableDeclaration2);

  variableDeclaration2->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variableDeclaration2, subroutineScope);
}

SgStatement *
CPPOpenCLHostSubroutineDirectLoop::createKernelFunctionCallStatement ()
{
    using SageBuilder::buildVariableDeclaration;
    using SageBuilder::buildOpaqueType;
    using SageBuilder::buildIntType;
    using SageBuilder::buildFunctionCallExp;
    using SageInterface::appendStatement;
    using SageBuilder::buildAssignInitializer;
    using SageBuilder::buildExprListExp;
    using SageBuilder::buildStringVal;
    using SageBuilder::buildVarRefExp;
    using SageBuilder::buildOpaqueVarRefExp;
    using SageBuilder::buildAssignStatement;
    using SageBuilder::buildPlusPlusOp;
    
    /* cl_int ciErrNum = CL_SUCCESS */
    SgType* cl_int_type = buildOpaqueType("cl_int", subroutineScope);
    SgVariableDeclaration* ciErrNumDeclaration = buildVariableDeclaration(
                                                                          "ciErrNum",
                                                                          cl_int_type,
                                                                          buildAssignInitializer(
                                                                                                 buildOpaqueVarRefExp(SgName("CL_SUCCESS"),
                                                                                                                subroutineScope),
                                                                                                 cl_int_type),
                                                                          subroutineScope);
    variableDeclarations->add ("ciErrNum", ciErrNumDeclaration);
    appendStatement (ciErrNumDeclaration, subroutineScope);

    
    SgType* cl_event_type = buildOpaqueType("cl_event", subroutineScope);
    SgVariableDeclaration* ceEventDeclaration = buildVariableDeclaration(
                                                                          "ceEvent",
                                                                          cl_event_type,
                                                                          NULL,
                                                                          subroutineScope);
    variableDeclarations->add ("ceEvent", ceEventDeclaration);
    appendStatement (ceEventDeclaration, subroutineScope);

    
    
    /* cl_kernel hkernel = getKernel("kernel_name"); */
    SgType* cl_kernel_type = buildOpaqueType("cl_kernel", subroutineScope);
    SgVariableDeclaration* kernelDeclaration = buildVariableDeclaration(
                                                                        "hKernel",
                                                                        cl_kernel_type,
                                                                        buildAssignInitializer(
                                                                                               buildFunctionCallExp (
                                                                                                                     SgName("getKernel"),
                                                                                                                     cl_kernel_type,
                                                                                                                     buildExprListExp(buildStringVal ("kernel_name")),
                                                                                                                     subroutineScope),
                                                                                                                     cl_kernel_type),
                                                                        subroutineScope);
    variableDeclarations->add ("hKernel", kernelDeclaration);
    appendStatement (kernelDeclaration, subroutineScope);
    
    /* int _arg = 0; */
    SgVariableDeclaration* argDeclaration = buildVariableDeclaration(
                                                                     "_arg",
                                                                     buildIntType(),
                                                                     buildAssignInitializer(
                                                                                            buildIntVal (0),
                                                                                            buildIntType ()),
                                                                     subroutineScope);
    variableDeclarations->add ("_arg", argDeclaration);
    appendStatement (argDeclaration, subroutineScope);

    /* op_dat kernel set args */
    SgExpression* sof = buildSizeOfOp (buildOpaqueVarRefExp(SgName("cl_mem"), subroutineScope));
    SgExpression* hkernel = buildVarRefExp("hKernel", subroutineScope);
    SgExpression* _arginc = buildPlusPlusOp (buildVarRefExp ("_arg", subroutineScope), SgUnaryOp::postfix);
    
    for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
    {            
        if (parallelLoop->isDuplicateOpDat (i) == false)
        {
            
            SgExprStatement* argSetStat =
              buildExprStatement (
                                  buildIorAssignOp (
                                                    buildVarRefExp("ciErrNum", subroutineScope),
                                                    buildFunctionCallExp (
                                                                          SgName("clSetKernelArg"),
                                                                          cl_int_type,
                                                                          buildExprListExp(
                                                                                           hkernel,
                                                                                           _arginc,
                                                                                           sof,
                                                                                           buildAddressOfOp (
                                                                                                             buildDotExp(
                                                                                                                         buildVarRefExp(variableDeclarations->get(VariableNames::getOpDatName(i))),
                                                                                                                         buildOpaqueVarRefExp(SgName("data_d"), subroutineScope)))
                                                                                                     ),
                                                                                    subroutineScope)
                                                               ));
            appendStatement(argSetStat, subroutineScope);
        }
    }

    /* additionnal kernel arg setting */
    SgExprStatement* offset_s_arg =
    buildExprStatement (
                        buildIorAssignOp (
                                          buildVarRefExp("ciErrNum", subroutineScope),
                                          buildFunctionCallExp (
                                                                SgName("clSetKernelArg"),
                                                                cl_int_type,
                                                                buildExprListExp(
                                                                                 hkernel,
                                                                                 _arginc,
                                                                                 buildSizeOfOp (buildIntType ()),
                                                                                 buildAddressOfOp ( buildVarRefExp(variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::warpScratchpadSize)) )
                                                                                 ),
                                                                subroutineScope)
                                          ));
    
    appendStatement(offset_s_arg, subroutineScope);

    SgExprStatement* setsize_arg =
    buildExprStatement (
                        buildIorAssignOp (
                                          buildVarRefExp("ciErrNum", subroutineScope),
                                          buildFunctionCallExp (
                                                                SgName("clSetKernelArg"),
                                                                cl_int_type,
                                                                buildExprListExp(
                                                                                 hkernel,
                                                                                 _arginc,
                                                                                 buildSizeOfOp (buildIntType ()),
                                                                                 buildAddressOfOp (
                                                                                                   buildDotExp(
                                                                                                               buildVarRefExp(variableDeclarations->get(VariableNames::getOpSetName ())),
                                                                                                               buildOpaqueVarRefExp(SgName("size"), subroutineScope)))
                                                                                 ),
                                                                subroutineScope)
                                          ));
    
    appendStatement(setsize_arg, subroutineScope);
    
    SgExprStatement* nshared_arg =
    buildExprStatement (
                        buildIorAssignOp (
                                          buildVarRefExp("ciErrNum", subroutineScope),
                                          buildFunctionCallExp (
                                                                SgName("clSetKernelArg"),
                                                                cl_int_type,
                                                                buildExprListExp(
                                                                                 hkernel,
                                                                                 _arginc,
                                                                                 buildVarRefExp (variableDeclarations->get (OpenCL::CPP::sharedMemorySize)),
                                                                                 buildOpaqueVarRefExp(SgName("NULL"), subroutineScope)),
                                                                subroutineScope)
                                          ));
    
    appendStatement(nshared_arg, subroutineScope);    
    
    appendStatement (
                     buildExprStatement (
                                         buildFunctionCallExp (
                                                               SgName ("assert_m"),
                                                               buildIntType (), // is it the actual type ?
                                                               buildExprListExp(
                                                                                buildEqualityOp (
                                                                                                buildVarRefExp("ciErrNum", subroutineScope),
                                                                                                 buildOpaqueVarRefExp(SgName("CL_SUCCESS"),
                                                                                                                      subroutineScope)
                                                                                                 ),
                                                                                buildStringVal ("error setting kernel arguments")
                                                                                ),
                                                               subroutineScope
                                                               )
                                         ),                    
                     subroutineScope);
    
    // actual kernel enqueue call
    
    SgExprStatement* kcall = 
    buildExprStatement (
                        buildIorAssignOp (
                                          buildVarRefExp ("ciErrNum", subroutineScope),
                                          buildFunctionCallExp (
                                                                SgName("clEnqueueNDRangeKernel"),
                                                                cl_int_type,
                                                                buildExprListExp(
                                                                                 buildOpaqueVarRefExp(SgName("cqCommandQueue"), subroutineScope),
                                                                                 hkernel,
                                                                                 buildIntVal (1),
                                                                                 buildOpaqueVarRefExp (SgName ("NULL"), subroutineScope),
                                                                                 buildAddressOfOp (buildVarRefExp(OpenCL::CPP::totalThreads, subroutineScope)),
                                                                                 buildOpaqueVarRefExp (SgName ("NULL"), subroutineScope),
                                                                                 buildIntVal (0),
                                                                                 buildOpaqueVarRefExp (SgName ("NULL"), subroutineScope),
                                                                                 buildAddressOfOp (buildVarRefExp("ceEvent", subroutineScope))
                                                                                 ),
                                                                subroutineScope
                                                                )
                                          )
                        );
    appendStatement(kcall, subroutineScope);
    
    appendStatement (
                     buildExprStatement (
                                         buildFunctionCallExp (
                                                               SgName ("assert_m"),
                                                               buildIntType (), // is it the actual type ?
                                                               buildExprListExp(
                                                                                buildEqualityOp (
                                                                                                 buildVarRefExp("ciErrNum", subroutineScope),
                                                                                                 buildOpaqueVarRefExp(SgName("CL_SUCCESS"),
                                                                                                                      subroutineScope)
                                                                                                 ),
                                                                                buildStringVal ("error executing kernel")
                                                                                ),
                                                               subroutineScope
                                                               )
                                         ),                    
                     subroutineScope);
    
    SgExprStatement* kcallfinish =
    buildExprStatement (
                        buildIorAssignOp (
                                          buildVarRefExp ("ciErrNum", subroutineScope),
                                          buildFunctionCallExp (
                                                                SgName("clFinish"),
                                                                cl_int_type,
                                                                buildExprListExp (buildOpaqueVarRefExp(SgName("cqCommandQueue"), subroutineScope)),
                                                                subroutineScope
                                                                )
                                          )
                        );
    appendStatement(kcallfinish, subroutineScope);
                                                                                 
    appendStatement (
                     buildExprStatement (
                                         buildFunctionCallExp (
                                                               SgName ("assert_m"),
                                                               buildIntType (), // is it the actual type ?
                                                               buildExprListExp(
                                                                                buildEqualityOp (
                                                                                                 buildVarRefExp("ciErrNum", subroutineScope),
                                                                                                 buildOpaqueVarRefExp(SgName("CL_SUCCESS"),
                                                                                                                      subroutineScope)
                                                                                                 ),
                                                                                buildStringVal ("error completing device commands")
                                                                                ),
                                                               subroutineScope
                                                               )
                                         ),                    
                     subroutineScope);
    
    // Nicolas: no need for return value
    return NULL;
}

void
CPPOpenCLHostSubroutineDirectLoop::createStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildBoolValExp;
  using SageBuilder::buildEqualityOp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;



  createOpenCLKernelInitialisationStatements ();
  
  //FIXME

  //appendStatement (createTransferOpDatStatements (), subroutineScope);

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionPrologueStatements ();
  }

  createVariableSizesInitialisationStatements ();

  //appendStatement (createInitialiseConstantsCallStatement (), subroutineScope);

    createKernelFunctionCallStatement ();

  //appendStatement (createThreadSynchroniseCallStatement (), subroutineScope);

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionEpilogueStatements ();
  }
}

// Nicolas
void
CPPOpenCLHostSubroutineDirectLoop::createOpenCLKernelLocalVariableDeclarations ()
{
    // todo add to variableDeclarations (local scope)
    variableDeclarations->add(
                              OpenCL::CPP::blocksPerGrid,
                              CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
                                                                                             OpenCL::CPP::blocksPerGrid, 
                                                                                             buildIntType(), 
                                                                                             subroutineScope, 
                                                                                             0, NULL)
                              );

    variableDeclarations->add(
                              OpenCL::CPP::threadsPerBlock,
                              CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
                                                                                             OpenCL::CPP::threadsPerBlock, 
                                                                                             buildIntType(), 
                                                                                             subroutineScope, 
                                                                                             0, NULL)
                              );
    variableDeclarations->add(
                              OpenCL::CPP::totalThreads,
                              CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
                                                                                             OpenCL::CPP::totalThreads, 
                                                                                             buildIntType(), 
                                                                                             subroutineScope, 
                                                                                             0, NULL)
                              );
    variableDeclarations->add(
                              OpenCL::CPP::sharedMemorySize,
                              CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
                                                                                             OpenCL::CPP::sharedMemorySize, 
                                                                                             buildIntType(), 
                                                                                             subroutineScope, 
                                                                                             0, NULL)
                              );
    variableDeclarations->add(
                              ReductionSubroutine::reductionArraySize,
                              CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
                                                                                             ReductionSubroutine::reductionArraySize, 
                                                                                             buildIntType(), 
                                                                                             subroutineScope, 
                                                                                             0, NULL)
                              );
    variableDeclarations->add(
                              DirectLoop::CPP::KernelSubroutine::warpScratchpadSize,
                              CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
                                                                                             DirectLoop::CPP::KernelSubroutine::warpScratchpadSize, 
                                                                                             buildIntType(), 
                                                                                             subroutineScope, 
                                                                                             0, NULL)
                              );
}

void
CPPOpenCLHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  //createDataMarshallingLocalVariableDeclarations ();

  createOpenCLKernelLocalVariableDeclarations ();

  createOpenCLKernelLocalVariableDeclarationsForDirectLoop ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionLocalVariableDeclarations ();
  }
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

CPPOpenCLHostSubroutineDirectLoop::CPPOpenCLHostSubroutineDirectLoop (
    std::string const & subroutineName, 
    std::string const & userSubroutineName,
    std::string const & kernelSubroutineName,
    CPPParallelLoop * parallelLoop, 
    SgScopeStatement * moduleScope,
//    CPPInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
    CPPOpenCLDataSizesDeclarationDirectLoop * dataSizesDeclaration,
    CPPOpDatDimensionsDeclaration * opDatDimensionsDeclaration) :
//    CPPOpenCLModuleDeclarations * moduleDeclarations) :
  CPPOpenCLHostSubroutine (
      subroutineName, 
      userSubroutineName,
      kernelSubroutineName, 
      parallelLoop,
      moduleScope)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of direct loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
