#include <Debug.h>
#include <boost/lexical_cast.hpp>
#include <KernelSubroutine.h>
#include <CommonNamespaces.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <ROSEHelper.h>

void
KernelSubroutine::createArgsSizesFormalParameter (
    DeviceDataSizesDeclaration & deviceDataSizesDeclaration)
{
  formalParameterDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectAndDirectLoop::Fortran::VariableNames::argsSizes,
          deviceDataSizesDeclaration.getType (), subroutineScope);

  formalParameterDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]->get_declarationModifier ().get_typeModifier ().setDevice ();

  formalParameters->append_arg (
      *(formalParameterDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]->get_variables ().begin ()));
}

void
KernelSubroutine::createLocalThreadVariables (ParallelLoop & parallelLoop)
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
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if ((parallelLoop.isDirectLoop () == true
        && parallelLoop.get_OP_DAT_Dimension (i) > 1
        && parallelLoop.get_OP_MAP_Value (i) == DIRECT)
        || (parallelLoop.get_OP_MAP_Value (i) == GLOBAL
            && parallelLoop.get_OP_Access_Value (i) != READ_ACCESS)
        || (parallelLoop.get_OP_MAP_Value (i) == INDIRECT
            && parallelLoop.get_OP_Access_Value (i) == INC_ACCESS))
    {
      string const localThreadVarName =
          IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
              + lexical_cast <string> (i)
              + IndirectAndDirectLoop::Fortran::VariableSuffixes::localVarName;

      /*
       * ======================================================
       * Retrieving size from type of corresponding OP_DAT
       * declaration
       * ======================================================
       */

      SgType * opDatBaseType = parallelLoop.get_OP_DAT_Type (i);

      SgArrayType * arrayType = isSgArrayType (opDatBaseType);

      ROSE_ASSERT ( arrayType != NULL );

      opDatBaseType = arrayType->get_base_type ();

      SgArrayType * variableType = FortranTypesBuilder::getArray_RankOne (
          opDatBaseType, 0, parallelLoop.get_OP_DAT_Dimension (i) - 1);

      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          localThreadVarName, variableType, NULL, subroutineScope);

      variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      appendStatement (variableDeclaration, subroutineScope);

      localVariables_localThreadVariables[i] = variableDeclaration;
    }
  }
}

void
KernelSubroutine::createAutosharedVariable (ParallelLoop & parallelLoop)
{
  /*
   * ======================================================
   * For now only real(8) or real(4): this is decided
   * by checking the fortran type kinds of parallel loop
   * op_dat arguments
   * ======================================================
   */

  SgType * autosharedType = NULL;

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
      SgType * opDatArrayType = parallelLoop.get_OP_DAT_Type (i);

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
KernelSubroutine::initialiseLocalThreadVariables (ParallelLoop & parallelLoop)
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
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    int dim = parallelLoop.get_OP_DAT_Dimension (i);

    if (parallelLoop.get_OP_MAP_Value (i) == GLOBAL
        && parallelLoop.get_OP_Access_Value (i) != READ_ACCESS)
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
                  buildVarRefExp (localVariables_localThreadVariables[i]),
                  buildVarRefExp (
                      localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::setElementCounter]));

      if (parallelLoop.get_OP_Access_Value (i) == INC_ACCESS)
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
            localVariables_localThreadVariables[i]), arrayAccessComplexExpr);

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
KernelSubroutine::createAndAppendReductionSubroutineCall (
    ParallelLoop & parallelLoop)
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

  if (parallelLoop.isReductionRequired () == true)
  {
    for (unsigned int i = 1; i
        <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
    {
      if (parallelLoop.isReductionRequired (i) == true)
      {
        int dim = parallelLoop.get_OP_DAT_Dimension (i);

        switch (parallelLoop.get_OP_Access_Value (i))
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
                * reductionSymbol =
                    reductionSubroutines[i]-> search_for_symbol_from_symbol_table ();

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
                formalParameter_OP_DATs[i]), deviceVarAccess);

            SgExpression
                * localThreadVar =
                    buildPntrArrRefExp (
                        buildVarRefExp (localVariables_localThreadVariables[i]),
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
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectAndDirectLoop::Fortran::KernelSubroutine::offsetForReduction,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  formalParameterDeclarations[IndirectAndDirectLoop::Fortran::KernelSubroutine::offsetForReduction]->get_declarationModifier ().get_typeModifier ().setValue ();

  formalParameters->append_arg (
      *(formalParameterDeclarations[IndirectAndDirectLoop::Fortran::KernelSubroutine::offsetForReduction]->get_variables ().begin ()));
}

SgStatement *
KernelSubroutine::createUserSubroutineCall (
    UserDeviceSubroutine & userDeviceSubroutine,
    SgVariableDeclaration * variable_setElementCounter,
    SgVariableDeclaration * variable_offsetInThreadBlock,
    ParallelLoop & parallelLoop, std::map <unsigned int,
        SgVariableDeclaration *> * formalParameters_GlobalToLocalMapping,
    std::map <unsigned int, SgVariableDeclaration *> * localVariables_nbytes)
{
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprListExp;
  using boost::lexical_cast;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Creating call to user device subroutine", 2);

  SgExprListExp * userDeviceSubroutineParameters = buildExprListExp ();

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    int dim = parallelLoop.get_OP_DAT_Dimension (i);

    SgExpression * parameterExpression = buildIntVal (1);

    if (parallelLoop.get_OP_MAP_Value (i) == GLOBAL)
    {
      if (parallelLoop.get_OP_Access_Value (i) == READ_ACCESS)
      {

        /*
         * ======================================================
         * Case of global variable accessed in read mode:
         * we directly access the device variable, by
         * passing the kernel the variable name in positions
         * 0:argSize%<devVarName>-1. The name of the proper field
         * is obtained by appending "argument", <i>, and "_Size"
         * ======================================================
         */

        string const variableName =
            IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
                + lexical_cast <string> (i);

        string const variableSizeName = variableName
            + IndirectAndDirectLoop::Fortran::VariableSuffixes::Size;

        SgExpression
            * argSizeField =
                buildDotExp (
                    buildVarRefExp (
                        formalParameterDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]),
                    buildOpaqueVarRefExp (variableSizeName, subroutineScope));

        SgExpression * minusOneExpression = buildSubtractOp (argSizeField,
            buildIntVal (1));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (ROSEHelper::getFileInfo (), buildIntVal (
                0), minusOneExpression, buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            ROSEHelper::getFileInfo ());
        arraySubscriptExpression->setCompilerGenerated ();
        arraySubscriptExpression->setOutputInCodeGeneration ();

        parameterExpression = buildPntrArrRefExp (buildVarRefExp (
            formalParameter_OP_DATs[i]), arraySubscriptExpression);
      }
      else
      {
        /*
         * ======================================================
         * Case of global variable accessed *not* in read mode:
         * we access the corresponding local thread variable
         * ======================================================
         */

        parameterExpression = buildVarRefExp (
            localVariables_localThreadVariables[i]);
      }
    }
    else if (parallelLoop.get_OP_MAP_Value (i) == INDIRECT
        && parallelLoop.get_OP_Access_Value (i) == INC_ACCESS)
    {
      parameterExpression = buildVarRefExp (
          localVariables_localThreadVariables[i]);
    }
    else if (parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      SgVarRefExp
          * autoshared_Reference =
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]);

      SgVarRefExp * globalToLocalMappingArray_Reference = buildVarRefExp (
          (*formalParameters_GlobalToLocalMapping)[i]);

      SgVarRefExp * nbytes_Reference = buildVarRefExp (
          (*localVariables_nbytes)[i]);

      SgAddOp * lowerBound_addExpression1 = buildAddOp (buildVarRefExp (
          variable_setElementCounter), buildVarRefExp (
          variable_offsetInThreadBlock));

      SgPntrArrRefExp * lowerBound_arrayExpression = buildPntrArrRefExp (
          globalToLocalMappingArray_Reference, lowerBound_addExpression1);

      SgMultiplyOp * lowerBound_multiplyExpression = buildMultiplyOp (
          lowerBound_arrayExpression, buildIntVal (
              parallelLoop.get_OP_DAT_Dimension (i)));

      SgAddOp * lowerBound_addExpression2 = buildAddOp (nbytes_Reference,
          lowerBound_multiplyExpression);

      SgAddOp * upperBound_addExpression1 = buildAddOp (buildVarRefExp (
          variable_setElementCounter), buildVarRefExp (
          variable_offsetInThreadBlock));

      SgPntrArrRefExp * upperBound_arrayExpression = buildPntrArrRefExp (
          globalToLocalMappingArray_Reference, upperBound_addExpression1);

      SgMultiplyOp * upperBound_multiplyExpression = buildMultiplyOp (
          upperBound_arrayExpression, buildIntVal (
              parallelLoop.get_OP_DAT_Dimension (i)));

      SgAddOp * upperBound_addExpression2 = buildAddOp (nbytes_Reference,
          upperBound_multiplyExpression);

      SgAddOp * upperBound_addExpression3 = buildAddOp (
          upperBound_addExpression2, buildIntVal (
              parallelLoop.get_OP_DAT_Dimension (i)));

      SgSubscriptExpression * subscriptExpression = new SgSubscriptExpression (
          ROSEHelper::getFileInfo (), lowerBound_addExpression2,
          upperBound_addExpression3, buildIntVal (1));
      subscriptExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

      parameterExpression = buildPntrArrRefExp (autoshared_Reference,
          buildExprListExp (subscriptExpression));

    }
    else if (parallelLoop.get_OP_MAP_Value (i) == DIRECT)
    {
      if (parallelLoop.getNumberOfIndirectDataSets () > 0)
      {

        SgExpression * deviceVarAccessDirectBegin = buildMultiplyOp (
            buildAddOp (buildVarRefExp (variable_setElementCounter),
                buildVarRefExp (variable_offsetInThreadBlock)), buildIntVal (
                dim));

        SgExpression * deviceVarAccessDirectEnd = buildAddOp (
            deviceVarAccessDirectBegin, buildIntVal (dim));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (ROSEHelper::getFileInfo (),
                deviceVarAccessDirectBegin, deviceVarAccessDirectEnd,
                buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            ROSEHelper::getFileInfo ());
        arraySubscriptExpression->setCompilerGenerated ();
        arraySubscriptExpression->setOutputInCodeGeneration ();

        parameterExpression = buildPntrArrRefExp (buildVarRefExp (
            formalParameter_OP_DATs[i]), arraySubscriptExpression);

      }
      else if (dim == 1)
      {
        SgExpression * nVarRef = buildVarRefExp (variable_setElementCounter);
        SgExpression * nPlusDimMinusOneExpr = buildAddOp (nVarRef, buildIntVal (
            dim - 1));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (ROSEHelper::getFileInfo (), nVarRef,
                nPlusDimMinusOneExpr, buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            ROSEHelper::getFileInfo ());
        arraySubscriptExpression->setCompilerGenerated ();
        arraySubscriptExpression->setOutputInCodeGeneration ();

        parameterExpression = buildPntrArrRefExp (buildVarRefExp (
            formalParameter_OP_DATs[i]), arraySubscriptExpression);
      }
      else
      {
        parameterExpression = buildVarRefExp (
            localVariables_localThreadVariables[i]);
      }
    }

    userDeviceSubroutineParameters->append_expression (parameterExpression);
  }

  return buildFunctionCallStmt (userDeviceSubroutine.getSubroutineName (),
      buildVoidType (), userDeviceSubroutineParameters, subroutineScope);
}

