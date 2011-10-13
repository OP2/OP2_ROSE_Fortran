#include <FortranCUDAHostSubroutine.h>
#include <FortranKernelSubroutine.h>
#include <FortranParallelLoop.h>
#include <FortranCUDAOpDatCardinalitiesDeclaration.h>
#include <FortranCUDAModuleDeclarations.h>
#include <FortranOpDatDimensionsDeclaration.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <Debug.h>
#include <CommonNamespaces.h>
#include <CUDA.h>

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
    if (parallelLoop->isReductionRequired (i))
    {
      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (CUDA::blocksPerGrid)), buildIntVal (
          parallelLoop->getOpDatDimension (i)));

      SgExprStatement * assignmentStatement1 = buildAssignStatement (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getReductionCardinalityName (i))),
          multiplyExpression1);

      appendStatement (assignmentStatement1, subroutineScope);

      SgExprListExp * allocateParameters = buildExprListExp (
          buildPntrArrRefExp (buildVarRefExp (
              moduleDeclarations->getReductionArrayHostDeclaration (i)),
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::getReductionCardinalityName (i)))));

      FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
          allocateParameters, subroutineScope);

      SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (
          buildVarRefExp (moduleDeclarations->getReductionArrayHostDeclaration (
              i)), buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter1)));

      SgExprStatement * assignmentStatement2;

      if (isSgTypeInt (parallelLoop->getOpDatBaseType (i)))
      {
        assignmentStatement2 = buildAssignStatement (arrayIndexExpression1,
            buildIntVal (0));
      }
      else
      {
        assignmentStatement2 = buildAssignStatement (arrayIndexExpression1,
            buildFloatVal (0));
      }

      SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement2);

      SgAssignOp * loopInitialization = buildAssignOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter1)),
          buildIntVal (0));

      SgFortranDo * loopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              loopInitialization, buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::getReductionCardinalityName (i))),
              buildIntVal (1), loopBody);

      appendStatement (loopStatement, subroutineScope);

      SgExprStatement * assignmentStatement3 = buildAssignStatement (
          buildVarRefExp (
              moduleDeclarations->getReductionArrayDeviceDeclaration (i)),
          buildVarRefExp (moduleDeclarations->getReductionArrayHostDeclaration (
              i)));

      appendStatement (assignmentStatement3, subroutineScope);
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
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating reduction epilogue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i) == true)
    {
      if (parallelLoop->getOpDatDimension (i) == 1)
      {
        Debug::getInstance ()->debugMessage (
            "Creating statements for OP_DAT argument '"
                + lexical_cast <string> (i) + "'", Debug::FUNCTION_LEVEL,
            __FILE__, __LINE__);

        /*
         * ======================================================
         * Assign device reduction array to host reduction array
         * ======================================================
         */

        SgExprStatement * assignmentStatement1 = buildAssignStatement (
            buildVarRefExp (
                moduleDeclarations->getReductionArrayHostDeclaration (i)),
            buildVarRefExp (
                moduleDeclarations->getReductionArrayDeviceDeclaration (i)));

        appendStatement (assignmentStatement1, subroutineScope);

        /*
         * ======================================================
         * Iterate over all elements in the reduction host array
         * and compute either the sum, maximum, or minimum
         * ======================================================
         */

        SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (
            buildVarRefExp (
                moduleDeclarations->getReductionArrayHostDeclaration (i)),
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)));

        SgExpression * reductionComputationExpression;

        if (parallelLoop->isIncremented (i))
        {
          reductionComputationExpression = buildAddOp (arrayIndexExpression1,
              buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::getOpDatHostName (i))));
        }
        else if (parallelLoop->isMaximised (i))
        {
          SgFunctionSymbol * maxFunctionSymbol =
              FortranTypesBuilder::buildNewFortranFunction ("max",
                  subroutineScope);

          SgExprListExp * actualParameters = buildExprListExp (
              arrayIndexExpression1, buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::getOpDatHostName (i))));

          reductionComputationExpression = buildFunctionCallExp (
              maxFunctionSymbol, actualParameters);
        }
        else if (parallelLoop->isMinimised (i))
        {
          SgFunctionSymbol * maxFunctionSymbol =
              FortranTypesBuilder::buildNewFortranFunction ("min",
                  subroutineScope);

          SgExprListExp * actualParameters = buildExprListExp (
              arrayIndexExpression1, buildVarRefExp (variableDeclarations->get (
                  OP2::VariableNames::getOpDatHostName (i))));

          reductionComputationExpression = buildFunctionCallExp (
              maxFunctionSymbol, actualParameters);
        }

        ROSE_ASSERT (reductionComputationExpression != NULL);

        SgExprStatement * assignmentStatement3 = buildAssignStatement (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getOpDatHostName (i))),
            reductionComputationExpression);

        SgBasicBlock * outerLoopBody = buildBasicBlock ();

        appendStatement (assignmentStatement3, outerLoopBody);

        SgExpression * outerLoopInitialization = buildAssignOp (
            buildVarRefExp (variableDeclarations->get (
                CommonVariableNames::iterationCounter1)), buildIntVal (0));

        SgFortranDo * outerLoopStatement =
            FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
                outerLoopInitialization, buildVarRefExp (
                    variableDeclarations->get (
                        OP2::VariableNames::getReductionCardinalityName (i))),
                buildIntVal (1), outerLoopBody);

        appendStatement (outerLoopStatement, subroutineScope);

        SgExprListExp * deallocateParameters = buildExprListExp (
            buildPntrArrRefExp (buildVarRefExp (
                moduleDeclarations->getReductionArrayHostDeclaration (i)),
                buildVarRefExp (variableDeclarations->get (
                    OP2::VariableNames::getReductionCardinalityName (i)))));

        FortranStatementsAndExpressionsBuilder::appendDeallocateStatement (
            deallocateParameters, subroutineScope);
      }
    }
  }
}

void
FortranCUDAHostSubroutine::createReductionLocalVariableDeclarations ()
{
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed for reduction",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (CommonVariableNames::iterationCounter1,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::iterationCounter1,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (CommonVariableNames::iterationCounter2,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::iterationCounter2,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      string const & variableName =
          OP2::VariableNames::getReductionCardinalityName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));
    }
  }
}

SgExpression *
FortranCUDAHostSubroutine::createRHSOfInitialiseOpDatCardinalityStatement (
    SgScopeStatement * scope, unsigned int OP_DAT_ArgumentGroup)
{
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildMultiplyOp;

  if (parallelLoop->isDirect (OP_DAT_ArgumentGroup)
      || parallelLoop->isIndirect (OP_DAT_ArgumentGroup))
  {
    SgDotExp * dotExpression1 = buildDotExp (buildVarRefExp (
        variableDeclarations->get (OP2::VariableNames::getOpDatName (
            OP_DAT_ArgumentGroup))), buildOpaqueVarRefExp (
        OP2::VariableNames::dimension, scope));

    SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
        variableDeclarations->get (OP2::VariableNames::getOpDatName (
            OP_DAT_ArgumentGroup))), buildOpaqueVarRefExp (
        OP2::VariableNames::set, scope));

    SgDotExp * dotExpression3 = buildDotExp (dotExpression2,
        buildOpaqueVarRefExp (OP2::VariableNames::size, scope));

    SgExpression * multiplyExpression = buildMultiplyOp (dotExpression1,
        dotExpression3);

    return multiplyExpression;
  }
  else if (parallelLoop->isReductionRequired (OP_DAT_ArgumentGroup))
  {
    if (parallelLoop->isGlobalScalar (OP_DAT_ArgumentGroup))
    {
      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::getOpSetName ())),
          buildOpaqueVarRefExp (OP2::VariableNames::size, scope));

      return dotExpression;
    }
    else
    {
      SgDotExp * dotExpression1 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::getOpDatName (
              OP_DAT_ArgumentGroup))), buildOpaqueVarRefExp (
          OP2::VariableNames::dimension, scope));

      SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::getOpSetName ())),
          buildOpaqueVarRefExp (OP2::VariableNames::size, scope));

      SgExpression * multiplyExpression = buildMultiplyOp (dotExpression1,
          dotExpression2);

      return multiplyExpression;
    }
  }
  else if (parallelLoop->isGlobalArray (OP_DAT_ArgumentGroup)
      && parallelLoop->isRead (OP_DAT_ArgumentGroup))
  {
    SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
        variableDeclarations->get (OP2::VariableNames::getOpDatName (
            OP_DAT_ArgumentGroup))), buildOpaqueVarRefExp (
        OP2::VariableNames::dimension, scope));

    return dotExpression;
  }
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
      "Creating statements to initialise OP_DAT dimensions",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isGlobalRead (i) == false)
    {
      SgDotExp * dotExpression1 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::opDatDimensions)),
          buildVarRefExp (dimensionsDeclaration->getOpDatDimensionField (i)));

      SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (OP2::VariableNames::getOpDatName (i))),
          buildOpaqueVarRefExp (OP2::VariableNames::dimension, block));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          dotExpression1, dotExpression2);

      appendStatement (assignmentStatement, block);
    }
  }

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise OP_DAT cardinalities (on device)",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      FortranParallelLoop * fortranParallelLoop =
          static_cast <FortranParallelLoop *> (parallelLoop);

      if (fortranParallelLoop->isCardinalityDeclarationNeeded (i))
      {
        SgDotExp * dotExpression = buildDotExp (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::opDatCardinalities)), buildVarRefExp (
                cardinalitiesDeclaration->getFieldDeclarations ()->get (
                    OP2::VariableNames::getOpDatCardinalityName (i))));

        SgExpression * rhsOfAssigment =
            createRHSOfInitialiseOpDatCardinalityStatement (block, i);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            dotExpression, rhsOfAssigment);

        appendStatement (assignmentStatement, block);
      }
    }
  }

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise OP_DAT cardinalities (on host)",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      FortranParallelLoop * fortranParallelLoop =
          static_cast <FortranParallelLoop *> (parallelLoop);

      if (fortranParallelLoop->isCardinalityDeclarationNeeded (i))
      {
        SgExpression * rhsOfAssigment =
            createRHSOfInitialiseOpDatCardinalityStatement (block, i);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getOpDatCardinalityName (i))),
            rhsOfAssigment);

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

        SgAggregateInitializer * parameterExpression3A =
            FortranStatementsAndExpressionsBuilder::buildShapeExpression (
                variableDeclarations->get (
                    OP2::VariableNames::getOpDatCardinalityName (i)));

        SgStatement * callStatementA =
            SubroutineCalls::Fortran::createCToFortranPointerCallStatement (
                subroutineScope, parameterExpression1A, parameterExpression2A,
                parameterExpression3A);

        appendStatement (callStatementA, block);
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
      FortranParallelLoop * fortranParallelLoop =
          static_cast <FortranParallelLoop *> (parallelLoop);

      if (fortranParallelLoop->isCardinalityDeclarationNeeded (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating OP_DAT size variable for OP_DAT "
                + lexical_cast <string> (i), Debug::HIGHEST_DEBUG_LEVEL,
            __FILE__, __LINE__);

        string const & variableName =
            OP2::VariableNames::getOpDatCardinalityName (i);

        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, FortranTypesBuilder::getFourByteInteger (),
                subroutineScope));
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isGlobal (i))
      {
        if (parallelLoop->isGlobalScalar (i))
        {
          string const & variableName =
              OP2::VariableNames::getOpDatHostName (i);

          variableDeclarations->add (
              variableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  variableName, buildPointerType (
                      parallelLoop->getOpDatBaseType (i)), subroutineScope));
        }
        else
        {
          string const & variableName = OP2::VariableNames::getOpDatDeviceName (
              i);

          variableDeclarations->add (
              variableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  variableName, FortranTypesBuilder::getArray_RankOne (
                      parallelLoop->getOpDatBaseType (i)), subroutineScope, 2,
                  CUDA_DEVICE, ALLOCATABLE));
        }
      }
      else
      {
        string const & variableName =
            OP2::VariableNames::getOpDatDeviceName (i);

        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, FortranTypesBuilder::getArray_RankOne (
                    parallelLoop->getOpDatBaseType (i)), subroutineScope, 2,
                CUDA_DEVICE, ALLOCATABLE));
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

void
FortranCUDAHostSubroutine::createOpDatCardinalitiesDeclaration ()
{
  Debug::getInstance ()->debugMessage (
      "Generating OP_DAT cardinalities declaration ", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (
      OP2::VariableNames::opDatCardinalities,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::opDatCardinalities,
          cardinalitiesDeclaration->getType (), subroutineScope, 1, CUDA_DEVICE));
}

void
FortranCUDAHostSubroutine::createOpDatDimensionsDeclaration ()
{
  Debug::getInstance ()->debugMessage (
      "Generating OP_DAT dimensions declaration", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (OP2::VariableNames::opDatDimensions,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::opDatDimensions,
          dimensionsDeclaration->getType (), subroutineScope, 1, CUDA_DEVICE));
}

FortranCUDAHostSubroutine::FortranCUDAHostSubroutine (
    SgScopeStatement * moduleScope, FortranKernelSubroutine * kernelSubroutine,
    FortranParallelLoop * parallelLoop,
    FortranCUDAOpDatCardinalitiesDeclaration * dataSizesDeclaration,
    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration,
    FortranCUDAModuleDeclarations * moduleDeclarations) :
  FortranHostSubroutine (moduleScope, kernelSubroutine, parallelLoop),
      cardinalitiesDeclaration (dataSizesDeclaration), dimensionsDeclaration (
          opDatDimensionsDeclaration), moduleDeclarations (moduleDeclarations)
{
  subroutineHeaderStatement->get_functionModifier ().setCudaHost ();
}
