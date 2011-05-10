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
	 * Copy back and de-allocation only used for op_dat
	 * which are *not* reduction variables
	 * ======================================================
	 */	

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
			if ( parallelLoop.isReductionRequiredForSpecificArgument ( i ) == false ) {
	
				SgVarRefExp * opDatDeviceReference = buildVarRefExp (
						localVariables_OP_DAT_VariablesOnDevice[i]);

				SgVarRefExp * c2FortranPointerReference = buildVarRefExp (
						localVariables_CToFortranPointers[i]);

				SgExpression * assignmentExpression = buildAssignOp (
						c2FortranPointerReference, opDatDeviceReference);

				SgExprStatement * assignmentStatement = buildExprStatement (
						assignmentExpression);

				appendStatement (assignmentStatement, subroutineScope);
			}
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
			if ( parallelLoop.isReductionRequiredForSpecificArgument ( i ) == false )
			{
				SgVarRefExp * opDatDeviceReference = buildVarRefExp (
						localVariables_OP_DAT_VariablesOnDevice[i]);

				SgExprListExp * deallocateParameters = buildExprListExp (
						opDatDeviceReference);

				FortranStatementsAndExpressionsBuilder::appendDeallocateStatement (
						deallocateParameters, subroutineScope);
			}
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
          formalParameters_OP_DAT[i]);

      SgVarRefExp * opDatSizeReference = buildVarRefExp (
          localVariables_OP_DAT_Sizes[i]);

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
          formalParameters_OP_DAT[i]);

      SgVarRefExp * c2FortranPointerReference = buildVarRefExp (
          localVariables_CToFortranPointers[i]);

      SgExpression * datField = buildDotExp (opDatFormalArgumentReference,
          buildOpaqueVarRefExp ("dat", subroutineScope));

      SgExpression * shapeExpression =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              localVariables_OP_DAT_Sizes[i], subroutineScope);

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
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
			if ( parallelLoop.isReductionRequiredForSpecificArgument ( i ) == false )
			{
				SgVarRefExp * opDatSizeReference = buildVarRefExp (
						localVariables_OP_DAT_Sizes[i]);

				SgVarRefExp * opDatDeviceReference = buildVarRefExp (
						localVariables_OP_DAT_VariablesOnDevice[i]);

				SgExprListExp * arrayIndexExpression = buildExprListExp (
						opDatSizeReference);

				SgPntrArrRefExp * subscriptExpression = buildPntrArrRefExp (
						opDatDeviceReference, arrayIndexExpression);

				FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
						buildExprListExp (subscriptExpression), subroutineScope);
			}
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
			if ( parallelLoop.isReductionRequiredForSpecificArgument ( i ) == false )
			{
				SgVarRefExp * opDatDeviceReference = buildVarRefExp (
						localVariables_OP_DAT_VariablesOnDevice[i]);

				SgVarRefExp * c2FortranPointerReference = buildVarRefExp (
						localVariables_CToFortranPointers[i]);

				SgExpression * assignmentExpression = buildAssignOp (
						opDatDeviceReference, c2FortranPointerReference);

				SgStatement * assignmentStatement = buildExprStatement (
						assignmentExpression);

				appendStatement (assignmentStatement, subroutineScope);
			}
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
  using std::make_pair;
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
      string const variableName = "arg" + lexical_cast <string> (i) + "Size";

      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          subroutineScope);

      variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      localVariables_OP_DAT_Sizes.insert (make_pair (i, variableDeclaration));

      appendStatement (variableDeclaration, subroutineScope);
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
      string const variableName = "c2fPtrArg" + lexical_cast <string> (i);

      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          variableName, buildPointerType (
              FortranTypesBuilder::getArray_RankOne (
                  FortranTypesBuilder::getDoublePrecisionFloat ())), NULL,
          subroutineScope);

      variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      localVariables_CToFortranPointers.insert (make_pair (i,
          variableDeclaration));

      appendStatement (variableDeclaration, subroutineScope);
    }
  }

	/*
	 * ======================================================
	 * argument variable on device only needed if op_dat
	 * argument is *not* a reduction variable
	 * ======================================================
	 */
	

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
			if ( parallelLoop.isReductionRequiredForSpecificArgument ( i ) == false )
			{		
				string const variableName = kernelDatArgumentsNames::argNamePrefix + 
				  lexical_cast <string> (i);

				SgVariableDeclaration * variableDeclaration =
						buildVariableDeclaration (variableName, parallelLoop.get_OP_DAT_Type (
								i), NULL, subroutineScope);

				variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
				variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();
				variableDeclaration->get_declarationModifier ().get_typeModifier ().setAllocatable ();

				localVariables_OP_DAT_VariablesOnDevice.insert (make_pair (i,
						variableDeclaration));

				appendStatement (variableDeclaration, subroutineScope);
			}
    }
  }
}

void
HostSubroutine::createCUDAKernelVariables ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::string;

  string const CUDAVariable_blocksPerGridVariableName = "nblocks";
  string const CUDAVariable_threadsPerBlockVariableName = "nthread";
  string const CUDAVariable_sharedMemorySizeVariableName = "nshared";

  CUDAVariable_blocksPerGrid = buildVariableDeclaration (
      CUDAVariable_blocksPerGridVariableName,
      FortranTypesBuilder::getFourByteInteger (), NULL, subroutineScope);

  CUDAVariable_threadsPerBlock = buildVariableDeclaration (
      CUDAVariable_threadsPerBlockVariableName,
      FortranTypesBuilder::getFourByteInteger (), NULL, subroutineScope);

  CUDAVariable_sharedMemorySize = buildVariableDeclaration (
      CUDAVariable_sharedMemorySizeVariableName,
      FortranTypesBuilder::getFourByteInteger (), NULL, subroutineScope);

  CUDAVariable_blocksPerGrid->get_declarationModifier ().get_accessModifier ().setUndefined ();
  CUDAVariable_threadsPerBlock->get_declarationModifier ().get_accessModifier ().setUndefined ();
  CUDAVariable_sharedMemorySize->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (CUDAVariable_blocksPerGrid, subroutineScope);
  appendStatement (CUDAVariable_threadsPerBlock, subroutineScope);
  appendStatement (CUDAVariable_sharedMemorySize, subroutineScope);

  /*
   * ======================================================
   * Also builds threadSynchRet
   * ======================================================
   */
  localVariables_Others[variableName_threadSynchRet]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName_threadSynchRet,
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
  using std::make_pair;

  Debug::getInstance ()->debugMessage (
      "Creating host subroutine formal parameters", 2);

  string const OP_DAT_VariableNamePrefix = "arg";
  string const OP_INDIRECTION_VariableNamePrefix = "idx";
  string const OP_MAP_VariableNamePrefix = "map";
  string const OP_ACCESS_VariableNamePrefix = "access";

  /*
   * ======================================================
   * This variable is the integer suffix appended to formal
   * parameters in an OP_DAT batch of arguments
   * ======================================================
   */
  int variableNameSuffix = 0;

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

        string const variableName = "subroutineName";

        SgVariableDeclaration * charArrayDeclaration =
            buildVariableDeclaration (variableName,
                FortranTypesBuilder::getString (
                    userDeviceSubroutine.getSubroutineName ().length ()), NULL,
                subroutineScope);

        formalParameters->append_arg (
            *(charArrayDeclaration->get_variables ().begin ()));

        charArrayDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();
        charArrayDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

        appendStatement (charArrayDeclaration, subroutineScope);

        formalParameter_SubroutineName = charArrayDeclaration;

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

            if (iequals (className, OP2::OP_SET_NAME))
            {
              /*
               * ======================================================
               * Found an OP_SET argument
               * ======================================================
               */
              Debug::getInstance ()->debugMessage ("OP_SET found", 10);

              string const variableName = "set";

              SgVariableDeclaration * opSetDeclaration =
                  buildVariableDeclaration (variableName, classReference, NULL,
                      subroutineScope);

              opSetDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();
              opSetDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

              formalParameters->append_arg (
                  *(opSetDeclaration->get_variables ().begin ()));

              appendStatement (opSetDeclaration, subroutineScope);

              formalParameter_OP_SET = opSetDeclaration;
            }

            else if (iequals (className, OP2::OP_MAP_NAME))
            {
              /*
               * ======================================================
               * Found an OP_MAP argument
               * ======================================================
               */
              Debug::getInstance ()->debugMessage ("OP_MAP found", 10);

              string const variableName = OP_MAP_VariableNamePrefix
                  + lexical_cast <string> (variableNameSuffix);

              SgVariableDeclaration * opMapDeclaration =
                  buildVariableDeclaration (variableName, classReference, NULL,
                      subroutineScope);

              opMapDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();
              opMapDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

              formalParameters->append_arg (
                  *(opMapDeclaration->get_variables ().begin ()));

              appendStatement (opMapDeclaration, subroutineScope);

              formalParameters_OP_MAP.insert (make_pair (variableNameSuffix,
                  opMapDeclaration));
            }

            else if (iequals (className, OP2::OP_DAT_NAME))
            {
              /*
               * ======================================================
               * Found an OP_DAT argument
               * ======================================================
               */

              Debug::getInstance ()->debugMessage ("OP_DAT found", 10);

              /*
               * ======================================================
               * A new batch of OP_DAT arguments has been discovered
               * Therefore, increment the variable name suffix
               * ======================================================
               */
              variableNameSuffix++;

              string const variableName = OP_DAT_VariableNamePrefix
                  + lexical_cast <string> (variableNameSuffix);

              SgVariableDeclaration * opDatDeclaration =
                  buildVariableDeclaration (variableName, classReference, NULL,
                      subroutineScope);

              opDatDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();
              opDatDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

              formalParameters->append_arg (
                  *(opDatDeclaration->get_variables ().begin ()));

              appendStatement (opDatDeclaration, subroutineScope);

              formalParameters_OP_DAT.insert (make_pair (variableNameSuffix,
                  opDatDeclaration));
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

            string const variableName = OP_ACCESS_VariableNamePrefix
                + lexical_cast <string> (variableNameSuffix);

            SgVariableDeclaration * opAccessDeclaration =
                buildVariableDeclaration (variableName, buildIntType (), NULL,
                    subroutineScope);

            opAccessDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();
            opAccessDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

            formalParameters->append_arg (
                *(opAccessDeclaration->get_variables ().begin ()));

            appendStatement (opAccessDeclaration, subroutineScope);

            formalParameters_OP_ACCESS.insert (make_pair (variableNameSuffix,
                opAccessDeclaration));

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

        Debug::getInstance ()->debugMessage ("Index found", 10);

        string const variableName = OP_INDIRECTION_VariableNamePrefix
            + lexical_cast <string> (variableNameSuffix);

        SgVariableDeclaration * opIndirectionDeclaration =
            buildVariableDeclaration (variableName, buildIntType (), NULL,
                subroutineScope);

        opIndirectionDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();
        opIndirectionDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

        formalParameters->append_arg (
            *(opIndirectionDeclaration->get_variables ().begin ()));

        appendStatement (opIndirectionDeclaration, subroutineScope);

        formalParameters_OP_INDIRECTION.insert (make_pair (variableNameSuffix,
            opIndirectionDeclaration));

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

  SgVarRefExp * threadSynchRetReference = buildVarRefExp (
      localVariables_Others[variableName_threadSynchRet]);

  SgFunctionCallExp * threadSynchRetFunctionCall = buildFunctionCallExp (
      cudaThreadSynchronizeFunctionSymbol, buildExprListExp ());

  SgStatement * threadSynchFunctionCall = buildExprStatement (buildAssignOp (
      threadSynchRetReference, threadSynchRetFunctionCall));

  return threadSynchFunctionCall;
}



void
HostSubroutine::createReductionVariables ( ParallelLoop & parallelLoop )
{
	using SageBuilder::buildIntVal;
	using SageBuilder::buildIntType;
	using SageBuilder::buildAssignInitializer;
	using SageBuilder::buildVariableDeclaration;
	using SageInterface::appendStatement;
	
	/*
	 * ======================================================
	 * Check if we need the reduction support
	 * ======================================================
	 */
	if ( parallelLoop.isReductionRequired () == true ) {
		
		for (unsigned int i = 1; i
				 <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
		{
			if ( parallelLoop.isReductionRequiredForSpecificArgument ( i ) == true )
			{
				
				/*
				 * ======================================================
				 * Create CUDA variables for reduction if needed
				 * ======================================================
				 */
				reductionVariable_baseOffsetInSharedMemory = buildVariableDeclaration (
					"redStartOffset", FortranTypesBuilder::getFourByteInteger (),
					buildAssignInitializer (buildIntVal (0), buildIntType ()),
					subroutineScope );
				
				reductionVariable_maxBytesInSharedMemory = buildVariableDeclaration (
					"maxReductionSize", FortranTypesBuilder::getFourByteInteger (),
					buildAssignInitializer (buildIntVal (0), buildIntType ()),
					subroutineScope );
				
				reductionVariable_numberOfThreadItems = buildVariableDeclaration (
					"reductItems", FortranTypesBuilder::getFourByteInteger (),
					buildAssignInitializer (buildIntVal (0), buildIntType ()),
					subroutineScope );

				reductionVariable_maximumNumberOfThreadBlocks = buildVariableDeclaration (
					"maxBlocks", FortranTypesBuilder::getFourByteInteger (),
					buildAssignInitializer (buildIntVal (0), buildIntType ()),
					subroutineScope );
				
				reductionVariable_reductionArrayOnHost = buildVariableDeclaration (
					"redArrayHost", parallelLoop.get_OP_DAT_Type ( i ),
					NULL,
					//buildAssignInitializer (buildIntVal (0), buildIntType ()),
					subroutineScope ); 
				
				reductionVariable_reductionArrayOnHost->
				get_declarationModifier ().get_typeModifier ().setAllocatable ();
				
				reductionVariable_reductionArrayOnDevice = buildVariableDeclaration (
					"redArrayDevice", parallelLoop.get_OP_DAT_Type ( i ),
					NULL,
					//buildAssignInitializer (buildIntVal (0), buildIntType ()),
					subroutineScope );
				
				reductionVariable_reductionArrayOnDevice->
				get_declarationModifier ().get_typeModifier ().setAllocatable ();
				
				reductionVariable_reductionArrayOnDevice->
				get_declarationModifier ().get_typeModifier ().setDevice ();				
				
				reductionVariable_baseOffsetInSharedMemory->
					get_declarationModifier ().get_accessModifier ().setUndefined ();
				reductionVariable_maxBytesInSharedMemory->
					get_declarationModifier ().get_accessModifier ().setUndefined ();
				reductionVariable_numberOfThreadItems->
					get_declarationModifier ().get_accessModifier ().setUndefined ();
				reductionVariable_maximumNumberOfThreadBlocks->
					get_declarationModifier ().get_accessModifier ().setUndefined ();
				reductionVariable_reductionArrayOnHost->
					get_declarationModifier ().get_accessModifier ().setUndefined ();
				reductionVariable_reductionArrayOnDevice->
					get_declarationModifier ().get_accessModifier ().setUndefined ();
				
				
				appendStatement ( reductionVariable_baseOffsetInSharedMemory, subroutineScope );
				appendStatement ( reductionVariable_maxBytesInSharedMemory, subroutineScope );
				appendStatement ( reductionVariable_numberOfThreadItems, subroutineScope );
				appendStatement ( reductionVariable_maximumNumberOfThreadBlocks, subroutineScope );
				appendStatement ( reductionVariable_reductionArrayOnHost, subroutineScope );				
				appendStatement ( reductionVariable_reductionArrayOnDevice, subroutineScope );
				
			}
		}
	}
}

void
HostSubroutine::createAndAppendThreadSynchCall ( )
{
	using SageInterface::appendStatement;
	
	/*
	 * ======================================================
	 * threadSynchRet = cudaThreadSynchronize()
	 * ======================================================
	 */	 		

	SgStatement * threadSynchCallStmt = buildThreadSynchroniseFunctionCall ( subroutineScope );

	appendStatement ( threadSynchCallStmt, subroutineScope);
}

void
HostSubroutine::createSupportForReductionVariablesBeforeKernel ( ParallelLoop & parallelLoop )
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
	
	if ( parallelLoop.isReductionRequired () == false ) return;

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
		if ( parallelLoop.isReductionRequiredForSpecificArgument ( i ) == true )
		{
			int currentDim = parallelLoop.get_OP_DAT_Dimension ( i );
			if ( currentDim  > dim ) dim = currentDim;
			
			SgExpression * fortranKind = getFortranKindOfOpDat ( 
				parallelLoop.get_OP_DAT_Type ( i ) );
			
			SgIntVal * kindVal = isSgIntVal ( fortranKind );
			
			ROSE_ASSERT ( kindVal != NULL );
			
			int actualValue = -1;
			
			if ( maxUsedFortranKind < ( actualValue = kindVal->get_value() ) )
				maxUsedFortranKind = actualValue;
		}
	}
	
	SgVarRefExp * maxBlockVarRef = buildVarRefExp ( reductionVariable_maximumNumberOfThreadBlocks );
	
	SgExpression * initMaxBlocks = buildAssignOp ( maxBlockVarRef,
		buildVarRefExp ( CUDAVariable_blocksPerGrid ) );
	
	appendStatement ( buildExprStatement ( initMaxBlocks ) , subroutineScope );

	SgExpression * assignReductItems = buildAssignOp ( 
		buildVarRefExp ( reductionVariable_numberOfThreadItems ),
		buildMultiplyOp ( maxBlockVarRef,buildIntVal ( dim ) ) );
	
	appendStatement ( buildExprStatement ( assignReductItems ), subroutineScope );
	
	SgVarRefExp * reductItemsVarRef = buildVarRefExp ( reductionVariable_numberOfThreadItems );

	SgVarRefExp * redArrayHost = 
		buildVarRefExp ( reductionVariable_reductionArrayOnHost );
	
	SgPntrArrRefExp * allocateHostRedVarParams = buildPntrArrRefExp (
		redArrayHost, reductItemsVarRef );
	
	FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
		buildExprListExp ( allocateHostRedVarParams ), subroutineScope );

	SgVarRefExp * redArrayDev = 
		buildVarRefExp ( reductionVariable_reductionArrayOnDevice );

	SgPntrArrRefExp * allocateDeviceRedVarParams = buildPntrArrRefExp (
		redArrayDev, 
		reductItemsVarRef );
	
	FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
		buildExprListExp ( allocateDeviceRedVarParams ), subroutineScope );

	SgVarRefExp * itVar1VarRef = 
	  buildVarRefExp ( localVariables_Others[reductionSubroutineNames::iterationVarForReductionName1] );
	
	SgExpression * initLoop = buildAssignOp ( itVar1VarRef,	buildIntVal ( 0 ) );
	
	/*
	 * ======================================================
	 * Warning: for now, only real values are supported!
	 * ======================================================
	 */
	
	SgExpression * setToZeroRedArrayHost = buildAssignOp ( 
	  buildPntrArrRefExp ( redArrayHost, itVar1VarRef ), 
		buildFloatVal ( 0.0 ) );
	
	SgBasicBlock * initLoopBody = buildBasicBlock ( 
	  buildExprStatement ( setToZeroRedArrayHost ) );
	
	SgFortranDo * initLoopStatement =
	FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
		initLoop, reductItemsVarRef, buildIntVal (1), initLoopBody );
	
	appendStatement ( initLoopStatement , subroutineScope );

	SgExpression * copyHostToDeviceArray = buildAssignOp ( redArrayDev, redArrayHost );
	
	appendStatement ( buildExprStatement ( copyHostToDeviceArray ), subroutineScope );
	
	SgVarRefExp * nSharedVarRef = buildVarRefExp ( CUDAVariable_sharedMemorySize );
	
	SgExpression * assignReductionOffset = buildAssignOp ( 
	 buildVarRefExp ( reductionVariable_baseOffsetInSharedMemory ),
	 nSharedVarRef );
	
	appendStatement ( buildExprStatement ( assignReductionOffset ), subroutineScope );

	SgVarRefExp * maxRedSizeVarRef = buildVarRefExp ( reductionVariable_maxBytesInSharedMemory );
	SgExpression * initMaxShared = buildAssignOp (
	  maxRedSizeVarRef, buildMultiplyOp ( buildVarRefExp ( CUDAVariable_threadsPerBlock ),
		  buildIntVal ( maxUsedFortranKind ) ) );	
	
	appendStatement ( buildExprStatement ( initMaxShared ), subroutineScope );

	SgExpression * recomputeNshared = buildAssignOp ( 
		nSharedVarRef, buildAddOp ( nSharedVarRef, maxRedSizeVarRef ) );
	
	appendStatement ( buildExprStatement ( recomputeNshared ), subroutineScope );
	
}

void
HostSubroutine::createSupportForReductionVariablesAfterKernel ( ParallelLoop & parallelLoop )
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
	
	if ( parallelLoop.isReductionRequired () == false ) return;
		
	SgExpression * redArrayHostVarRef = buildVarRefExp ( reductionVariable_reductionArrayOnHost );
	
	SgExpression * copyDeviceTohostArray = buildAssignOp ( redArrayHostVarRef,
		buildVarRefExp ( reductionVariable_reductionArrayOnDevice ) );
		
	appendStatement ( buildExprStatement ( copyDeviceTohostArray ), subroutineScope );
	
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
	
	for ( unsigned int i = 1; i
			 <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
		if ( parallelLoop.isReductionRequiredForSpecificArgument ( i ) == true ) {
			dim = parallelLoop.get_OP_DAT_Dimension ( i );
			positionInOPDatsArray = i;
		}
	
	SgVarRefExp * itVar2VarRef = 
	  buildVarRefExp ( localVariables_Others[reductionSubroutineNames::iterationVarForReductionName2] );

	
	SgExpression * initInnerLoop = buildAssignOp ( 
	  itVar2VarRef, buildIntVal ( 0 ) );

	SgExpression * c2fPtrAccess = buildPntrArrRefExp (
	  buildVarRefExp ( localVariables_CToFortranPointers[positionInOPDatsArray] ),
			buildAddOp ( itVar2VarRef , buildIntVal ( 1 ) ) );

	SgVarRefExp * itVar1VarRef = 
	  buildVarRefExp ( localVariables_Others[reductionSubroutineNames::iterationVarForReductionName1] );

	SgExpression * assignExpInHostReduction = buildAddOp ( 
	  c2fPtrAccess,
		buildPntrArrRefExp ( redArrayHostVarRef,
		  buildAddOp ( itVar2VarRef, buildMultiplyOp ( itVar1VarRef,
			buildIntVal ( dim ) ) ) ) );


	SgExpression * finalReduceOnHostExpr = buildAssignOp ( c2fPtrAccess,
		assignExpInHostReduction );

	SgBasicBlock * innerLoopBody = buildBasicBlock ( 
	  buildExprStatement ( finalReduceOnHostExpr ) );

	SgFortranDo * innerRedCopyLoop =
	FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
		initInnerLoop, buildIntVal ( dim - 1 ), buildIntVal (1), innerLoopBody );
	
	SgBasicBlock * outerLoopBody = buildBasicBlock ( innerRedCopyLoop );

	SgExpression * initOuterLoop = buildAssignOp ( itVar1VarRef, buildIntVal ( 0 ) );
	
	SgFortranDo * outerRedCopyLoop =
	  FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
		  initOuterLoop, buildVarRefExp ( reductionVariable_numberOfThreadItems ),
			buildIntVal (1), outerLoopBody );
	
	
	appendStatement ( outerRedCopyLoop, subroutineScope );	
		
}


void
HostSubroutine::createAndAppendIterationVariablesForReduction ( ParallelLoop & parallelLoop )
{
	using SageBuilder::buildIntVal;
	using SageBuilder::buildIntType;
	using SageBuilder::buildAssignInitializer;
	using SageBuilder::buildVariableDeclaration;
	using SageInterface::appendStatement;
	using std::string;
	
	if ( parallelLoop.isReductionRequired () == true ) {

		string const iterVariableName1 = reductionSubroutineNames::iterationVarForReductionName1;
		string const iterVariableName2 = reductionSubroutineNames::iterationVarForReductionName2;

		localVariables_Others[iterVariableName1] = buildVariableDeclaration (
		iterVariableName1, FortranTypesBuilder::getFourByteInteger (),
		buildAssignInitializer (buildIntVal (0), buildIntType ()),
		subroutineScope);

		localVariables_Others[iterVariableName2] = buildVariableDeclaration (
			iterVariableName2, FortranTypesBuilder::getFourByteInteger (),
			buildAssignInitializer (buildIntVal (0), buildIntType ()),
			subroutineScope);
		
		localVariables_Others[iterVariableName1]->
			get_declarationModifier ().get_accessModifier ().setUndefined ();
		localVariables_Others[iterVariableName2]->
			get_declarationModifier ().get_accessModifier ().setUndefined ();
		
		appendStatement ( localVariables_Others[iterVariableName1], subroutineScope );
		appendStatement ( localVariables_Others[iterVariableName2], subroutineScope );
	}
}

HostSubroutine::HostSubroutine (std::string const & subroutineName,
    UserDeviceSubroutine & userDeviceSubroutine, ParallelLoop & parallelLoop,
    SgScopeStatement * moduleScope) :
  Subroutine (subroutineName + "_host")
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


