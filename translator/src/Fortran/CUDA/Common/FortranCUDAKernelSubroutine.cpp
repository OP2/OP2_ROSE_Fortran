#include <Debug.h>
#include <FortranCUDAKernelSubroutine.h>
#include <CommonNamespaces.h>
#include <FortranCUDAReductionSubroutine.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

void
FortranCUDAKernelSubroutine::createInitialiseLocalThreadVariablesStatements ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAddOp;
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * We essentially do a similar thing to the one we make
   * when we declare local thread variables
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) == GLOBAL
        && parallelLoop->getOpAccessValue (i) != READ_ACCESS)
    {
      SgBasicBlock * loopBody = buildBasicBlock ();

      SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              VariableNames::getOpDatLocalName (i))), buildVarRefExp (
              variableDeclarations->get (
                  DirectLoop::Fortran::KernelSubroutine::setElementCounter)));

      if (parallelLoop->getOpAccessValue (i) == INC_ACCESS)
      {
        // for (int d=0; d<DIM; d++) ARG_l[d)=ZERO_TYP;

        SgExprStatement * assignmentStatement = buildAssignStatement (
            arrayIndexExpression1, buildIntVal (0));

        appendStatement (assignmentStatement, loopBody);
      }
      else
      {
        //for (int d=0; d<DIM; d++) ARG_l[d)=ARG[d+blockIdx.x*DIM);

        SgDotExp * dotExpression = buildDotExp (buildOpaqueVarRefExp (
            CUDA::Fortran::blockidx, subroutineScope), buildOpaqueVarRefExp (
            CUDA::Fortran::x, subroutineScope));

        SgMultiplyOp * multiplyExpression = buildMultiplyOp (dotExpression,
            buildIntVal (parallelLoop->getOpDatDimension (i)));

        SgAddOp * addExpression = buildAddOp (buildVarRefExp (
            variableDeclarations->get (
                DirectLoop::Fortran::KernelSubroutine::setElementCounter)),
            multiplyExpression);

        SgPntrArrRefExp * arrayIndexExpression2 = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                VariableNames::getOpDatLocalName (i))), addExpression);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            arrayIndexExpression1, arrayIndexExpression2);

        appendStatement (assignmentStatement, loopBody);
      }

      /*
       * ======================================================
       * Remember that local thread variables are accessed
       * in the C-like notation for arrays: 0:N-1
       * ======================================================
       */

      SgExpression * initializationExpression = buildAssignOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::setElementCounter)),
          buildIntVal (0));

      SgExpression * upperBoundExpression = buildIntVal (
          parallelLoop->getOpDatDimension (i) - 1);

      SgFortranDo * loopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              initializationExpression, upperBoundExpression, buildIntVal (1),
              loopBody);

      appendStatement (loopStatement, subroutineScope);
    }
  }
}

void
FortranCUDAKernelSubroutine::createReductionLoopStatements ()
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
  using std::string;

  Debug::getInstance ()->debugMessage ("Adding reduction subroutine call",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i) == true)
    {
      switch (parallelLoop->getOpAccessValue (i))
      {
        case INC_ACCESS:
        {
          SgBasicBlock * loopBody = buildBasicBlock ();

          /*
           * ======================================================
           * Get the reduction subroutine symbol
           * ======================================================
           */

          SgSymbol
              * reductionSymbol =
                  parallelLoop->getReductionSubroutineHeader (i)-> search_for_symbol_from_symbol_table ();

          SgFunctionSymbol * reductionFunctionSymbol = isSgFunctionSymbol (
              reductionSymbol);

          ROSE_ASSERT (reductionFunctionSymbol != NULL);

          /*
           * ======================================================
           * Create parameters
           * ======================================================
           */

          SgDotExp * dotExpression = buildDotExp (buildOpaqueVarRefExp (
              CUDA::Fortran::blockidx, subroutineScope), buildOpaqueVarRefExp (
              CUDA::Fortran::x, subroutineScope));

          SgSubtractOp * subtractExpression = buildSubtractOp (dotExpression,
              buildIntVal (1));

          SgMultiplyOp * multiplyExpression = buildMultiplyOp (
              subtractExpression, buildIntVal (1));

          SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
              variableDeclarations->get (
                  DirectLoop::Fortran::KernelSubroutine::setElementCounter)),
              multiplyExpression);

          SgAddOp * addExpression2 = buildAddOp (addExpression1, buildIntVal (
              parallelLoop->getOpDatDimension (i) - 1));

          SgSubscriptExpression * subscriptExpression =
              new SgSubscriptExpression (RoseHelper::getFileInfo (),
                  addExpression1, addExpression2, buildIntVal (1));

          subscriptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

          SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (
              buildVarRefExp (variableDeclarations->get (
                  VariableNames::getOpDatName (i))), subscriptExpression);

          SgPntrArrRefExp * arrayIndexExpression2 = buildPntrArrRefExp (
              buildVarRefExp (variableDeclarations->get (
                  VariableNames::getOpDatLocalName (i))),
              buildVarRefExp (variableDeclarations->get (
                  DirectLoop::Fortran::KernelSubroutine::setElementCounter)));

          SgExprListExp * actualParameters = buildExprListExp (
              arrayIndexExpression1, arrayIndexExpression2, buildVarRefExp (
                  variableDeclarations->get (
                      DirectLoop::Fortran::KernelSubroutine::warpSize)),
              buildVarRefExp (variableDeclarations->get (
                  ReductionSubroutine::offsetForReduction)));

          SgFunctionCallExp * reductionFunctionCall = buildFunctionCallExp (
              reductionFunctionSymbol, actualParameters);

          appendStatement (buildExprStatement (reductionFunctionCall), loopBody);

          SgAssignOp * initializationExpression = buildAssignOp (
              buildVarRefExp (variableDeclarations->get (
                  DirectLoop::Fortran::KernelSubroutine::setElementCounter)),
              buildIntVal (0));

          SgIntVal * upperBoundExpression = buildIntVal (
              parallelLoop->getOpDatDimension (i) - 1);

          SgFortranDo * loopStatement =
              FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
                  initializationExpression, upperBoundExpression, buildIntVal (
                      1), loopBody);

          appendStatement (loopStatement, subroutineScope);

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

void
FortranCUDAKernelSubroutine::createLocalThreadDeclarations ()
{
  using SageBuilder::buildIntVal;
  using SageBuilder::buildArrayType;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildExprListExp;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local thread variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if ((parallelLoop->isDirectLoop () == true
        && parallelLoop->getOpDatDimension (i) > 1
        && parallelLoop->getOpMapValue (i) == DIRECT)
        || (parallelLoop->getOpMapValue (i) == GLOBAL
            && parallelLoop->getOpAccessValue (i) != READ_ACCESS)
        || (parallelLoop->getOpMapValue (i) == INDIRECT
            && parallelLoop->getOpAccessValue (i) == INC_ACCESS))
    {
      SgType * opDatType = parallelLoop->getOpDatType (i);

      SgArrayType * arrayType = isSgArrayType (opDatType);

      SgType * opDatBaseType = arrayType->get_base_type ();

      string const & variableName = VariableNames::getOpDatLocalName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getArray_RankOne (
                  opDatBaseType, 0, parallelLoop->getOpDatDimension (i) - 1),
              subroutineScope));
    }
  }
}

void
FortranCUDAKernelSubroutine::createAutoSharedDeclaration ()
{
  bool found = false;

  unsigned int i = 1;

  while (found == false && i <= parallelLoop->getNumberOfOpDatArgumentGroups ())
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgType * opDatType = parallelLoop->getOpDatType (i);

      SgArrayType * isArrayType = isSgArrayType (opDatType);

      SgType * opDatBaseType = isArrayType->get_base_type ();

      if (opDatBaseType->variantT () == V_SgTypeFloat)
      {
        SgExpression * upperBound = new SgAsteriskShapeExp (
            RoseHelper::getFileInfo ());

        variableDeclarations->add (CommonVariableNames::autoshared,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                CommonVariableNames::autoshared,
                FortranTypesBuilder::getArray_RankOne (opDatBaseType, 0,
                    upperBound), subroutineScope, 1, SHARED));

        found = true;
      }
    }

    ++i;
  }

  if (found == false)
  {
    Debug::getInstance ()->errorMessage (
        "Unable to create autoshared variable as none of the OP_DATs have a base type of reals.");
  }
}

FortranCUDAKernelSubroutine::FortranCUDAKernelSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration) :
  FortranKernelSubroutine (subroutineName, userSubroutineName, parallelLoop,
      moduleScope), opDatDimensionsDeclaration (opDatDimensionsDeclaration)
{
  using SageInterface::addTextForUnparser;

  addTextForUnparser (subroutineHeaderStatement, "attributes(global) ",
      AstUnparseAttribute::e_before);
}
