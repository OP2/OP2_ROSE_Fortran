#include <FortranCUDAKernelSubroutine.h>
#include <CommonNamespaces.h>
#include <FortranCUDAReductionSubroutine.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <RoseHelper.h>
#include <Debug.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

SgExpression *
FortranCUDAKernelSubroutine::buildOpGlobalActualParameterExpression (
    unsigned int OP_DAT_ArgumentGroup)
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using std::string;

  SgExpression * parameterExpression;

  if (parallelLoop->getOpAccessValue (OP_DAT_ArgumentGroup) == READ_ACCESS)
  {
    Debug::getInstance ()->debugMessage ("OP_GBL with read access",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    string const variableName = VariableNames::getOpDatSizeName (
        OP_DAT_ArgumentGroup);

    SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
        variableDeclarations->get (
            VariableNames::getDataSizesVariableDeclarationName (
                userSubroutineName))), buildOpaqueVarRefExp (variableName,
        subroutineScope));

    SgSubtractOp * subtractExpression = buildSubtractOp (dotExpression,
        buildIntVal (1));

    SgSubscriptExpression * subscriptExpression = new SgSubscriptExpression (
        RoseHelper::getFileInfo (), buildIntVal (0), subtractExpression,
        buildIntVal (1));

    subscriptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

    parameterExpression = buildPntrArrRefExp (buildVarRefExp (
        variableDeclarations->get (VariableNames::getOpDatName (
            OP_DAT_ArgumentGroup))), subscriptExpression);
  }
  else
  {
    Debug::getInstance ()->debugMessage ("OP_GBL with write access",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    /*
     * ======================================================
     * Case of global variable accessed NOT in read mode:
     * we access the corresponding local thread variable
     * ======================================================
     */

    parameterExpression = buildVarRefExp (variableDeclarations->get (
        VariableNames::getOpDatLocalName (OP_DAT_ArgumentGroup)));
  }

  return parameterExpression;
}

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
        SgExprStatement * assignmentStatement = buildAssignStatement (
            arrayIndexExpression1, buildIntVal (0));

        appendStatement (assignmentStatement, loopBody);
      }
      else
      {
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
      /*
       * ======================================================
       * Create reduction call parameters
       * ======================================================
       */

      SgDotExp * dotExpression1 = buildDotExp (buildOpaqueVarRefExp (
          CUDA::Fortran::blockidx, subroutineScope), buildOpaqueVarRefExp (
          CUDA::Fortran::x, subroutineScope));

      SgSubtractOp * subtractExpression1 = buildSubtractOp (dotExpression1,
          buildIntVal (1));

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
          subtractExpression1, buildIntVal (1));

      SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::setElementCounter)),
          multiplyExpression1);

      SgAddOp * addExpression2 = buildAddOp (addExpression1, buildIntVal (
          parallelLoop->getOpDatDimension (i) - 1));

      SgSubscriptExpression * subscriptExpression1 = new SgSubscriptExpression (
          RoseHelper::getFileInfo (), addExpression1, addExpression2,
          buildIntVal (1));

      subscriptExpression1->set_endOfConstruct (RoseHelper::getFileInfo ());

      /*
       * ======================================================
       * Index into OP_DAT array
       * ======================================================
       */

      SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              VariableNames::getOpDatName (i))), subscriptExpression1);

      /*
       * ======================================================
       * Index into local OP_DAT array
       * ======================================================
       */

      SgPntrArrRefExp * arrayIndexExpression2 = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              VariableNames::getOpDatLocalName (i))), buildVarRefExp (
              variableDeclarations->get (
                  DirectLoop::Fortran::KernelSubroutine::setElementCounter)));

      /*
       * ======================================================
       * Reduction operation parameter
       * ======================================================
       */

      SgIntVal * reductionType;

      switch (parallelLoop->getOpAccessValue (i))
      {
        case INC_ACCESS:
        {
          reductionType = buildIntVal (INCREMENT);
          break;
        }

        case MAX_ACCESS:
        {
          reductionType = buildIntVal (MAX_ACCESS);
          break;
        }

        case MIN_ACCESS:
        {
          reductionType = buildIntVal (MIN_ACCESS);
          break;
        }
      }

      SgExprListExp * actualParameters = buildExprListExp (
          arrayIndexExpression1, arrayIndexExpression2, buildVarRefExp (
              variableDeclarations->get (
                  DirectLoop::Fortran::KernelSubroutine::warpSize)),
          buildVarRefExp (variableDeclarations->get (
              ReductionSubroutine::offsetForReduction)), reductionType);

      /*
       * ======================================================
       * Create reduction function call
       * ======================================================
       */

      SgFunctionSymbol * reductionFunctionSymbol =
          isSgFunctionSymbol (
              reductionSubroutines->getHeader (parallelLoop->getReductionTuple (
                  i))->get_symbol_from_symbol_table ());

      ROSE_ASSERT (reductionFunctionSymbol != NULL);

      SgFunctionCallExp * reductionFunctionCall = buildFunctionCallExp (
          reductionFunctionSymbol, actualParameters);

      /*
       * ======================================================
       * Create loop to repeatedly call reduction subroutine
       * up to the number of dimensions of OP_DAT
       * ======================================================
       */

      SgBasicBlock * loopBody = buildBasicBlock ();

      appendStatement (buildExprStatement (reductionFunctionCall), loopBody);

      SgAssignOp * initializationExpression = buildAssignOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::setElementCounter)),
          buildIntVal (0));

      SgIntVal * upperBoundExpression = buildIntVal (
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
FortranCUDAKernelSubroutine::createAutoSharedDeclarations ()
{
  using std::find;
  using std::vector;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating autoshared declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  vector <string> autosharedNames;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isGlobalScalar (i) == false)
      {
        string const autosharedVariableName =
            VariableNames::getAutosharedDeclarationName (
                parallelLoop->getOpDatBaseType (i),
                parallelLoop->getSizeOfOpDat (i));

        if (find (autosharedNames.begin (), autosharedNames.end (),
            autosharedVariableName) == autosharedNames.end ())
        {
          Debug::getInstance ()->debugMessage (
              "Creating autoshared declaration with name '"
                  + autosharedVariableName + "' for OP_DAT '"
                  + parallelLoop->getOpDatVariableName (i) + "'",
              Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

          SgExpression * upperBound = new SgAsteriskShapeExp (
              RoseHelper::getFileInfo ());

          variableDeclarations->add (
              autosharedVariableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  autosharedVariableName,
                  FortranTypesBuilder::getArray_RankOne (
                      parallelLoop->getOpDatBaseType (i), 0, upperBound),
                  subroutineScope, 1, SHARED));

          autosharedNames.push_back (autosharedVariableName);
        }
      }
    }
  }
}

FortranCUDAKernelSubroutine::FortranCUDAKernelSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    FortranReductionSubroutines * reductionSubroutines,
    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration) :
  FortranKernelSubroutine (subroutineName, userSubroutineName, parallelLoop,
      moduleScope), opDatDimensionsDeclaration (opDatDimensionsDeclaration)
{
  using SageInterface::addTextForUnparser;

  this->reductionSubroutines = reductionSubroutines;

  addTextForUnparser (subroutineHeaderStatement, "attributes(global) ",
      AstUnparseAttribute::e_before);
}
