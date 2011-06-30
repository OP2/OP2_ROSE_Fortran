#include <Debug.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <FortranCUDAHostSubroutine.h>
#include <FortranCUDAReductionSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

SgStatement *
FortranCUDAHostSubroutine::createThreadSynchroniseCallStatement ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignStatement;

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction (
          CUDA::Fortran::cudaThreadSynchronize, subroutineScope);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      buildExprListExp ());

  SgExprStatement * callStatement = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (CUDA::Fortran::threadSynchRet)), functionCall);

  return callStatement;
}

SgStatement *
FortranCUDAHostSubroutine::createInitialiseConstantsCallStatement ()
{
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildFunctionCallStmt;

  SgExprListExp * actualParameters = buildExprListExp ();

  SgExprStatement * functionCallStatement = buildFunctionCallStmt (
      initialiseConstantsSubroutine->getSubroutineName (), buildVoidType (),
      actualParameters, subroutineScope);

  return functionCallStatement;
}

void
FortranCUDAHostSubroutine::createReductionPrologueStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildFloatVal;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildAddOp;
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * We need to allocate enough additional shared memory
   * thus we need the highest Fortran kind for OP_DATs
   * of reduction variables
   * ======================================================
   */
  unsigned int maxUsedFortranKind = -1;

  unsigned int dim = -1;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i) == true)
    {
      if (parallelLoop->getOpDatDimension (i) > dim)
      {
        dim = parallelLoop->getOpDatDimension (i);
      }

      SgExpression * fortranKind =
          FortranStatementsAndExpressionsBuilder::getFortranKindOf_OP_DAT (
              parallelLoop->getOpDatType (i));

      SgIntVal * kindVal = isSgIntVal (fortranKind);

      ROSE_ASSERT (kindVal != NULL);

      int actualValue = kindVal->get_value ();

      if (maxUsedFortranKind < actualValue)
      {
        maxUsedFortranKind = actualValue;
      }
    }
  }

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::maximumNumberOfThreadBlocks)), buildVarRefExp (
          variableDeclarations->get (CUDA::Fortran::blocksPerGrid)));

  appendStatement (assignmentStatement1, subroutineScope);

  SgMultiplyOp
      * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              ReductionSubroutine::maximumNumberOfThreadBlocks)), buildIntVal (
          dim));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::numberOfThreadItems)), multiplyExpression2);

  appendStatement (assignmentStatement2, subroutineScope);

  SgPntrArrRefExp * arrayIndexExpression3 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (ReductionSubroutine::reductionArrayHost)),
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::numberOfThreadItems)));

  FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
      buildExprListExp (arrayIndexExpression3), subroutineScope);

  SgPntrArrRefExp * arrayIndexExpression4 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (ReductionSubroutine::reductionArrayDevice)),
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::numberOfThreadItems)));

  FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
      buildExprListExp (arrayIndexExpression4), subroutineScope);

  SgPntrArrRefExp * arrayIndexExpression5 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (ReductionSubroutine::reductionArrayHost)),
      buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter1)));

  SgExprStatement * assignmentStatement5 = buildAssignStatement (
      arrayIndexExpression5, buildFloatVal (0.0));

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement5);

  SgAssignOp * loopInitialization = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      buildIntVal (0));

  SgFortranDo * loopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          loopInitialization, buildVarRefExp (variableDeclarations->get (
              ReductionSubroutine::numberOfThreadItems)), buildIntVal (1),
          loopBody);

  appendStatement (loopStatement, subroutineScope);

  SgExprStatement * assignmentStatement6 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::reductionArrayDevice)), buildVarRefExp (
          variableDeclarations->get (ReductionSubroutine::reductionArrayHost)));

  appendStatement (assignmentStatement6, subroutineScope);

  SgExprStatement * assignmentStatement7 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::sharedMemoryStartOffset)), buildVarRefExp (
          variableDeclarations->get (CUDA::Fortran::sharedMemorySize)));

  appendStatement (assignmentStatement7, subroutineScope);

  SgMultiplyOp * multiplyExpression8 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (CUDA::Fortran::threadsPerBlock)), buildIntVal (
      maxUsedFortranKind));

  SgExprStatement * assignmentStatement8 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::maximumBytesInSharedMemory)),
      multiplyExpression8);

  appendStatement (assignmentStatement8, subroutineScope);

  SgAddOp * addExpression9 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (CUDA::Fortran::sharedMemorySize)),
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::maximumBytesInSharedMemory)));

  SgExprStatement * assignmentStatement9 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          CUDA::Fortran::sharedMemorySize)), addExpression9);

  appendStatement (assignmentStatement9, subroutineScope);
}

void
FortranCUDAHostSubroutine::createReductionEpilogueStatements ()
{
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildBasicBlock;
  using SageInterface::appendStatement;

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::reductionArrayHost)),
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::reductionArrayDevice)));

  appendStatement (assignmentStatement1, subroutineScope);

  unsigned int dim = -1;
  unsigned int positionInOPDatsArray = -1;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i) == true)
    {
      dim = parallelLoop->getOpDatDimension (i);
      positionInOPDatsArray = i;
    }
  }

  SgAddOp * addExpression2 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildIntVal (1));

  SgPntrArrRefExp * arrayIndexExpression2 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (VariableNames::getCToFortranVariableName (
          positionInOPDatsArray))), addExpression2);

  SgMultiplyOp * mutliplyExpression2 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      buildIntVal (dim));

  SgAddOp * addExpression2b = buildAddOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      mutliplyExpression2);

  SgPntrArrRefExp * arrayIndexExpression2b = buildPntrArrRefExp (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::reductionArrayHost)), addExpression2b);

  SgExpression * addExpression2c = buildAddOp (arrayIndexExpression2,
      arrayIndexExpression2b);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      arrayIndexExpression2, addExpression2c);

  SgBasicBlock * innerLoopBody = buildBasicBlock (assignmentStatement2);

  SgExpression * innerLoopInitialization = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildIntVal (0));

  SgFortranDo * innerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          innerLoopInitialization, buildIntVal (dim - 1), buildIntVal (1),
          innerLoopBody);

  SgBasicBlock * outerLoopBody = buildBasicBlock (innerLoopStatement);

  SgExpression * initOuterLoop = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      buildIntVal (0));

  SgFortranDo * outerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initOuterLoop, buildVarRefExp (variableDeclarations->get (
              ReductionSubroutine::numberOfThreadItems)), buildIntVal (1),
          outerLoopBody);

  appendStatement (outerLoopStatement, subroutineScope);
}

void
FortranCUDAHostSubroutine::createReductionLocalVariableDeclarations ()
{
  using SageBuilder::buildIntVal;
  using SageBuilder::buildIntType;
  using SageBuilder::buildAssignInitializer;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;

  SgVariableDeclaration * variableDeclaration1 = buildVariableDeclaration (
      CommonVariableNames::iterationCounter1,
      FortranTypesBuilder::getFourByteInteger (), buildAssignInitializer (
          buildIntVal (0), buildIntType ()), subroutineScope);

  variableDeclaration1-> get_declarationModifier ().get_accessModifier ().setUndefined ();

  variableDeclarations->add (CommonVariableNames::iterationCounter1,
      variableDeclaration1);

  appendStatement (variableDeclaration1, subroutineScope);

  SgVariableDeclaration * variableDeclaration2 = buildVariableDeclaration (
      CommonVariableNames::iterationCounter2,
      FortranTypesBuilder::getFourByteInteger (), buildAssignInitializer (
          buildIntVal (0), buildIntType ()), subroutineScope);

  variableDeclaration2-> get_declarationModifier ().get_accessModifier ().setUndefined ();

  variableDeclarations->add (CommonVariableNames::iterationCounter2,
      variableDeclaration2);

  appendStatement (variableDeclaration2, subroutineScope);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i) == true)
    {
      SgVariableDeclaration * variableDeclaration3 = buildVariableDeclaration (
          ReductionSubroutine::sharedMemoryStartOffset,
          FortranTypesBuilder::getFourByteInteger (), buildAssignInitializer (
              buildIntVal (0), buildIntType ()), subroutineScope);

      variableDeclarations->add (ReductionSubroutine::sharedMemoryStartOffset,
          variableDeclaration3);

      variableDeclaration3-> get_declarationModifier ().get_accessModifier ().setUndefined ();

      appendStatement (variableDeclaration3, subroutineScope);

      SgVariableDeclaration * variableDeclaration4 = buildVariableDeclaration (
          ReductionSubroutine::maximumBytesInSharedMemory,
          FortranTypesBuilder::getFourByteInteger (), buildAssignInitializer (
              buildIntVal (0), buildIntType ()), subroutineScope);

      variableDeclarations->add (
          ReductionSubroutine::maximumBytesInSharedMemory, variableDeclaration4);

      variableDeclaration4 -> get_declarationModifier ().get_accessModifier ().setUndefined ();

      appendStatement (variableDeclaration4, subroutineScope);

      SgVariableDeclaration * variableDeclaration5 = buildVariableDeclaration (
          ReductionSubroutine::numberOfThreadItems,
          FortranTypesBuilder::getFourByteInteger (), buildAssignInitializer (
              buildIntVal (0), buildIntType ()), subroutineScope);

      variableDeclarations->add (ReductionSubroutine::numberOfThreadItems,
          variableDeclaration5);

      variableDeclaration5-> get_declarationModifier ().get_accessModifier ().setUndefined ();

      appendStatement (variableDeclaration5, subroutineScope);

      SgVariableDeclaration * variableDeclaration6 = buildVariableDeclaration (
          ReductionSubroutine::maximumNumberOfThreadBlocks,
          FortranTypesBuilder::getFourByteInteger (), buildAssignInitializer (
              buildIntVal (0), buildIntType ()), subroutineScope);

      variableDeclarations->add (
          ReductionSubroutine::maximumNumberOfThreadBlocks,
          variableDeclaration6);

      variableDeclaration6-> get_declarationModifier ().get_accessModifier ().setUndefined ();

      appendStatement (variableDeclaration6, subroutineScope);

      SgVariableDeclaration * variableDeclaration7 = buildVariableDeclaration (
          ReductionSubroutine::reductionArrayHost, parallelLoop->getOpDatType (
              i), NULL, subroutineScope);

      variableDeclarations->add (ReductionSubroutine::reductionArrayHost,
          variableDeclaration7);

      variableDeclaration7-> get_declarationModifier ().get_accessModifier ().setUndefined ();
      variableDeclaration7-> get_declarationModifier ().get_typeModifier ().setAllocatable ();

      appendStatement (variableDeclaration7, subroutineScope);

      SgVariableDeclaration * variableDeclaration8 = buildVariableDeclaration (
          ReductionSubroutine::reductionArrayDevice,
          parallelLoop->getOpDatType (i), NULL, subroutineScope);

      variableDeclarations->add (ReductionSubroutine::reductionArrayDevice,
          variableDeclaration8);

      variableDeclaration8-> get_declarationModifier ().get_accessModifier ().setUndefined ();
      variableDeclaration8-> get_declarationModifier ().get_typeModifier ().setAllocatable ();
      variableDeclaration8-> get_declarationModifier ().get_typeModifier ().setDevice ();

      appendStatement (variableDeclaration8, subroutineScope);
    }
  }
}

void
FortranCUDAHostSubroutine::createCUDAKernelEpilogueStatements ()
{
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;
  using std::map;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to copy data back from device and deallocate", 2);

  /*
   * ======================================================
   * Copy back and de-allocate only used for OP_DAT which
   * are NOT reduction variables
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i) == false)
    {
      SgExprStatement * assignmentStatement = buildAssignStatement (
          buildVarRefExp (variableDeclarations->get (
              VariableNames::getCToFortranVariableName (i))),
          buildVarRefExp (variableDeclarations->get (
              VariableNames::getOpDatDeviceName (i))));

      appendStatement (assignmentStatement, subroutineScope);
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i) == false)
    {
      SgExprListExp * deallocateParameters = buildExprListExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatDeviceName (i))));

      FortranStatementsAndExpressionsBuilder::appendDeallocateStatement (
          deallocateParameters, subroutineScope);
    }
  }
}

void
FortranCUDAHostSubroutine::createCUDAKernelPrologueStatements ()
{
  using SageBuilder::buildVoidType;
  using SageBuilder::buildIntType;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * Allocation and copy in only used for OP_DAT which are
   * NOT reduction variables
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i) == false)
    {
      SgExprListExp * arrayIndexExpression = buildExprListExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatSizeName (i))));

      SgPntrArrRefExp * subscriptExpression = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              VariableNames::getOpDatDeviceName (i))), arrayIndexExpression);

      FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
          buildExprListExp (subscriptExpression), subroutineScope);
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i) == false)
    {
      SgExprStatement * assignmentStatement = buildAssignStatement (
          buildVarRefExp (variableDeclarations->get (
              VariableNames::getOpDatDeviceName (i))), buildVarRefExp (
              variableDeclarations->get (
                  VariableNames::getCToFortranVariableName (i))));

      appendStatement (assignmentStatement, subroutineScope);
    }
  }
}

void
FortranCUDAHostSubroutine::createTransferOpDatStatements (
    SgScopeStatement * statementScope)
{
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      /*
       * ======================================================
       * Statement setting the size of the OP_DAT variable
       * ======================================================
       */

      SgDotExp * dotExpression1 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatName (i))),
          buildOpaqueVarRefExp (CommonVariableNames::dim, subroutineScope));

      SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatName (i))),
          buildOpaqueVarRefExp (CommonVariableNames::set, subroutineScope));

      SgDotExp * dotExpression3 = buildDotExp (dotExpression2,
          buildOpaqueVarRefExp (CommonVariableNames::size, subroutineScope));

      SgExpression * multiplyExpression = buildMultiplyOp (dotExpression1,
          dotExpression3);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          buildVarRefExp (variableDeclarations->get (
              VariableNames::getOpDatSizeName (i))), multiplyExpression);

      appendStatement (assignmentStatement, subroutineScope);

      /*
       * ======================================================
       * Statement to convert OP_DAT between C and Fortran
       * pointers
       * ======================================================
       */

      SgDotExp * parameterExpression1 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatName (i))),
          buildOpaqueVarRefExp (CommonVariableNames::dat, subroutineScope));

      SgVarRefExp * parameterExpression2 = buildVarRefExp (
          variableDeclarations->get (VariableNames::getCToFortranVariableName (
              i)));

      SgExpression * parameterExpression3 =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              variableDeclarations->get (VariableNames::getOpDatSizeName (i)),
              subroutineScope);

      SgStatement * callStatement =
          SubroutineCalls::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpression1, parameterExpression2,
              parameterExpression3);

      appendStatement (callStatement, subroutineScope);
    }
  }
}

void
FortranCUDAHostSubroutine::createOpDatDimensionInitialisationStatements ()
{
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageInterface::appendStatement;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgVarRefExp * opDatDimensionsReference = buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::opDatDimensions));

      SgVarRefExp * fieldReference = buildVarRefExp (
          opDatDimensionsDeclaration->getOpDatDimensionField (i));

      SgDotExp * dotExpression1 = buildDotExp (opDatDimensionsReference,
          fieldReference);

      SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatName (i))),
          buildOpaqueVarRefExp (CommonVariableNames::dim, subroutineScope));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          dotExpression1, dotExpression2);

      appendStatement (assignmentStatement, subroutineScope);
    }
  }
}

void
FortranCUDAHostSubroutine::createDataMarshallingLocalVariableDeclarations ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildPointerType;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variables to allow data marshalling between host and device",
      2);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = VariableNames::getOpDatSizeName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName =
          VariableNames::getCToFortranVariableName (i);

      SgArrayType * isArrayType =
          isSgArrayType (parallelLoop->getOpDatType (i));

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, buildPointerType (isArrayType), subroutineScope));
    }
  }

  /*
   * ======================================================
   * OP_DAT variable on device only needed if it is NOT a
   * reduction variable
   * ======================================================
   */
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i) == false)
    {
      string const & variableName = VariableNames::getOpDatDeviceName (i);

      SgArrayType * isArrayType =
          isSgArrayType (parallelLoop->getOpDatType (i));

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, isArrayType, subroutineScope, 2, DEVICE,
              ALLOCATABLE));
    }
  }
}

void
FortranCUDAHostSubroutine::createCUDAKernelLocalVariableDeclarations ()
{
  variableDeclarations->add (CUDA::Fortran::blocksPerGrid,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::Fortran::blocksPerGrid,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (CUDA::Fortran::threadsPerBlock,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::Fortran::threadsPerBlock,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (CUDA::Fortran::sharedMemorySize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::Fortran::sharedMemorySize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (CUDA::Fortran::threadSynchRet,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::Fortran::threadSynchRet,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}

FortranCUDAHostSubroutine::FortranCUDAHostSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    std::string const & kernelSubroutineName,
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration) :
  FortranHostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop, moduleScope),
      initialiseConstantsSubroutine (initialiseConstantsSubroutine),
      opDatDimensionsDeclaration (opDatDimensionsDeclaration)
{
  using SageInterface::addTextForUnparser;

  addTextForUnparser (subroutineHeaderStatement, "attributes(host) ",
      AstUnparseAttribute::e_before);
}
