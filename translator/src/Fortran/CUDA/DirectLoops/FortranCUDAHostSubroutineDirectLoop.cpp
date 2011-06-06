#include <boost/lexical_cast.hpp>
#include <FortranCUDAHostSubroutineDirectLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <ROSEHelper.h>
#include <Debug.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranCUDAHostSubroutineDirectLoop::createKernelCall ()
{
  using SageBuilder::buildDotExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVoidType;
  using SageInterface::appendStatement;
  using std::map;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating kernel call", 2);

  SgExprListExp * kernelParameters = buildExprListExp ();

  /*
   * ======================================================
   * argSizes
   * ======================================================
   */
  kernelParameters->append_expression (
      buildVarRefExp (
          localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]));

  /*
   * ======================================================
   * op_dat content arguments on device
   * ======================================================
   */
  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false)
    {

      /*
       * ======================================================
       * check if it is a reduction variable
       * ======================================================
       */
      if (parallelLoop->isReductionRequired (i) == false)
      {
        SgVarRefExp * opDatDeviceReference = buildVarRefExp (
            localVariableDeclarations[get_OP_DAT_DeviceVariableName (i)]);

        kernelParameters->append_expression (opDatDeviceReference);
      }
      else
      {
        SgVarRefExp
            * redArrayOnDeviceRef =
                buildVarRefExp (
                    localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionArrayDevice]);

        kernelParameters->append_expression (redArrayOnDeviceRef);
      }
    }
  }

  /*
   * ======================================================
   * offsetS
   * ======================================================
   */
  kernelParameters->append_expression (
      buildVarRefExp (
          localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock]));

  /*
   * ======================================================
   * set%size
   * ======================================================
   */
  SgExpression * opSetFormalArgumentReference = buildVarRefExp (
      formalParameterDeclarations[get_OP_SET_FormalParameterName ()]);

  SgExpression * sizeFieldExpression =
      buildDotExp (opSetFormalArgumentReference,
          buildOpaqueVarRefExp (
              IndirectAndDirectLoop::Fortran::HostSubroutine::size,
              subroutineScope));

  kernelParameters->append_expression (sizeFieldExpression);

  /*
   * ======================================================
   * warpSizeOP2
   * ======================================================
   */
  kernelParameters->append_expression (
      buildVarRefExp (
          localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]));

  /*
   * ======================================================
   * offset in shared memory for reductions (if needed)
   * ======================================================
   */
  if (parallelLoop->isReductionRequired () == true)
  {
    kernelParameters->append_expression (
        buildVarRefExp (
            localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset]));
  }

  SgExprStatement
      * kernelCall =
          buildFunctionCallStmt (
              kernelSubroutineName + "<<<"
                  + ROSEHelper::getFirstVariableName (
                      localVariableDeclarations[CUDA::Fortran::VariableNames::blocksPerGrid])
                  + ", "
                  + ROSEHelper::getFirstVariableName (
                      localVariableDeclarations[CUDA::Fortran::VariableNames::threadsPerBlock])
                  + ", "
                  + ROSEHelper::getFirstVariableName (
                      localVariableDeclarations[CUDA::Fortran::VariableNames::sharedMemorySize])
                  + ">>>", buildVoidType (), kernelParameters, subroutineScope);

  appendStatement (kernelCall, subroutineScope);
}

void
FortranCUDAHostSubroutineDirectLoop::initialiseDeviceVariablesSizesVariable ()
{
  using std::string;
  using boost::lexical_cast;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprStatement;
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * In direct loop sizes are only related to op_dat
   * variables
   * ======================================================
   */

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false)
    {

      string const & variableName =
          IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
              + lexical_cast <string> (i);

      string const sizeFieldExpression = variableName
          + IndirectAndDirectLoop::Fortran::VariableSuffixes::Size;

      SgExpression
          * sizeVariableField =
              buildDotExp (
                  buildVarRefExp (
                      localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]),
                  buildOpaqueVarRefExp (sizeFieldExpression, subroutineScope));

      SgVarRefExp * varRefExpression = NULL;

      /*
       * ======================================================
       * The size value changes for reduction variables
       * ======================================================
       */

      if (parallelLoop->isReductionRequired (i) == false)
      {
        varRefExpression = buildVarRefExp (
            localVariableDeclarations[get_OP_DAT_SizeVariableName (i)]);
      }
      else
      {
        varRefExpression
            = buildVarRefExp (
                localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::numberOfThreadItems]);
      }

      SgExpression * assignExpression = buildAssignOp (sizeVariableField,
          varRefExpression);

      appendStatement (buildExprStatement (assignExpression), subroutineScope);
    }
  }
}

void
FortranCUDAHostSubroutineDirectLoop::initialiseAllCUDAVariables ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageInterface::appendStatement;

  SgVarRefExp * variable_OP_WARP_SIZE = buildOpaqueVarRefExp ("OP_WARP_SIZE",
      subroutineScope);

  /*
   * ======================================================
   * initialising fixed value variables nblocks, nthreads
   * and warpSizeOP2
   * ======================================================
   */

  SgExpression * assignExpressionNblocks = buildAssignOp (buildVarRefExp (
      localVariableDeclarations[CUDA::Fortran::VariableNames::blocksPerGrid]),
      buildIntVal (CUDA::Fortran::DirectLoop::nblocks));

  SgExpression
      * assignExpressionNthreads =
          buildAssignOp (
              buildVarRefExp (
                  localVariableDeclarations[CUDA::Fortran::VariableNames::threadsPerBlock]),
              buildIntVal (CUDA::Fortran::DirectLoop::nthreads));

  SgExpression
      * assignExpressionWarpSizeOP2 =
          buildAssignOp (
              buildVarRefExp (
                  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]),
              variable_OP_WARP_SIZE);

  SgExpression
      * assignExpressionSharedMemorySize =
          buildAssignOp (
              buildVarRefExp (
                  localVariableDeclarations[CUDA::Fortran::VariableNames::sharedMemorySize]),
              buildIntVal (0));

  appendStatement (buildExprStatement (assignExpressionNblocks),
      subroutineScope);

  appendStatement (buildExprStatement (assignExpressionNthreads),
      subroutineScope);

  appendStatement (buildExprStatement (assignExpressionWarpSizeOP2),
      subroutineScope);

  appendStatement (buildExprStatement (assignExpressionSharedMemorySize),
      subroutineScope);

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
  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false)
    {
      int dim = parallelLoop->get_OP_DAT_Dimension (i);
      if (dim > 1 && parallelLoop->get_OP_MAP_Value (i) != GLOBAL)
      {

        /*
         * ======================================================
         * nshared = max ( nshared, size * dim )
         * ======================================================
         */

        SgFunctionSymbol * shiftFunctionSymbol =
            FortranTypesBuilder::buildNewFortranFunction ("max",
                subroutineScope);

        /*
         * ======================================================
         * Retrieving size from type of corresponding OP_DAT
         * declaration
         * ======================================================
         */
        SgExpression * opDatKindSize =
            FortranStatementsAndExpressionsBuilder::getFortranKindOf_OP_DAT (
                parallelLoop->get_OP_DAT_Type (i));

        SgExpression * secondParameterMaxCall = buildMultiplyOp (buildIntVal (
            dim), opDatKindSize);

        SgExprListExp
            * actualParameters =
                buildExprListExp (
                    buildVarRefExp (
                        localVariableDeclarations[CUDA::Fortran::VariableNames::sharedMemorySize]),
                    secondParameterMaxCall);

        SgFunctionCallExp * maxFunctionCall = buildFunctionCallExp (
            shiftFunctionSymbol, actualParameters);

        FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
            localVariableDeclarations[CUDA::Fortran::VariableNames::sharedMemorySize],
            maxFunctionCall, subroutineScope);
      }
    }
  }

  /*
   * ======================================================
   * 	offsetS = nshared * OP_WARP_SIZE
   * ======================================================
   */
  SgExpression
      * multiplyNsharedPerWarpSize =
          buildMultiplyOp (
              buildVarRefExp (
                  localVariableDeclarations[CUDA::Fortran::VariableNames::sharedMemorySize]),
              variable_OP_WARP_SIZE);

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock],
      multiplyNsharedPerWarpSize, subroutineScope);

  /*
   * ======================================================
   * nshared = nshared * nthreads
   * ======================================================
   */
  SgExpression
      * multiplyNsharedPerNthreads =
          buildMultiplyOp (
              buildVarRefExp (
                  localVariableDeclarations[CUDA::Fortran::VariableNames::sharedMemorySize]),
              buildVarRefExp (
                  localVariableDeclarations[CUDA::Fortran::VariableNames::threadsPerBlock]));

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      localVariableDeclarations[CUDA::Fortran::VariableNames::sharedMemorySize],
      multiplyNsharedPerNthreads, subroutineScope);
}

void
FortranCUDAHostSubroutineDirectLoop::createAdditionalCUDAKernelVariableDeclarations ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildIntType;
  using SageBuilder::buildAssignInitializer;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA configuration parameters", 2);

  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock]
      = buildVariableDeclaration (
          DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock,
          FortranTypesBuilder::getFourByteInteger (), buildAssignInitializer (
              buildIntVal (0), buildIntType ()), subroutineScope);

  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock]->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (
      localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock],
      subroutineScope);

  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]
      = buildVariableDeclaration (
          DirectLoop::Fortran::KernelSubroutine::warpSize,
          FortranTypesBuilder::getFourByteInteger (), buildAssignInitializer (
              buildIntVal (0), buildIntType ()), subroutineScope);

  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (
      localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize],
      subroutineScope);
}

void
FortranCUDAHostSubroutineDirectLoop::createStatements ()
{
  using SageInterface::appendStatement;

  initialiseAllCUDAVariables ();

  initialiseDataMarshallingVariables ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createSupportForReductionVariablesBeforeKernel ();
  }

  initialiseDeviceVariablesSizesVariable ();

  appendStatement (createThreadSynchroniseCall (), subroutineScope);

  if (parallelLoop->isReductionRequired () == true)
  {
    createSupportForReductionVariablesAfterKernel ();
  }

  copyDataBackFromDeviceAndDeallocate ();
}

void
FortranCUDAHostSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectAndDirectLoop::Fortran::VariableNames::argsSizes,
          dataSizesDeclarationOfDirectLoop->getType (), subroutineScope, 1,
          DEVICE);

  createDataMarshallingDeclarations ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionIterationVariableDeclarations ();
  }

  createCUDAKernelVariableDeclarations ();

  createAdditionalCUDAKernelVariableDeclarations ();

  createReductionVariableDeclarations ();
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

FortranCUDAHostSubroutineDirectLoop::FortranCUDAHostSubroutineDirectLoop (
    std::string const & subroutineName,
    std::string const & userSubroutineName,
    std::string const & kernelSubroutineName,
    FortranCUDADataSizesDeclarationDirectLoop * dataSizesDeclarationOfDirectLoop,
    ParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  FortranCUDAHostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop, moduleScope)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of direct loop", 2);

  this->dataSizesDeclarationOfDirectLoop = dataSizesDeclarationOfDirectLoop;

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
