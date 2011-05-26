#include <boost/lexical_cast.hpp>
#include <Debug.h>
#include <KernelSubroutine.h>
#include <CommonNamespaces.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <ROSEHelper.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

std::string
KernelSubroutine::get_OP_DAT_VariableName (unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
      + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
KernelSubroutine::get_OP_DAT_SizeVariableName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
      + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + IndirectAndDirectLoop::Fortran::VariableSuffixes::Size;
}

std::string
KernelSubroutine::getLocalThread_OP_DAT_VariableName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
      + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + IndirectAndDirectLoop::Fortran::VariableSuffixes::local;
}

void
KernelSubroutine::createLocalThreadVariables ()
{
  using std::string;
  using boost::lexical_cast;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildArrayType;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildExprListExp;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Creating local thread variables", 2);

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if ((parallelLoop->isDirectLoop () == true
        && parallelLoop->get_OP_DAT_Dimension (i) > 1
        && parallelLoop->get_OP_MAP_Value (i) == DIRECT)
        || (parallelLoop->get_OP_MAP_Value (i) == GLOBAL
            && parallelLoop->get_OP_Access_Value (i) != READ_ACCESS)
        || (parallelLoop->get_OP_MAP_Value (i) == INDIRECT
            && parallelLoop->get_OP_Access_Value (i) == INC_ACCESS))
    {
      SgType * opDatBaseType = parallelLoop->get_OP_DAT_Type (i);

      SgArrayType * arrayType = isSgArrayType (opDatBaseType);

      ROSE_ASSERT (arrayType != NULL);

      opDatBaseType = arrayType->get_base_type ();

      localVariableDeclarations[getLocalThread_OP_DAT_VariableName (i)]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              getLocalThread_OP_DAT_VariableName (i),
              FortranTypesBuilder::getArray_RankOne (opDatBaseType, 0,
                  parallelLoop->get_OP_DAT_Dimension (i) - 1), subroutineScope);
    }
  }
}

void
KernelSubroutine::createAutosharedVariable ()
{
  SgType * autosharedType = NULL;

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false)
    {
      SgType * opDatArrayType = parallelLoop->get_OP_DAT_Type (i);

      SgArrayType * isArrayType = isSgArrayType (opDatArrayType);

      ROSE_ASSERT (isArrayType != NULL);

      SgType * opDatBaseType = isArrayType->get_base_type ();

      SgType * isRealType = isSgTypeFloat (opDatBaseType);

      /*
       * ======================================================
       * We copy the user type with kind (if exists) otherwise
       * we assume real(4) (CUDA Fortran specific)
       * ======================================================
       */

      if (isRealType != NULL)
      {
        autosharedType = opDatBaseType;
      }
    }
  }

  if (autosharedType != NULL)
  {
    /*
     * ======================================================
     * At least one of OP_DAT variables is of type REAL,
     * hence declare the autoshared variable
     * ======================================================
     */
    SgExpression * upperBound = new SgAsteriskShapeExp (
        ROSEHelper::getFileInfo ());

    localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]
        = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            IndirectAndDirectLoop::Fortran::VariableNames::autoshared,
            FortranTypesBuilder::getArray_RankOne (autosharedType, 0,
                upperBound), subroutineScope);

    localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]->get_declarationModifier ().get_typeModifier ().setShared ();
  }
}

void
KernelSubroutine::initialiseLocalThreadVariables ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildAddOp;
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * We essentially do a similar thing to the one we make
   * when we declare local thread variables
   * ======================================================
   */

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    int dim = parallelLoop->get_OP_DAT_Dimension (i);

    if (parallelLoop->get_OP_MAP_Value (i) == GLOBAL
        && parallelLoop->get_OP_Access_Value (i) != READ_ACCESS)
    {
      /*
       * ======================================================
       * This condition only changes the loop body, not its
       * lower and upper bounds and stride
       * ======================================================
       */
      SgVarRefExp * variable_X = buildOpaqueVarRefExp (
          CUDA::Fortran::FieldNames::x, subroutineScope);

      SgVarRefExp * variable_Blockidx = buildOpaqueVarRefExp (
          CUDA::Fortran::VariableNames::blockidx, subroutineScope);

      SgBasicBlock * loopBodyBlock;

      SgExpression
          * accessToIPosition =
              buildPntrArrRefExp (
                  buildVarRefExp (
                      localVariableDeclarations[getLocalThread_OP_DAT_VariableName (
                          i)]),
                  buildVarRefExp (
                      localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::setElementCounter]));

      if (parallelLoop->get_OP_Access_Value (i) == INC_ACCESS)
      {
        // for (int d=0; d<DIM; d++) ARG_l[d]=ZERO_TYP;
        SgExpression * assignArgToZero = buildAssignOp (accessToIPosition,
            buildIntVal (0));

        loopBodyBlock = buildBasicBlock (buildExprStatement (assignArgToZero));
      }
      else
      {
        //for (int d=0; d<DIM; d++) ARG_l[d]=ARG[d+blockIdx.x*DIM];
        SgExpression * blockIdxPerDim = buildMultiplyOp (buildDotExp (
            variable_Blockidx, variable_X), buildIntVal (dim));

        SgExpression
            * arrayAccessComplexExpr =
                buildAddOp (
                    buildVarRefExp (
                        localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::setElementCounter]),
                    blockIdxPerDim);

        SgExpression * complexAccessToArg = buildPntrArrRefExp (buildVarRefExp (
            localVariableDeclarations[getLocalThread_OP_DAT_VariableName (i)]),
            arrayAccessComplexExpr);

        SgExpression * assignArgToComplexAccess = buildAssignOp (
            accessToIPosition, complexAccessToArg);

        loopBodyBlock = buildBasicBlock (buildExprStatement (
            assignArgToComplexAccess));
      }

      /*
       * ======================================================
       * Remember that local thread variables are accessed
       * in the C-like notation for arrays: 0:N-1
       * ======================================================
       */

      SgExpression
          * initializationExpression =
              buildAssignOp (
                  buildVarRefExp (
                      localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::setElementCounter]),
                  buildIntVal (0));

      SgExpression * upperBoundExpression = buildIntVal (dim - 1);

      /*
       * ======================================================
       * The stride of the loop counter is 1
       * ======================================================
       */
      SgExpression * strideExpression = buildIntVal (1);

      /*
       * ======================================================
       * Add the do-loop statement
       * ======================================================
       */

      SgFortranDo * fortranDoStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              initializationExpression, upperBoundExpression, strideExpression,
              loopBodyBlock);

      appendStatement (fortranDoStatement, subroutineScope);
    }
  }
}

void
KernelSubroutine::createAndAppendReductionSubroutineCall ()
{
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildVarRefExp;
  using SageInterface::appendStatement;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage ("Adding reduction subroutine call", 2);

  if (parallelLoop->isReductionRequired () == true)
  {
    for (unsigned int i = 1; i
        <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
    {
      if (parallelLoop->isReductionRequired (i) == true)
      {
        int dim = parallelLoop->get_OP_DAT_Dimension (i);

        switch (parallelLoop->get_OP_Access_Value (i))
        {
          case INC_ACCESS:
          {

            SgExpression
                * reductInitLoop =
                    buildAssignOp (
                        buildVarRefExp (
                            localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::setElementCounter]),
                        buildIntVal (0));

            SgExpression * reductUpperBound = buildIntVal (dim - 1);

            SgExpression * reductIncrementLoop = buildIntVal (1);

            /*
             * ======================================================
             * Get the reduction subroutine symbol
             * ======================================================
             */

            SgSymbol
                * reductionSymbol = parallelLoop->getReductionSubroutineHeader (
                    i)-> search_for_symbol_from_symbol_table ();

            SgFunctionSymbol * reductionFunctionSymbol = isSgFunctionSymbol (
                reductionSymbol);

            ROSE_ASSERT ( reductionFunctionSymbol != NULL );

            /*
             * ======================================================
             * Creating parameters
             * ======================================================
             */

            SgVarRefExp * blockidx_Reference1 = buildOpaqueVarRefExp (
                CUDA::Fortran::VariableNames::blockidx, subroutineScope);

            SgVarRefExp * x_Reference1 = buildOpaqueVarRefExp (
                CUDA::Fortran::FieldNames::x, subroutineScope);

            SgExpression * blockidXDotX = buildDotExp (blockidx_Reference1,
                x_Reference1);

            SgExpression * blockidXDotXMinus1 = buildSubtractOp (blockidXDotX,
                buildIntVal (1));

            SgExpression
                * baseIndexDevVar =
                    buildAddOp (
                        buildVarRefExp (
                            localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::setElementCounter]),
                        buildMultiplyOp (blockidXDotXMinus1, buildIntVal (1)));

            SgExpression * endIndexDevVar = buildAddOp (baseIndexDevVar,
                buildIntVal (dim - 1));

            SgSubscriptExpression * deviceVarAccess =
                new SgSubscriptExpression (ROSEHelper::getFileInfo (),
                    baseIndexDevVar, endIndexDevVar, buildIntVal (1));

            deviceVarAccess->set_endOfConstruct (ROSEHelper::getFileInfo ());
            deviceVarAccess->setCompilerGenerated ();
            deviceVarAccess->setOutputInCodeGeneration ();

            SgExpression * deviceVar = buildPntrArrRefExp (buildVarRefExp (
                formalParameterDeclarations[get_OP_DAT_VariableName (i)]),
                deviceVarAccess);

            SgExpression
                * localThreadVar =
                    buildPntrArrRefExp (
                        buildVarRefExp (
                            localVariableDeclarations[getLocalThread_OP_DAT_VariableName (
                                i)]),
                        buildVarRefExp (
                            localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::setElementCounter]));

            SgExprListExp
                * reductionActualParams =
                    buildExprListExp (
                        deviceVar,
                        localThreadVar,
                        buildVarRefExp (
                            formalParameterDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]),
                        buildVarRefExp (
                            formalParameterDeclarations[IndirectAndDirectLoop::Fortran::KernelSubroutine::offsetForReduction]));

            SgFunctionCallExp * reductionFunctionCall = buildFunctionCallExp (
                reductionFunctionSymbol, reductionActualParams);

            SgBasicBlock * reductCallLoopBody = buildBasicBlock (
                buildExprStatement (reductionFunctionCall));

            SgFortranDo
                * reductionLoop =
                    FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
                        reductInitLoop, reductUpperBound, reductIncrementLoop,
                        reductCallLoopBody);

            appendStatement (reductionLoop, subroutineScope);

            break;
          }

          case MAX_ACCESS:
          {
            Debug::getInstance ()->errorMessage (
                "Error: OP_MAX on reduction variables is not supported");
          }

          case MIN_ACCESS:
          {
            Debug::getInstance ()->errorMessage (
                "Error: OP_MIN on reduction variables is not supported");
          }

          default:
          {
            Debug::getInstance ()->errorMessage (
                "Error: wrong accessing code for global variable");
          }

        }
      }
    }
  }
}

void
KernelSubroutine::createAndAppendSharedMemoryOffesetForReduction ()
{
  formalParameterDeclarations[IndirectAndDirectLoop::Fortran::KernelSubroutine::offsetForReduction]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          IndirectAndDirectLoop::Fortran::KernelSubroutine::offsetForReduction,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE);
}

void
KernelSubroutine::createArgsSizesFormalParameter (
    DataSizesDeclaration * dataSizesDeclaration)
{
  formalParameterDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          IndirectAndDirectLoop::Fortran::VariableNames::argsSizes,
          dataSizesDeclaration->getType (), subroutineScope, formalParameters,
          1, DEVICE);
}

KernelSubroutine::KernelSubroutine (std::string const & subroutineName,
    UserDeviceSubroutine * userDeviceSubroutine, ParallelLoop * parallelLoop) :
  Subroutine (subroutineName + SubroutineNameSuffixes::kernelSuffix)
{
  this->userDeviceSubroutine = userDeviceSubroutine;

  this->parallelLoop = parallelLoop;
}
