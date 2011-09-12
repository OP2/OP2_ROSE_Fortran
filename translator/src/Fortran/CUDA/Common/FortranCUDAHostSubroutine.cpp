#include <Debug.h>
#include <CUDA.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <FortranCUDAHostSubroutine.h>
#include <RoseStatementsAndExpressionsBuilder.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

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

  Debug::getInstance ()->debugMessage (
      "Creating reduction prologue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i) == true)
    {
      unsigned int dim = parallelLoop->getOpDatDimension (i);

      SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (CUDA::blocksPerGrid)), buildIntVal (dim));

      SgExprStatement * assignmentStatement2 = buildAssignStatement (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::threadItems)), multiplyExpression2);

      appendStatement (assignmentStatement2, subroutineScope);

      SgPntrArrRefExp * arrayIndexExpression5 = buildPntrArrRefExp (
          buildVarRefExp (moduleDeclarations->getReductionArrayHostDeclaration (
              i)), buildVarRefExp (variableDeclarations->get (
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
                  OP2::VariableNames::threadItems)), buildIntVal (1), loopBody);

      appendStatement (loopStatement, subroutineScope);

      SgExprStatement * assignmentStatement6 = buildAssignStatement (
          buildVarRefExp (
              moduleDeclarations->getReductionArrayDeviceDeclaration (i)),
          buildVarRefExp (moduleDeclarations->getReductionArrayHostDeclaration (
              i)));

      appendStatement (assignmentStatement6, subroutineScope);

      SgExprStatement * assignmentStatement7 = buildAssignStatement (
          buildVarRefExp (
              variableDeclarations->get (OP2::VariableNames::offset)),
          buildVarRefExp (variableDeclarations->get (CUDA::sharedMemorySize)));

      appendStatement (assignmentStatement7, subroutineScope);

      SgMultiplyOp * multiplyExpression8 =
          buildMultiplyOp (buildVarRefExp (variableDeclarations->get (
              CUDA::threadsPerBlock)), buildIntVal (dim));

      SgExprStatement * assignmentStatement8 = buildAssignStatement (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::sharedMemoryBytes)), multiplyExpression8);

      appendStatement (assignmentStatement8, subroutineScope);

      SgAddOp * addExpression9 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (CUDA::sharedMemorySize)), buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::sharedMemoryBytes)));

      SgExprStatement * assignmentStatement9 = buildAssignStatement (
          buildVarRefExp (variableDeclarations->get (CUDA::sharedMemorySize)),
          addExpression9);

      appendStatement (assignmentStatement9, subroutineScope);
    }
  }
}

void
FortranCUDAHostSubroutine::createReductionEpilogueStatements ()
{
  using boost::lexical_cast;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildBasicBlock;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating reduction epilogue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i) == true)
    {
      unsigned int dim = parallelLoop->getOpDatDimension (i);

      Debug::getInstance ()->debugMessage (
          "Creating statements for OP_DAT argument '" + lexical_cast <string> (
              i) + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

      /*
       * ======================================================
       * Assign device reduction array to host reduction array
       * ======================================================
       */

      SgExprStatement * assignmentStatement1 = buildAssignStatement (
          buildVarRefExp (moduleDeclarations->getReductionArrayHostDeclaration (
              i)), buildVarRefExp (
              moduleDeclarations->getReductionArrayDeviceDeclaration (i)));

      appendStatement (assignmentStatement1, subroutineScope);

      SgAddOp * addExpression2 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter2)),
          buildIntVal (1));

      SgPntrArrRefExp * arrayIndexExpression2 = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getOpDatDeviceName (i))), addExpression2);

      SgMultiplyOp * mutliplyExpression2 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter1)),
          buildIntVal (dim));

      SgAddOp * addExpression2b = buildAddOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter2)),
          mutliplyExpression2);

      SgPntrArrRefExp * arrayIndexExpression2b = buildPntrArrRefExp (
          buildVarRefExp (moduleDeclarations->getReductionArrayHostDeclaration (
              i)), addExpression2b);

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
                  OP2::VariableNames::threadItems)), buildIntVal (1),
              outerLoopBody);

      appendStatement (outerLoopStatement, subroutineScope);
    }
  }
}

void
FortranCUDAHostSubroutine::createReductionLocalVariableDeclarations ()
{
  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed for reduction",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVariableDeclaration * variableDeclaration1 =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::iterationCounter1,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  variableDeclarations->add (CommonVariableNames::iterationCounter1,
      variableDeclaration1);

  SgVariableDeclaration * variableDeclaration2 =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::iterationCounter2,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  variableDeclarations->add (CommonVariableNames::iterationCounter2,
      variableDeclaration2);

  SgVariableDeclaration * variableDeclaration3 =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::offset,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  variableDeclarations->add (OP2::VariableNames::offset, variableDeclaration3);

  SgVariableDeclaration * variableDeclaration4 =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::sharedMemoryBytes,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  variableDeclarations->add (OP2::VariableNames::sharedMemoryBytes,
      variableDeclaration4);

  SgVariableDeclaration * variableDeclaration5 =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::threadItems,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  variableDeclarations->add (OP2::VariableNames::threadItems,
      variableDeclaration5);
}

SgBasicBlock *
FortranCUDAHostSubroutine::createTransferOpDatStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating statements to transfer OP_DATs onto device",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise size of OP_DAT",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      /*
       * ======================================================
       * Statement to initialise size of OP_DAT
       * ======================================================
       */
      SgDotExp * dotExpression1 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::getOpDatName (i))),
          buildOpaqueVarRefExp (OP2::VariableNames::dimension, block));

      if (parallelLoop->isGlobal (i))
      {
        if (parallelLoop->isRead (i))
        {
          SgExprStatement * assignmentStatement = buildAssignStatement (
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::getOpDatSizeName (i))), dotExpression1);

          appendStatement (assignmentStatement, block);
        }
      }
      else
      {
        SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::getOpDatName (i))),
            buildOpaqueVarRefExp (OP2::VariableNames::set, block));

        SgDotExp * dotExpression3 = buildDotExp (dotExpression2,
            buildOpaqueVarRefExp (OP2::VariableNames::size, block));

        SgExpression * multiplyExpression = buildMultiplyOp (dotExpression1,
            dotExpression3);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getOpDatSizeName (i))), multiplyExpression);

        appendStatement (assignmentStatement, block);
      }
    }
  }

  Debug::getInstance ()->debugMessage (
      "Creating statements to convert OP_DATs between C and Fortran pointers",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isGlobalScalar (i))
      {
        Debug::getInstance ()->debugMessage ("Global scalar conversion",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        SgDotExp * parameterExpression1A = buildDotExp (buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::getOpDatName (i))),
            buildOpaqueVarRefExp (OP2::VariableNames::dataOnHost, block));

        SgVarRefExp * parameterExpression1B =
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getOpDatHostName (i)));

        SgStatement * callStatementA =
            SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
                subroutineScope, parameterExpression1A, parameterExpression1B);

        appendStatement (callStatementA, block);
      }
      else
      {
        /*
         * ======================================================
         * Statement to convert OP_DAT between C and Fortran
         * pointers
         * ======================================================
         */

        Debug::getInstance ()->debugMessage (
            "Direct/Indirect/Global non-scalar conversion",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        SgDotExp * parameterExpression1A = buildDotExp (buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::getOpDatName (i))),
            buildOpaqueVarRefExp (OP2::VariableNames::dataOnDevice, block));

        SgVarRefExp * parameterExpression2A = buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::getOpDatDeviceName (
                i)));

        SgExpression * parameterExpression3A = buildOpaqueVarRefExp ("(/"
            + buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getOpDatSizeName (i)))->unparseToString ()
            + "/)", block);

        SgStatement * callStatementA =
            SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
                subroutineScope, parameterExpression1A, parameterExpression2A,
                parameterExpression3A);

        appendStatement (callStatementA, block);

        if (parallelLoop->isGlobal (i))
        {
          Debug::getInstance ()->debugMessage (
              "Global non-scalar additional conversion",
              Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

          SgDotExp * parameterExpression1B = buildDotExp (
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::getOpDatName (i))), buildOpaqueVarRefExp (
                  OP2::VariableNames::dataOnHost, block));

          SgVarRefExp * parameterExpression2B = buildVarRefExp (
              variableDeclarations->get (OP2::VariableNames::getOpDatHostName (
                  i)));

          SgExpression
              * parameterExpression3B =
                  buildOpaqueVarRefExp (
                      "(/"
                          + buildVarRefExp (variableDeclarations->get (
                              OP2::VariableNames::getOpDatSizeName (i)))->unparseToString ()
                          + "/)", block);

          SgStatement * callStatementB =
              SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
                  subroutineScope, parameterExpression1B,
                  parameterExpression2B, parameterExpression3B);

          appendStatement (callStatementB, block);
        }
      }
    }
  }

  Debug::getInstance ()->debugMessage (
      "Creating statements to transfer OP_GBL from host to device",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isGlobalNonScalar (i))
      {
        Debug::getInstance ()->debugMessage ("Global non-scalar transfer",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getOpDatDeviceName (i))), buildVarRefExp (
                variableDeclarations->get (
                    OP2::VariableNames::getOpDatHostName (i))));

        appendStatement (assignmentStatement, block);
      }
    }
  }

  return block;
}

SgBasicBlock *
FortranCUDAHostSubroutine::createFirstTimeExecutionStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildBoolValExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildMultiplyOp;
  using SageInterface::appendStatement;

  SgBasicBlock * block = buildBasicBlock ();

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise OP_DAT dimensions",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isGlobalScalar (i) == false)
    {
      SgVarRefExp * opDatDimensionsReference = buildVarRefExp (
          moduleDeclarations->getDimensionsVariableDeclaration ());

      SgVarRefExp * fieldReference = buildVarRefExp (
          opDatDimensionsDeclaration->getOpDatDimensionField (i));

      SgDotExp * dotExpression1 = buildDotExp (opDatDimensionsReference,
          fieldReference);

      SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::getOpDatName (i))),
          buildOpaqueVarRefExp (OP2::VariableNames::dimension, block));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          dotExpression1, dotExpression2);

      appendStatement (assignmentStatement, block);
    }
  }

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise OP_DAT sizes", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgDotExp * dotExpression1 = buildDotExp (buildVarRefExp (
          moduleDeclarations->getDataSizesVariableDeclaration ()),
          buildVarRefExp (dataSizesDeclaration->getFieldDeclarations ()->get (
              OP2::VariableNames::getOpDatSizeName (i))));

      SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::getOpDatName (i))),
          buildOpaqueVarRefExp (OP2::VariableNames::dimension, block));

      if (parallelLoop->isGlobalNonScalar (i))
      {
        if (parallelLoop->isRead (i))
        {
          SgExprStatement * assignmentStatement = buildAssignStatement (
              dotExpression1, dotExpression2);

          appendStatement (assignmentStatement, block);
        }
        else if (parallelLoop->isWritten (i) || parallelLoop->isReadAndWritten (
            i))
        {
          SgPntrArrRefExp * arrayIndexExpression3 = buildPntrArrRefExp (
              buildVarRefExp (
                  moduleDeclarations->getReductionArrayHostDeclaration (i)),
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::threadItems)));

          FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
              buildExprListExp (arrayIndexExpression3), block);

          SgPntrArrRefExp * arrayIndexExpression4 = buildPntrArrRefExp (
              buildVarRefExp (
                  moduleDeclarations->getReductionArrayDeviceDeclaration (i)),
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::threadItems)));

          FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
              buildExprListExp (arrayIndexExpression4), block);
        }
        else if (parallelLoop->isIncremented (i))
        {
          Debug::getInstance ()->debugMessage (
              "Non-scalar OP_GBL with increment access",
              Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);
        }
      }
      else
      {
        SgDotExp * dotExpression3 = buildDotExp (buildVarRefExp (
            variableDeclarations->get (OP2::VariableNames::getOpDatName (i))),
            buildOpaqueVarRefExp (OP2::VariableNames::set, block));

        SgDotExp * dotExpression4 = buildDotExp (dotExpression3,
            buildOpaqueVarRefExp (OP2::VariableNames::size, block));

        SgExpression * multiplyExpression = buildMultiplyOp (dotExpression2,
            dotExpression4);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            dotExpression1, multiplyExpression);

        appendStatement (assignmentStatement, block);
      }
    }
  }

  return block;
}

void
FortranCUDAHostSubroutine::createDataMarshallingLocalVariableDeclarations ()
{
  using boost::lexical_cast;
  using SageBuilder::buildPointerType;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations for data marshalling",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      Debug::getInstance ()->debugMessage (
          "Creating OP_DAT size variable for OP_DAT " + lexical_cast <string> (
              i), Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

      string const & variableName = OP2::VariableNames::getOpDatSizeName (i);

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
      if (parallelLoop->isGlobal (i) && parallelLoop->isRead (i))
      {
        if (parallelLoop->isGlobalScalar (i))
        {
          string const & variableName =
              OP2::VariableNames::getOpDatHostName (i);

          variableDeclarations->add (
              variableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  variableName, parallelLoop->getOpDatType (i), subroutineScope));
        }
        else
        {
          string const & variableName = OP2::VariableNames::getOpDatDeviceName (
              i);

          variableDeclarations->add (
              variableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  variableName, parallelLoop->getOpDatBaseType (i),
                  subroutineScope, 2, DEVICE, ALLOCATABLE));
        }
      }
      else if (parallelLoop->isGlobal (i) && parallelLoop->isRead (i) == false)
      {
        SgType * baseType;

        if (parallelLoop->isGlobalScalar (i))
        {
          baseType = parallelLoop->getOpDatType (i);
        }
        else
        {
          baseType = parallelLoop->getOpDatBaseType (i);
        }

        string const & variableNameOnDevice =
            OP2::VariableNames::getOpDatDeviceName (i);

        variableDeclarations->add (variableNameOnDevice,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableNameOnDevice, baseType, subroutineScope, 2, DEVICE,
                ALLOCATABLE));

        string const & variableNameOnHost =
            OP2::VariableNames::getOpDatHostName (i);

        variableDeclarations->add (variableNameOnHost,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableNameOnHost, buildPointerType (baseType),
                subroutineScope));
      }
      else
      {
        string const & variableName =
            OP2::VariableNames::getOpDatDeviceName (i);

        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, parallelLoop->getOpDatBaseType (i),
                subroutineScope, 2, DEVICE, ALLOCATABLE));
      }
    }
  }
}

void
FortranCUDAHostSubroutine::createCUDAKernelLocalVariableDeclarations ()
{
  Debug::getInstance ()->debugMessage (
      "Creating CUDA kernel local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (CUDA::blocksPerGrid,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::blocksPerGrid, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (CUDA::threadsPerBlock,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::threadsPerBlock, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (CUDA::sharedMemorySize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::sharedMemorySize, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (CUDA::threadSynchRet,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::threadSynchRet, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));
}

FortranCUDAHostSubroutine::FortranCUDAHostSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    std::string const & kernelSubroutineName,
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    FortranCUDADataSizesDeclaration * dataSizesDeclaration,
    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration,
    FortranCUDAModuleDeclarations * moduleDeclarations) :
  FortranHostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop, moduleScope), dataSizesDeclaration (
      dataSizesDeclaration), opDatDimensionsDeclaration (
      opDatDimensionsDeclaration), moduleDeclarations (moduleDeclarations)
{
  using SageInterface::addTextForUnparser;

  addTextForUnparser (subroutineHeaderStatement, "attributes(host) ",
      AstUnparseAttribute::e_before);
}
