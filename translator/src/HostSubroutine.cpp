#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <Debug.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <HostSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

     std::string
    HostSubroutine::getUserSubroutineFormalParameterName ()
    {
      return "subroutineName";
    }


     std::string
    HostSubroutine::get_OP_SET_FormalParameterName ()
    {
      return "set";
    }


     std::string
    HostSubroutine::get_OP_INDIRECTION_FormalParameterName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_INDIRECTION
          + lexical_cast <string> (OP_DAT_ArgumentGroup);
    }


     std::string
    HostSubroutine::get_OP_MAP_FormalParameterName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_MAP
          + lexical_cast <string> (OP_DAT_ArgumentGroup);
    }


     std::string
    HostSubroutine::get_OP_ACCESS_FormalParameterName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_ACCESS
          + lexical_cast <string> (OP_DAT_ArgumentGroup);
    }

std::string
    HostSubroutine::get_OP_DAT_FormalParameterName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
          + lexical_cast <string> (OP_DAT_ArgumentGroup);
    }


     std::string
    HostSubroutine::get_OP_DAT_SizeVariableName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
          + lexical_cast <string> (OP_DAT_ArgumentGroup)
          + IndirectAndDirectLoop::Fortran::VariableSuffixes::Size;
    }


     std::string
    HostSubroutine::get_OP_DAT_DeviceVariableName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
          + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Device";
    }


     std::string
    HostSubroutine::getCToFortranVariableName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return "cFortranPointer" + lexical_cast <string> (OP_DAT_ArgumentGroup);
    }








void
HostSubroutine::copyDataBackFromDeviceAndDeallocate (
    ParallelLoop & parallelLoop)
{
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildAssignOp;
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

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.isReductionRequired (i) == false)
    {
      SgVarRefExp * opDatDeviceReference = buildVarRefExp (
          localVariableDeclarations[get_OP_DAT_DeviceVariableName (i)]);

      SgVarRefExp * c2FortranPointerReference = buildVarRefExp (
          localVariableDeclarations[getCToFortranVariableName (i)]);

      SgExpression * assignmentExpression = buildAssignOp (
          c2FortranPointerReference, opDatDeviceReference);

      SgExprStatement * assignmentStatement = buildExprStatement (
          assignmentExpression);

      appendStatement (assignmentStatement, subroutineScope);
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.isReductionRequired (i) == false)
    {
      SgVarRefExp * opDatDeviceReference = buildVarRefExp (
          localVariableDeclarations[get_OP_DAT_DeviceVariableName (i)]);

      SgExprListExp * deallocateParameters = buildExprListExp (
          opDatDeviceReference);

      FortranStatementsAndExpressionsBuilder::appendDeallocateStatement (
          deallocateParameters, subroutineScope);
    }
  }
}

void
HostSubroutine::initialiseDataMarshallingLocalVariables (
    ParallelLoop & parallelLoop)
{
  using SageBuilder::buildVoidType;
  using SageBuilder::buildIntType;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageInterface::appendStatement;
  using std::map;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Initialising data marshalling local variables", 2);

  map <string, OP_DAT_Declaration *>::const_iterator OP_DAT_iterator;

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
      SgVarRefExp * opDatFormalArgumentReference = buildVarRefExp (
          formalParameterDeclarations[get_OP_DAT_FormalParameterName (i)]);

      SgVarRefExp * opDatSizeReference = buildVarRefExp (
          localVariableDeclarations[get_OP_DAT_SizeVariableName (i)]);

      SgExpression * setField = buildDotExp (opDatFormalArgumentReference,
          buildOpaqueVarRefExp ("set", subroutineScope));

      SgExpression * setSizeField = buildDotExp (setField,
          buildOpaqueVarRefExp ("size", subroutineScope));

      SgExpression * dimField = buildDotExp (opDatFormalArgumentReference,
          buildOpaqueVarRefExp ("dim", subroutineScope));

      SgExpression * multiplyExpression = buildMultiplyOp (dimField,
          setSizeField);

      SgExpression * assignmentExpression = buildAssignOp (opDatSizeReference,
          multiplyExpression);

      SgStatement * assignmentStatement = buildExprStatement (
          assignmentExpression);

      appendStatement (assignmentStatement, subroutineScope);
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
      SgVarRefExp * opDatFormalArgumentReference = buildVarRefExp (
          formalParameterDeclarations[get_OP_DAT_FormalParameterName (i)]);

      SgVarRefExp * c2FortranPointerReference = buildVarRefExp (
          localVariableDeclarations[getCToFortranVariableName (i)]);

      SgExpression * datField = buildDotExp (opDatFormalArgumentReference,
          buildOpaqueVarRefExp ("dat", subroutineScope));

      SgExpression * shapeExpression =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              localVariableDeclarations[get_OP_DAT_SizeVariableName (i)],
              subroutineScope);

      SgStatement * callStatement = createCallToC_F_POINTER (datField,
          c2FortranPointerReference, shapeExpression);

      appendStatement (callStatement, subroutineScope);
    }
  }

  /*
   * ======================================================
   * Allocation and copy in only used for op_dat
   * which are *not* reduction variables
   * ======================================================
   */

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.isReductionRequired (i) == false)
    {
      SgVarRefExp * opDatSizeReference = buildVarRefExp (
          localVariableDeclarations[get_OP_DAT_SizeVariableName (i)]);

      SgVarRefExp * opDatDeviceReference = buildVarRefExp (
          localVariableDeclarations[get_OP_DAT_DeviceVariableName (i)]);

      SgExprListExp * arrayIndexExpression = buildExprListExp (
          opDatSizeReference);

      SgPntrArrRefExp * subscriptExpression = buildPntrArrRefExp (
          opDatDeviceReference, arrayIndexExpression);

      FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
          buildExprListExp (subscriptExpression), subroutineScope);
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.isReductionRequired (i) == false)
    {
      SgVarRefExp * opDatDeviceReference = buildVarRefExp (
          localVariableDeclarations[get_OP_DAT_DeviceVariableName (i)]);

      SgVarRefExp * c2FortranPointerReference = buildVarRefExp (
          localVariableDeclarations[getCToFortranVariableName (i)]);

      SgExpression * assignmentExpression = buildAssignOp (
          opDatDeviceReference, c2FortranPointerReference);

      SgStatement * assignmentStatement = buildExprStatement (
          assignmentExpression);

      appendStatement (assignmentStatement, subroutineScope);
    }
  }
}

void
HostSubroutine::createDataMarshallingLocalVariables (
    ParallelLoop & parallelLoop)
{
  using boost::lexical_cast;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildPointerType;
  using SageInterface::appendStatement;
  using std::map;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variables to allow data marshalling between host and device",
      2);

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
      string const & variableName = get_OP_DAT_SizeVariableName (i);

      localVariableDeclarations[variableName]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope);
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
      string const & variableName = getCToFortranVariableName (i);

      SgArrayType * isArrayType = isSgArrayType (parallelLoop.get_OP_DAT_Type (
          i));

      localVariableDeclarations[variableName]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, buildPointerType (isArrayType), subroutineScope);
    }
  }

  /*
   * ======================================================
   * OP_DAT variable on device only needed if it is NOT a
   * reduction variable
   * ======================================================
   */
  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.isReductionRequired (i) == false)
    {
      string const & variableName = get_OP_DAT_DeviceVariableName (i);

      SgArrayType * isArrayType = isSgArrayType (parallelLoop.get_OP_DAT_Type (
          i));

      localVariableDeclarations[variableName]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, isArrayType, subroutineScope, 2, DEVICE,
              ALLOCATABLE);
    }
  }
}

void
HostSubroutine::createCUDAKernelVariables ()
{
  localVariableDeclarations[CUDA::Fortran::VariableNames::blocksPerGrid]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::Fortran::VariableNames::blocksPerGrid,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  localVariableDeclarations[CUDA::Fortran::VariableNames::threadsPerBlock]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::Fortran::VariableNames::threadsPerBlock,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  localVariableDeclarations[CUDA::Fortran::VariableNames::sharedMemorySize]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::Fortran::VariableNames::sharedMemorySize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::threadSynchronizeReturnVariableName]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectAndDirectLoop::Fortran::HostSubroutine::threadSynchronizeReturnVariableName,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);
}

void
HostSubroutine::createFormalParameters (
    UserDeviceSubroutine & userDeviceSubroutine, ParallelLoop & parallelLoop)
{
  using boost::iequals;
  using boost::lexical_cast;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntType;
  using SageInterface::appendStatement;
  using std::vector;
  using std::string;
  using std::map;

  Debug::getInstance ()->debugMessage (
      "Creating host subroutine formal parameters", 2);

  /*
   * ======================================================
   * Keep track of the OP_DAT group number
   * ======================================================
   */
  int OP_DAT_ArgumentGroup = 0;

  for (vector <SgExpression *>::const_iterator it =
      parallelLoop.getActualArguments ().begin (); it
      != parallelLoop.getActualArguments ().end (); ++it)
  {
    Debug::getInstance ()->debugMessage ("Argument type: "
        + (*it)->class_name (), 8);

    switch ((*it)->variantT ())
    {
      case V_SgFunctionRefExp:
      {
        /*
         * ======================================================
         * Found the user subroutine argument
         * ======================================================
         */
        string const & variableName = getUserSubroutineFormalParameterName ();

        formalParameterDeclarations[variableName]
            = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, FortranTypesBuilder::getString (
                    userDeviceSubroutine.getSubroutineName ().length ()),
                subroutineScope, formalParameters, 1, INTENT_IN);

        break;
      }

      case V_SgVarRefExp:
      {
        SgVarRefExp * variableReference = isSgVarRefExp (*it);

        switch (variableReference->get_type ()->variantT ())
        {
          case V_SgClassType:
          {
            SgClassType* classReference = isSgClassType (
                variableReference->get_type ());

            string const className = classReference->get_name ().getString ();

            if (iequals (className, OP2::OP_SET))
            {
              /*
               * ======================================================
               * Found an OP_SET argument
               * ======================================================
               */
              string const & variableName = get_OP_SET_FormalParameterName ();

              formalParameterDeclarations[variableName]
                  = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                      variableName, classReference, subroutineScope,
                      formalParameters, 1, INTENT_IN);
            }

            else if (iequals (className, OP2::OP_MAP))
            {
              /*
               * ======================================================
               * Found an OP_MAP argument
               * ======================================================
               */
              string const & variableName = get_OP_MAP_FormalParameterName (
                  OP_DAT_ArgumentGroup);

              formalParameterDeclarations[variableName]
                  = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                      variableName, classReference, subroutineScope,
                      formalParameters, 1, INTENT_IN);
            }

            else if (iequals (className, OP2::OP_DAT))
            {
              /*
               * ======================================================
               * Found an OP_DAT argument
               * ======================================================
               */

              OP_DAT_ArgumentGroup++;

              string const & variableName = get_OP_DAT_FormalParameterName (
                  OP_DAT_ArgumentGroup);

              formalParameterDeclarations[variableName]
                  = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                      variableName, classReference, subroutineScope,
                      formalParameters, 1, INTENT_IN);
            }
            else
            {
              Debug::getInstance ()->errorMessage ("Unrecognised class: "
                  + className);
            }

            break;
          }

          case V_SgTypeInt:
          {
            /*
             * ======================================================
             * Found an OP_ACCESS argument
             * ======================================================
             */

            Debug::getInstance ()->debugMessage ("Access descriptor found", 10);

            string const & variableName = get_OP_ACCESS_FormalParameterName (
                OP_DAT_ArgumentGroup);

            formalParameterDeclarations[variableName]
                = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                    variableName, FortranTypesBuilder::getFourByteInteger (),
                    subroutineScope, formalParameters, 1, INTENT_IN);

            break;
          }

          default:
          {
            break;
          }
        }

        break;
      }

      case V_SgMinusOp:
      case V_SgIntVal:
      {
        /*
         * ======================================================
         * Found an indirection argument
         * ======================================================
         */

        string const & variableName = get_OP_INDIRECTION_FormalParameterName (
            OP_DAT_ArgumentGroup);

        formalParameterDeclarations[variableName]
            = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, FortranTypesBuilder::getFourByteInteger (),
                subroutineScope, formalParameters, 1, INTENT_IN);

        break;
      }

      default:
      {
        break;
      }
    }
  }
}

SgStatement *
HostSubroutine::createCallToC_F_POINTER (SgExpression * parameter1,
    SgExpression * parameter2, SgExpression * parameter3)
{
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildExprStatement;
  using SageInterface::appendStatement;

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranSubroutine ("c_f_pointer",
          subroutineScope);

  SgExprListExp * actualParameters = buildExprListExp (parameter1, parameter2,
      parameter3);

  SgFunctionCallExp * subroutineCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  return buildExprStatement (subroutineCall);
}

SgStatement *
HostSubroutine::buildThreadSynchroniseFunctionCall (
    SgScopeStatement * subroutineScope)
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprStatement;

  SgFunctionSymbol * cudaThreadSynchronizeFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("cudaThreadSynchronize",
          subroutineScope);

  SgVarRefExp
      * threadSynchRetReference =
          buildVarRefExp (
              localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::threadSynchronizeReturnVariableName]);

  SgFunctionCallExp * threadSynchRetFunctionCall = buildFunctionCallExp (
      cudaThreadSynchronizeFunctionSymbol, buildExprListExp ());

  SgStatement * threadSynchFunctionCall = buildExprStatement (buildAssignOp (
      threadSynchRetReference, threadSynchRetFunctionCall));

  return threadSynchFunctionCall;
}

void
HostSubroutine::createReductionVariables (ParallelLoop & parallelLoop)
{
  using SageBuilder::buildIntVal;
  using SageBuilder::buildIntType;
  using SageBuilder::buildAssignInitializer;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;

  if (parallelLoop.isReductionRequired () == true)
  {
    for (unsigned int i = 1; i
        <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
    {
      if (parallelLoop.isReductionRequired (i) == true)
      {
        localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset]
            = buildVariableDeclaration (
                IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset,
                FortranTypesBuilder::getFourByteInteger (),
                buildAssignInitializer (buildIntVal (0), buildIntType ()),
                subroutineScope);

        localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset]-> get_declarationModifier ().get_accessModifier ().setUndefined ();

        appendStatement (
            localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset],
            subroutineScope);

        localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::maximumBytesInSharedMemory]
            = buildVariableDeclaration (
                IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::maximumBytesInSharedMemory,
                FortranTypesBuilder::getFourByteInteger (),
                buildAssignInitializer (buildIntVal (0), buildIntType ()),
                subroutineScope);

        localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::maximumBytesInSharedMemory]-> get_declarationModifier ().get_accessModifier ().setUndefined ();

        appendStatement (
            localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::maximumBytesInSharedMemory],
            subroutineScope);

        localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::numberOfThreadItems]
            = buildVariableDeclaration (
                IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::numberOfThreadItems,
                FortranTypesBuilder::getFourByteInteger (),
                buildAssignInitializer (buildIntVal (0), buildIntType ()),
                subroutineScope);

        localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::numberOfThreadItems]-> get_declarationModifier ().get_accessModifier ().setUndefined ();

        appendStatement (
            localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::numberOfThreadItems],
            subroutineScope);

        localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::maximumNumberOfThreadBlocks]
            = buildVariableDeclaration (
                IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::maximumNumberOfThreadBlocks,
                FortranTypesBuilder::getFourByteInteger (),
                buildAssignInitializer (buildIntVal (0), buildIntType ()),
                subroutineScope);

        localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::maximumNumberOfThreadBlocks]-> get_declarationModifier ().get_accessModifier ().setUndefined ();

        appendStatement (
            localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::maximumNumberOfThreadBlocks],
            subroutineScope);

        localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionArrayHost]
            = buildVariableDeclaration (
                IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionArrayHost,
                parallelLoop.get_OP_DAT_Type (i), NULL, subroutineScope);

        localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionArrayHost]-> get_declarationModifier ().get_accessModifier ().setUndefined ();
        localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionArrayHost]-> get_declarationModifier ().get_typeModifier ().setAllocatable ();

        appendStatement (
            localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionArrayHost],
            subroutineScope);

        localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionArrayDevice]
            = buildVariableDeclaration (
                IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionArrayDevice,
                parallelLoop.get_OP_DAT_Type (i), NULL, subroutineScope);

        localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionArrayDevice]-> get_declarationModifier ().get_accessModifier ().setUndefined ();
        localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionArrayDevice]-> get_declarationModifier ().get_typeModifier ().setAllocatable ();
        localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionArrayDevice]-> get_declarationModifier ().get_typeModifier ().setDevice ();

        appendStatement (
            localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionArrayDevice],
            subroutineScope);
      }
    }
  }
}

void
HostSubroutine::createAndAppendThreadSynchCall ()
{
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * threadSynchRet = cudaThreadSynchronize()
   * ======================================================
   */

  SgStatement * threadSynchCallStmt = buildThreadSynchroniseFunctionCall (
      subroutineScope);

  appendStatement (threadSynchCallStmt, subroutineScope);
}

void
HostSubroutine::createSupportForReductionVariablesBeforeKernel (
    ParallelLoop & parallelLoop)
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprStatement;
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
   * First check if we actualy need a reduction variable
   * support
   * ======================================================
   */

  if (parallelLoop.isReductionRequired () == false)
    return;

  /*
   * ======================================================
   * We need to allocate enough additional shared memory
   * thus we need the highest fortra kind for op_dats
   * of reduction variables
   * ======================================================
   */
  int maxUsedFortranKind = -1;

  int dim = -1;

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isReductionRequired (i) == true)
    {
      int currentDim = parallelLoop.get_OP_DAT_Dimension (i);

      if (currentDim > dim)
      {
        dim = currentDim;
      }

      SgExpression * fortranKind =
          FortranStatementsAndExpressionsBuilder::getFortranKindOf_OP_DAT (
              parallelLoop.get_OP_DAT_Type (i));

      SgIntVal * kindVal = isSgIntVal (fortranKind);

      ROSE_ASSERT ( kindVal != NULL );

      int actualValue = -1;

      if (maxUsedFortranKind < (actualValue = kindVal->get_value ()))
      {
        maxUsedFortranKind = actualValue;
      }
    }
  }

  SgVarRefExp
      * maxBlockVarRef =
          buildVarRefExp (
              localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::maximumNumberOfThreadBlocks]);

  SgExpression * initMaxBlocks = buildAssignOp (maxBlockVarRef, buildVarRefExp (
      localVariableDeclarations[CUDA::Fortran::VariableNames::blocksPerGrid]));

  appendStatement (buildExprStatement (initMaxBlocks), subroutineScope);

  SgExpression
      * assignReductItems =
          buildAssignOp (
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::numberOfThreadItems]),
              buildMultiplyOp (maxBlockVarRef, buildIntVal (dim)));

  appendStatement (buildExprStatement (assignReductItems), subroutineScope);

  SgVarRefExp
      * reductItemsVarRef =
          buildVarRefExp (
              localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::numberOfThreadItems]);

  SgVarRefExp
      * redArrayHost =
          buildVarRefExp (
              localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionArrayHost]);

  SgPntrArrRefExp * allocateHostRedVarParams = buildPntrArrRefExp (
      redArrayHost, reductItemsVarRef);

  FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
      buildExprListExp (allocateHostRedVarParams), subroutineScope);

  SgVarRefExp
      * redArrayDev =
          buildVarRefExp (
              localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionArrayDevice]);

  SgPntrArrRefExp * allocateDeviceRedVarParams = buildPntrArrRefExp (
      redArrayDev, reductItemsVarRef);

  FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
      buildExprListExp (allocateDeviceRedVarParams), subroutineScope);

  SgVarRefExp
      * itVar1VarRef =
          buildVarRefExp (
              localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter1]);

  SgExpression * initLoop = buildAssignOp (itVar1VarRef, buildIntVal (0));

  /*
   * ======================================================
   * Warning: for now, only real values are supported!
   * ======================================================
   */

  SgExpression * setToZeroRedArrayHost = buildAssignOp (buildPntrArrRefExp (
      redArrayHost, itVar1VarRef), buildFloatVal (0.0));

  SgBasicBlock * initLoopBody = buildBasicBlock (buildExprStatement (
      setToZeroRedArrayHost));

  SgFortranDo * initLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initLoop, reductItemsVarRef, buildIntVal (1), initLoopBody);

  appendStatement (initLoopStatement, subroutineScope);

  SgExpression * copyHostToDeviceArray = buildAssignOp (redArrayDev,
      redArrayHost);

  appendStatement (buildExprStatement (copyHostToDeviceArray), subroutineScope);

  SgVarRefExp
      * nSharedVarRef =
          buildVarRefExp (
              localVariableDeclarations[CUDA::Fortran::VariableNames::sharedMemorySize]);

  SgExpression
      * assignReductionOffset =
          buildAssignOp (
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset]),
              nSharedVarRef);

  appendStatement (buildExprStatement (assignReductionOffset), subroutineScope);

  SgVarRefExp
      * maxRedSizeVarRef =
          buildVarRefExp (
              localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::maximumBytesInSharedMemory]);

  SgExpression
      * initMaxShared =
          buildAssignOp (
              maxRedSizeVarRef,
              buildMultiplyOp (
                  buildVarRefExp (
                      localVariableDeclarations[CUDA::Fortran::VariableNames::threadsPerBlock]),
                  buildIntVal (maxUsedFortranKind)));

  appendStatement (buildExprStatement (initMaxShared), subroutineScope);

  SgExpression * recomputeNshared = buildAssignOp (nSharedVarRef, buildAddOp (
      nSharedVarRef, maxRedSizeVarRef));

  appendStatement (buildExprStatement (recomputeNshared), subroutineScope);
}

void
HostSubroutine::createSupportForReductionVariablesAfterKernel (
    ParallelLoop & parallelLoop)
{
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildBasicBlock;
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * First check if we actualy need a reduction variable
   * support
   * ======================================================
   */

  if (parallelLoop.isReductionRequired () == false)
    return;

  SgExpression
      * redArrayHostVarRef =
          buildVarRefExp (
              localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionArrayHost]);

  SgExpression
      * copyDeviceTohostArray =
          buildAssignOp (
              redArrayHostVarRef,
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionArrayDevice]));

  appendStatement (buildExprStatement (copyDeviceTohostArray), subroutineScope);

  /*
   * ======================================================
   * final reduce on host for each reduction variable
   * Warning: only one variable supported for now
   * ======================================================
   */

  /*
   * ======================================================
   * Get dimension of reduction variable (we remember the
   * position of the reduction variable in the op_dat
   * argument list in the variable positionInOPDatsArray
   * ======================================================
   */

  int dim = -1;
  int positionInOPDatsArray = -1;

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isReductionRequired (i) == true)
    {
      dim = parallelLoop.get_OP_DAT_Dimension (i);
      positionInOPDatsArray = i;
    }
  }

  SgVarRefExp
      * itVar2VarRef =
          buildVarRefExp (
              localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter2]);

  SgExpression * initInnerLoop = buildAssignOp (itVar2VarRef, buildIntVal (0));

  SgExpression * c2fPtrAccess = buildPntrArrRefExp (buildVarRefExp (
      localVariableDeclarations[getCToFortranVariableName (
          positionInOPDatsArray)]), buildAddOp (itVar2VarRef, buildIntVal (1)));

  SgVarRefExp
      * itVar1VarRef =
          buildVarRefExp (
              localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter1]);

  SgExpression * assignExpInHostReduction = buildAddOp (c2fPtrAccess,
      buildPntrArrRefExp (redArrayHostVarRef, buildAddOp (itVar2VarRef,
          buildMultiplyOp (itVar1VarRef, buildIntVal (dim)))));

  SgExpression * finalReduceOnHostExpr = buildAssignOp (c2fPtrAccess,
      assignExpInHostReduction);

  SgBasicBlock * innerLoopBody = buildBasicBlock (buildExprStatement (
      finalReduceOnHostExpr));

  SgFortranDo * innerRedCopyLoop =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initInnerLoop, buildIntVal (dim - 1), buildIntVal (1), innerLoopBody);

  SgBasicBlock * outerLoopBody = buildBasicBlock (innerRedCopyLoop);

  SgExpression * initOuterLoop = buildAssignOp (itVar1VarRef, buildIntVal (0));

  SgFortranDo
      * outerRedCopyLoop =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              initOuterLoop,
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::numberOfThreadItems]),
              buildIntVal (1), outerLoopBody);

  appendStatement (outerRedCopyLoop, subroutineScope);
}

void
HostSubroutine::createAndAppendIterationVariablesForReduction (
    ParallelLoop & parallelLoop)
{
  using SageBuilder::buildIntVal;
  using SageBuilder::buildIntType;
  using SageBuilder::buildAssignInitializer;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::string;

  if (parallelLoop.isReductionRequired () == true)
  {
    localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter1]
        = buildVariableDeclaration (
            IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter1,
            FortranTypesBuilder::getFourByteInteger (), buildAssignInitializer (
                buildIntVal (0), buildIntType ()), subroutineScope);

    localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter2]
        = buildVariableDeclaration (
            IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter2,
            FortranTypesBuilder::getFourByteInteger (), buildAssignInitializer (
                buildIntVal (0), buildIntType ()), subroutineScope);

    localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter1]-> get_declarationModifier ().get_accessModifier ().setUndefined ();

    localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter2]-> get_declarationModifier ().get_accessModifier ().setUndefined ();

    appendStatement (
        localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter1],
        subroutineScope);

    appendStatement (
        localVariableDeclarations[IndirectAndDirectLoop::Fortran::HostSubroutine::reductionIterationCounter2],
        subroutineScope);
  }
}

HostSubroutine::HostSubroutine (std::string const & subroutineName,
    SgScopeStatement * moduleScope) :
  Subroutine (subroutineName + SubroutineNameSuffixes::hostSuffix)
{
  using SageBuilder::buildFunctionParameterList;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildProcedureHeaderStatement;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  formalParameters = buildFunctionParameterList ();

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  appendStatement (subroutineHeaderStatement, moduleScope);

  addTextForUnparser (subroutineHeaderStatement, "attributes(host) ",
      AstUnparseAttribute::e_before);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();
}
