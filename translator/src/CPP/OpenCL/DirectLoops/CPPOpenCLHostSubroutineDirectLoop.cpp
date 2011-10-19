#include <CPPOpenCLHostSubroutineDirectLoop.h>
#include <CPPOpenCLKernelSubroutineDirectLoop.h>
#include <CPPParallelLoop.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <Debug.h>
#include <CommonNamespaces.h>

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
}

SgStatement *
CPPOpenCLHostSubroutineDirectLoop::createKernelFunctionCallStatement ()
{
}

void
CPPOpenCLHostSubroutineDirectLoop::createOpenCLKernelInitialisationStatements ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildIntType;
  using SageBuilder::buildSizeOfOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageInterface::appendStatement;
  using std::string;

  string const OPWarpSizeVariableName = "OP_WARP_SIZE";
  string const maxFunctionName = "MAX";

  SgExpression * nblocks_ref = buildVarRefExp (variableDeclarations->get (
      OpenCL::CPP::blocksPerGrid));
  SgExpression * nthreads_ref = buildVarRefExp (variableDeclarations->get (
      OpenCL::CPP::threadsPerBlock));
  SgExpression * nthreadstot_ref = buildVarRefExp (variableDeclarations->get (
      OpenCL::CPP::totalThreads));
  SgExpression * nshared_ref = buildVarRefExp (variableDeclarations->get (
      OpenCL::CPP::sharedMemorySize));

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
  tempStatement = buildAssignStatement (nblocks_ref, buildIntVal (nblocks));

  appendStatement (tempStatement, subroutineScope);

  /*
   * ======================================================
   * nthread = 128 //TODO: change in order to wrap with OP_BLOCK_SIZE (how?)
   * ======================================================
   */

  tempStatement = buildAssignStatement (nthreads_ref, buildIntVal (nthreads));

  appendStatement (tempStatement, subroutineScope);

  /*
   * ======================================================
   * ntotthread = nblocks * nthread
   * ======================================================
   */

  tempStatement = buildAssignStatement (nthreadstot_ref, buildMultiplyOp (
      nblocks_ref, nthreads_ref));

  appendStatement (tempStatement, subroutineScope);

  /* ======================================================
   * nshared = 0
   * ======================================================
   */
  tempStatement = buildAssignStatement (nshared_ref, buildIntVal (0));

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
    SgExpression * dimN_val = buildIntVal (parallelLoop->getOpDatDimension (i));

    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->getOpDatDimension (i) > 1 && parallelLoop->isGlobal (i)
          == false)
      {
        /*
         * ======================================================
         * nshared = MAX(nshared, size * dim)
         * ======================================================
         */

        tempStatement = buildAssignStatement (nshared_ref,
            buildFunctionCallExp (maxFunctionName, buildIntType (),
                buildExprListExp (nshared_ref, buildMultiplyOp (buildSizeOfOp (
                    parallelLoop->getOpDatType (i)), dimN_val))));

        appendStatement (tempStatement, subroutineScope);
      }
    }
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

  appendStatement (createKernelFunctionCallStatement (), subroutineScope);

  //appendStatement (createThreadSynchroniseCallStatement (), subroutineScope);

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionEpilogueStatements ();
  }
}

void
CPPOpenCLHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  createOpenCLKernelLocalVariableDeclarationsForDirectLoop ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionDeclarations ();
  }
}

CPPOpenCLHostSubroutineDirectLoop::CPPOpenCLHostSubroutineDirectLoop (
    SgScopeStatement * moduleScope,
    CPPOpenCLKernelSubroutine * kernelSubroutine,
    CPPParallelLoop * parallelLoop) :
  CPPOpenCLHostSubroutine (moduleScope, kernelSubroutine, parallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "Creating OpenCL host subroutine for direct loop",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
