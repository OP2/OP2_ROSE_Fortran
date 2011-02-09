#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <stdio.h>

#include "Debug.h"
#include "OpParLoop.h"
/*
 * ====================================================================================================
 * Constants with internal linkage
 * ====================================================================================================
 */

namespace
{
  const std::string OP_PAR_LOOP_PREFIX = "op_par_loop";
  const std::string FORTRAN_FILE_SUFFIX = ".f95";
}

/*
 * ====================================================================================================
 * Private functions
 * ====================================================================================================
 */

void
OpParLoop::setSourcePosition (SgLocatedNode* locatedNode)
{
  // This function sets the source position to be marked as not available (since we often don't have token information)
  // These nodes WILL be unparsed in the code generation phase.

  // The SgLocatedNode has both a startOfConstruct and endOfConstruct source position.
  ROSE_ASSERT (locatedNode != NULL);

  // Make sure we never try to reset the source position of the global scope (set elsewhere in ROSE).
  ROSE_ASSERT (isSgGlobal (locatedNode) == NULL);

  // Check the endOfConstruct first since it is most likely NULL (helpful in debugging)
  ROSE_ASSERT (locatedNode->get_endOfConstruct () == NULL);
  ROSE_ASSERT (locatedNode->get_startOfConstruct () == NULL);

  // Call a mechanism defined in the SageInterface support
  SageInterface::setSourcePosition (locatedNode);
}

void
OpParLoop::setSourcePositionCompilerGenerated (SgLocatedNode* locatedNode)
{
  // This function sets the source position to be marked as compiler generated.

  // The SgLocatedNode has both a startOfConstruct and endOfConstruct source position.
  ROSE_ASSERT (locatedNode != NULL);

  // Check the endOfConstruct first since it is most likely NULL (helpful in debugging)
  ROSE_ASSERT (locatedNode->get_endOfConstruct () != NULL);
  ROSE_ASSERT (locatedNode->get_startOfConstruct () != NULL);

  locatedNode->get_startOfConstruct ()->setCompilerGenerated ();
  locatedNode->get_endOfConstruct ()->setCompilerGenerated ();
}

SgContainsStatement *
OpParLoop::buildContainsStatement (Sg_File_Info * fileInfo,
    SgScopeStatement * scope)
{
  SgContainsStatement * contains = new SgContainsStatement (fileInfo);
  contains->set_parent (scope);
  contains->set_definingDeclaration (contains);

  return contains;
}

void
OpParLoop::addUseStatements (SgSourceFile& sourceFile,
    SgDeclarationStatement* definingDeclaration, SgScopeStatement* scope)
{
  using namespace SageInterface;
  using namespace std;

  const string isoCBindingLib = "ISO_C_BINDING";
  const string cudaLib = "cudafor";

  SgUseStatement* useStatement1 = new SgUseStatement (
      sourceFile.get_file_info (), isoCBindingLib, false);
  useStatement1->set_definingDeclaration (definingDeclaration);
  appendStatement (useStatement1, scope);

  SgUseStatement* useStatement2 = new SgUseStatement (
      sourceFile.get_file_info (), cudaLib, false);
  useStatement2->set_definingDeclaration (definingDeclaration);
  appendStatement (useStatement2, scope);
}

void
OpParLoop::addImplicitStatement (SgSourceFile& sourceFile,
    SgDeclarationStatement* definingDeclaration, SgScopeStatement* scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

  SgImplicitStatement* implicitStatement = new SgImplicitStatement (
      sourceFile.get_file_info (), true);
  implicitStatement->set_definingDeclaration (definingDeclaration);
  appendStatement (implicitStatement, scope);

  SgAttributeSpecificationStatement* s = new SgAttributeSpecificationStatement (
      sourceFile.get_file_info ());
  s->set_attribute_kind (
      SgAttributeSpecificationStatement::e_intrinsicStatement);
  s->set_definingDeclaration (definingDeclaration);
  appendStatement (s, scope);
}

void
OpParLoop::createHostDeviceLocals (SgScopeStatement* scope)
{
  using SageBuilder::buildClassDeclaration;
  using SageBuilder::buildIntType;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::string;

  SgVariableDeclaration* CreturnDeclaration = buildVariableDeclaration ("CRet",
      buildIntType (), NULL, scope);
	CreturnDeclaration->get_declarationModifier ().get_accessModifier().setUndefined();
	
	
  appendStatement (CreturnDeclaration, scope);

  for (int i = 0; i < numberOfArgumentGroups; ++i)
  {
    string name = "data" + boost::lexical_cast <string> (i) + "Size";
    SgVariableDeclaration* dataSizeDeclaration = buildVariableDeclaration (
        name, buildIntType (), NULL, scope);
		dataSizeDeclaration->get_declarationModifier ().get_accessModifier().setUndefined();
		
    appendStatement (dataSizeDeclaration, scope);
  }
}

void
OpParLoop::createIndirectionDeclaration (SgFunctionParameterList* parameters,
    SgScopeStatement* scope)
{
  using SageBuilder::buildIntType;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::string;

  /*
   * This is the prefix for a data type representing indirection as
   * used by Mike Giles in his documentation
   */
  const string namePrefix = "idx";

  /*
   * Static so that the number of declarations of this type is
   * remembered
   */
  static int nameSuffix = 0;

  /*
   * The variable name
   */
  const string name = namePrefix + boost::lexical_cast <string> (nameSuffix);

  /*
   * Increase the suffix in readiness for the next declaration
   */
  nameSuffix++;

  /*
   * Create the declaration. OP_ACCESS has type integer
   */
  SgVariableDeclaration* indirectionDeclaration = buildVariableDeclaration (
      name, buildIntType (), NULL, scope);

  /*
   * Append the variable to the formal parameter list
   */
  parameters->append_arg (*(indirectionDeclaration->get_variables ().begin ()));

  /*
   * Set the attributes of the formal parameter
   */
  indirectionDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();

	indirectionDeclaration->get_declarationModifier ().get_accessModifier().setUndefined();
	
  appendStatement (indirectionDeclaration, scope);
}

void
OpParLoop::createOpAccessDeclaration (SgFunctionParameterList* parameters,
    SgScopeStatement* scope)
{
  using SageBuilder::buildIntType;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::string;

  /*
   * This is the prefix for OP_ACCESS data types used by Mike Giles
   * in his documentation
   */
  const string namePrefix = "access";

  /*
   * Static so that the number of declarations of this type is
   * remembered
   */
  static int nameSuffix = 0;

  /*
   * The variable name
   */
  const string name = namePrefix + boost::lexical_cast <string> (nameSuffix);

  /*
   * Increase the suffix in readiness for the next declaration
   */
  nameSuffix++;

  /*
   * Create the declaration. OP_ACCESS has type integer
   */
  SgVariableDeclaration* opAccessDeclaration = buildVariableDeclaration (name,
      buildIntType (), NULL, scope);

  /*
   * Append the variable to the formal parameter list
   */
  parameters->append_arg (*(opAccessDeclaration->get_variables ().begin ()));

  /*
   * Set the attributes of the formal parameter
   */
  opAccessDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();

	opAccessDeclaration->get_declarationModifier ().get_accessModifier().setUndefined();

	
  appendStatement (opAccessDeclaration, scope);
}

void
OpParLoop::createOpDatDeclaration (SgFunctionParameterList* parameters,
    SgScopeStatement* scope, SgType* opDatType)
{
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::string;

  /*
   * This is the prefix for OP_DAT data types used by Mike Giles
   * in his documentation
   */
  const string namePrefix = "arg";

  /*
   * Static so that the number of declarations of this type is
   * remembered
   */
  static int nameSuffix = 0;

  /*
   * The variable name
   */
  const string name = namePrefix + boost::lexical_cast <string> (nameSuffix);

  /*
   * Increase the suffix in readiness for the next declaration
   */
  nameSuffix++;

  /*
   * Create the declaration
   */
  SgVariableDeclaration* opDatDeclaration = buildVariableDeclaration (name,
      opDatType, NULL, scope);

  /*
   * Append the variable to the formal parameter list
   */
  parameters->append_arg (*(opDatDeclaration->get_variables ().begin ()));

	
	opDatDeclaration->get_declarationModifier ().get_accessModifier().setUndefined();

	
  appendStatement (opDatDeclaration, scope);
}

void
OpParLoop::createOpMapDeclaration (SgFunctionParameterList* parameters,
    SgScopeStatement* scope, SgType* opMapType)
{
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::string;

  /*
   * This is the prefix for OP_MAP data types used by Mike Giles
   * in his documentation
   */
  const string namePrefix = "ptr";

  /*
   * Static so that the number of declarations of this type is
   * remembered
   */
  static int nameSuffix = 0;

  /*
   * The variable name
   */
  const string name = namePrefix + boost::lexical_cast <string> (nameSuffix);

  /*
   * Increase the suffix in readiness for the next declaration
   */
  nameSuffix++;

  /*
   * Create the declaration
   */
  SgVariableDeclaration* opMapDeclaration = buildVariableDeclaration (name,
      opMapType, NULL, scope);

  /*
   * Append the variable to the formal parameter list
   */
  parameters->append_arg (*(opMapDeclaration->get_variables ().begin ()));

	opMapDeclaration->get_declarationModifier ().get_accessModifier().setUndefined();
	
  appendStatement (opMapDeclaration, scope);
}

void
OpParLoop::createOpSetDeclaration (SgFunctionParameterList* parameters,
    SgScopeStatement* scope, SgType* opSetType)
{
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::string;

  const string name = "cells";

  /*
   * Create the declaration
   */
  SgVariableDeclaration* opSetDeclaration = buildVariableDeclaration (name,
      opSetType, NULL, scope);

  /*
   * Append the variable to the formal parameter list
   */
  parameters->append_arg (*(opSetDeclaration->get_variables ().begin ()));

  /*
   * Set the attributes of the formal parameter
   */
  opSetDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();

	opSetDeclaration->get_declarationModifier ().get_accessModifier().setUndefined();

	
  appendStatement (opSetDeclaration, scope);
}

void
OpParLoop::createSubroutineName (SgFunctionParameterList* parameters,
    SgScopeStatement* scope)
{
  using SageBuilder::buildArrayType;
  using SageBuilder::buildCharType;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::string;

  /*
   * The name of the subroutine
   */
  const string name = "subroutineName";

  /*
   * Build asterisk shape expression indicating that the size of the
   * array is decided at run time
   */
  SgAsteriskShapeExp* asterisk = new SgAsteriskShapeExp ();
  Sg_File_Info* asteriskFileInfo = new Sg_File_Info ();
  asterisk->set_startOfConstruct (asteriskFileInfo);
  asterisk->set_endOfConstruct (asteriskFileInfo);

  /*
   * Build the type, which is a character array
   */
  SgArrayType* charArray = buildArrayType (buildCharType (), asterisk);

  /*
   * The dimension of the array is the asterisk
   */
  SgExprListExp* dimensionExprList = new SgExprListExp (asteriskFileInfo);
  dimensionExprList->append_expression (asterisk);
  charArray->set_dim_info (dimensionExprList);

  /*
   * The character array only has one dimension
   */
  charArray->set_rank (1);

  /*
   * Create the declaration
   */
  SgVariableDeclaration* charArrayDeclaration = buildVariableDeclaration (name,
      charArray, NULL, scope);

  /*
   * Append the variable to the formal parameter list
   */
  parameters->append_arg ( *(charArrayDeclaration->get_variables ().begin ()) );

  /*
   * Set the attributes of the formal parameter
   */
  charArrayDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();

	charArrayDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
	
	
  appendStatement (charArrayDeclaration, scope);
}

void
OpParLoop::createHostSubroutine (std::string kernelName,
    SgExpressionPtrList& args, SgSourceFile& sourceFile,
    SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

  const string OP_SET_NAME = "op_set";
  const string OP_DAT_NAME = "op_dat";
  const string OP_MAP_NAME = "op_map";

  /*
   * Create host subroutine parameters for 'op_par_loop_2'
   */
  SgFunctionParameterList* hostParameters = buildFunctionParameterList ();

  /*
   * Create host subroutine and append it to statements inside module
   */
  SgProcedureHeaderStatement* subroutineStatement =
      buildProcedureHeaderStatement (kernelName.c_str (), buildVoidType (),
																		 hostParameters,
																		 SgProcedureHeaderStatement::e_subroutine_subprogram_kind,
																		 scope
																		);
	
	appendStatement ( subroutineStatement, scope );
//  statements.push_back (subroutineStatement);
//  scopes.push_back (moduleStatement.get_definition ()->get_scope ());

  SgBasicBlock* subroutineScope =
      subroutineStatement->get_definition ()->get_body ();

  for (vector <SgExpression*>::iterator it = args.begin (); it != args.end (); ++it)
  {
    SgExpression* argument = (*it);
    Debug::getInstance ()->debugMessage ("Argument type: "
        + argument->class_name (), 8);

    switch (argument->variantT ())
    {
      case V_SgFunctionRefExp:
      {
        createSubroutineName (hostParameters, subroutineScope);
        break;
      }

      case V_SgVarRefExp:
      {
        SgVarRefExp* variableReference = isSgVarRefExp (argument);
        Debug::getInstance ()->debugMessage (
            "Variable reference expression type: "
                + variableReference->get_type ()->class_name (), 9);

        switch (variableReference->get_type ()->variantT ())
        {
          case V_SgClassType:
          {
            SgClassType* classReference = isSgClassType (
                variableReference->get_type ());
            const string className = classReference->get_name ().getString ();

            Debug::getInstance ()->debugMessage ("Class reference to: "
                + className, 10);

            if (strcmp (className.c_str (), OP_SET_NAME.c_str ()) == 0)
            {
              createOpSetDeclaration (hostParameters, subroutineScope,
                  classReference);
            }
            else if (strcmp (className.c_str (), OP_MAP_NAME.c_str ()) == 0)
            {
              createOpMapDeclaration (hostParameters, subroutineScope,
                  classReference);
            }
            else if (strcmp (className.c_str (), OP_DAT_NAME.c_str ()) == 0)
            {
              createOpDatDeclaration (hostParameters, subroutineScope,
                  classReference);
            }
            else
            {
              Debug::getInstance ()->debugMessage ("Unrecognised class: "
                  + className, 1);
              exit (1);
            }

            break;
          }

          case V_SgTypeInt:
          {
            createOpAccessDeclaration (hostParameters, subroutineScope);
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
      {
        createIndirectionDeclaration (hostParameters, subroutineScope);
        break;
      }

      default:
      {
        break;
      }
    }
  }

  createHostDeviceLocals (subroutineScope);
}




/*
 * Build a module
 */
SgModuleStatement *
OpParLoop::buildClassDeclarationAndDefinition (std::string name, SgScopeStatement* scope)
{
	using namespace SageBuilder;
  using namespace SageInterface;
	
  // This function builds a class declaration and definition 
  // (both the defining and nondefining declarations as required).
	
  // Build a file info object marked as a transformation
	Sg_File_Info* fileInfo = Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode();
	//	fileInfo->setOutputFileGeneration();
	fileInfo->setOutputInCodeGeneration();
	
	assert(fileInfo != NULL);
	
  // This is the class definition (the fileInfo is the position of the opening brace)
	SgClassDefinition* classDefinition   = new SgClassDefinition(fileInfo);
	assert(classDefinition != NULL);
	
	
	/*	SgModuleStatement::SgModuleStatement 	( 	Sg_File_Info *  	startOfConstruct,
	 SgName  	name = "",
	 SgClassDeclaration::class_types  	class_type = SgClassDeclaration::e_class,
	 SgClassType *  	type = NULL,
	 SgClassDefinition *  	definition = NULL	 
	 ) 	
	 */
  // Set the end of construct explictly (where not a transformation this is the location of the closing brace)
	classDefinition->set_endOfConstruct(fileInfo);
	
  // This is the defining declaration for the class (with a reference to the class definition)
	SgModuleStatement * classDeclaration = new SgModuleStatement(fileInfo,name.c_str(),SgClassDeclaration::e_struct,NULL,classDefinition);
	assert(classDeclaration != NULL);
	
  // Set the defining declaration in the defining declaration!
	classDeclaration->set_definingDeclaration(classDeclaration);
	
  // Set the non defining declaration in the defining declaration (both are required)
	SgClassDeclaration* nondefiningClassDeclaration = new SgClassDeclaration(fileInfo,name.c_str(),SgClassDeclaration::e_struct,NULL,NULL);
	assert(classDeclaration != NULL);
	nondefiningClassDeclaration->set_type(SgClassType::createType(nondefiningClassDeclaration));
	
  // Set the internal reference to the non-defining declaration
	classDeclaration->set_firstNondefiningDeclaration(nondefiningClassDeclaration);
	classDeclaration->set_type(nondefiningClassDeclaration->get_type());
	
  // Set the defining and no-defining declarations in the non-defining class declaration!
	nondefiningClassDeclaration->set_firstNondefiningDeclaration(nondefiningClassDeclaration);
	nondefiningClassDeclaration->set_definingDeclaration(classDeclaration);
	
  // Set the nondefining declaration as a forward declaration!
	nondefiningClassDeclaration->setForward();
	
  // Don't forget the set the declaration in the definition (IR node constructors are side-effect free!)!
	classDefinition->set_declaration(classDeclaration);
	
  // set the scope explicitly (name qualification tricks can imply it is not always the parent IR node!)
	classDeclaration->set_scope(scope);
	nondefiningClassDeclaration->set_scope(scope);
	
  // some error checking
	assert(classDeclaration->get_definingDeclaration() != NULL);
	assert(classDeclaration->get_firstNondefiningDeclaration() != NULL);
	assert(classDeclaration->get_definition() != NULL);
	
  // DQ (9/8/2007): Need to add function symbol to global scope!
	printf ("Fixing up the symbol table in scope = %p = %s for class = %p = %s \n",scope,scope->class_name().c_str(),classDeclaration,classDeclaration->get_name().str());
	SgClassSymbol* classSymbol = new SgClassSymbol(classDeclaration);
	scope->insert_symbol(classDeclaration->get_name(),classSymbol);
	ROSE_ASSERT(scope->lookup_class_symbol(classDeclaration->get_name()) != NULL);
	
	return classDeclaration;
}



SgModuleStatement *
OpParLoop::createModule (std::string kernelName, SgSourceFile& sourceFile)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

	SgGlobal * globalScope = sourceFile.get_globalScope();
		
  /*
   * Create module name
   */
  const string moduleName = OP_PAR_LOOP_PREFIX + "_" + kernelName + "_module";

  /*
   * Create the module statement
   */
	SgModuleStatement * moduleStatement = buildClassDeclarationAndDefinition ( moduleName, globalScope );
	
	
	moduleStatement->get_definition()->setCaseInsensitive ( true );
	
	appendStatement ( moduleStatement, globalScope );
	
	
  return moduleStatement;
}



SgProcedureHeaderStatement *
OpParLoop::generateCUDAUserKernel ( SgScopeStatement * scope )
{
	using namespace SageBuilder;
	using namespace SageInterface;
	using namespace std;
	
	// I need to find the definition inside the correct file (otherwise the definition would be empty)
	vector < SgProcedureHeaderStatement * >::iterator it = inputSubroutines.begin();
	
	while ( (*it)->get_name().getString() != (userKernelFunction->get_name()).getString() ) {
		it++;
		cout << "comparing: " << ((*it)->get_name()).getString() << " and " << (userKernelFunction->get_name()).getString() << endl;
	}
	
	SgProcedureHeaderStatement *subroutine;
	
	if ( (*it)->get_name().getString() == (userKernelFunction->get_name()).getString() ) {
		cout << "OK, found!!" << endl;
		
		SgProcedureHeaderStatement * realRoutine = (*it);
		
		//		subroutine = (SgProcedureHeaderStatement *) realRoutine->copy ( *(new SgTreeCopy) );
		
		subroutine = buildProcedureHeaderStatement ( (realRoutine->get_name()).str(),
																								buildVoidType (),
																								realRoutine->get_parameterList(),
																								SgProcedureHeaderStatement::e_subroutine_subprogram_kind,
																								scope );
		
		/* 
		 * now visiting the subtree to: 
		 * 1. append the statement to the statements to be unparse (visiting the statement list)
		 *		(TODO: may not work for nested bodies: the alternative is to use a copy routine that currently does
		 *		 not work)
		 * 2. set its OutputInCodeGeneration flag to true (using a special visitor)
		 */
		
		Rose_STL_Container < SgStatement * > toFill =	subroutine->get_definition()->get_body()->get_statements();
		
		Rose_STL_Container < SgStatement * > filling = realRoutine->get_definition()->get_body()->get_statements();
		
		Rose_STL_Container < SgStatement * >::iterator copyIt;
		
		for ( copyIt = filling.begin(); copyIt != filling.end(); copyIt++ ) {
			
			// adding the statement to the statement list
			toFill.push_back ( *copyIt );
			
			// appending statement too
			appendStatement ( *copyIt, subroutine->get_definition()->get_body() );	
			
		}
		
		deepVisitAndSetGeneration * newVisit = new deepVisitAndSetGeneration ();
		newVisit->traverse ( (SgNode *) subroutine, preorder );
		
		// now modifying the formal parameter declaration adding the "device" attribute
		SgFunctionParameterList * parameterList = subroutine->get_parameterList();
//		(SgFunctionParameterList * ) deepCopy ( subroutine->get_parameterList() );
		
		
		
		// scan the parameter list and the statements: when a same name is found, we add the device attribute
		// typedef in rose is   Rose_STL_Container<SgInitializedName*> --> SgInitializedNamePtrList 
		// TODO: this is somehow too general, if we consider that all fortran formal parameters are 
		// declared at the beginning of the declaration section and their number is known (!)
		SgInitializedNamePtrList list = parameterList->get_args();
		
		SgInitializedNamePtrList::iterator varMod;
		
		for ( varMod = list.begin(); varMod != list.end(); varMod++ ) {
			
			// checking every statement inside the subroutine
			copyIt = toFill.begin();
			while ( copyIt != toFill.end() ) {
				// check is copyIt is pointing to a SgVariableDeclaration
				SgVariableDeclaration * isVar;
				if ( (isVar = isSgVariableDeclaration ( *copyIt ) ) != NULL ) {
					// check if the name corresponds to the one we are looking for
					if ( isVar->get_definition()->get_vardefn()->get_name().getString() == 
							(*varMod)->get_name().getString() ) {
						// found the variable: adding the DEVICE attribute
						(*varMod)->get_declaration()->get_declarationModifier().get_typeModifier().setDevice();
						// if I have found the variable, I can stop searching
						break;
					}
				}
				copyIt++;
			}
		}
	}
	
	
	appendStatement ( subroutine, scope );

	
	return subroutine;
	
}




/*
 * Creates a CUDA source file
 */
SgSourceFile*
OpParLoop::createSourceFile (std::string kernelName)
{
  using namespace SageBuilder;
  using namespace std;

  /*
   * To create a new file (to which the AST is later unparsed), the API expects
   * the name of an existing file and the name of the output file. There is no
   * input file corresponding to our output file, therefore we first create a
   * dummy Fortran file
   */
  const string inputFileName = "BLANK" + FORTRAN_FILE_SUFFIX;

  FILE* inputFile = fopen (inputFileName.c_str (), "w+");
  if (inputFile != NULL)
  {
    fclose (inputFile);
  }
  else
  {
    Debug::getInstance ()->debugMessage (
        "Could not create dummy Fortran file '" + inputFileName + "'", 1);
    exit (1);
  }

  /*
   * Now generate the CUDA file
   */
  const string CUDA_FILE_NAME_SUFFIX = "_cuda_module.CUF";
  const string outputFileName = kernelName + CUDA_FILE_NAME_SUFFIX;

  Debug::getInstance ()->debugMessage ("Generating CUDA file '"
      + outputFileName + "'", 1);

  SgSourceFile* sourceFile = isSgSourceFile (buildFile (inputFileName,
      outputFileName, NULL));

  /*
   * Later unparse according to the Fortran 95 standard
   */
  sourceFile->set_F95_only (true);

  /*
   * No implicit symbols shall be allowed in the generated Fortran file
   */
  sourceFile->set_fortran_implicit_none (true);

	
	sourceFile->set_outputFormat ( SgFile::e_free_form_output_format );

	
  /*
   * Store the file so it can be unparsed after AST construction
   */
	kernelOutputFiles.push_back (sourceFile);	
	
  return sourceFile;
}


void
OpParLoop::createCudaModule ( std::string kernelName, SgSourceFile& sourceFile, SgExpressionPtrList& args )
{
	
	using namespace std;

	cout << "Creating CUDA module" << endl;
	
	SgModuleStatement * cudaModule = createModule (  kernelName, sourceFile);

	cout << "Creating contains statement" << endl;

	
	/*
   * Create the contains statement and append it to the module list of declarations
   */
  SgContainsStatement * containsStatement = buildContainsStatement ( sourceFile.get_file_info (), 
																																	   cudaModule->get_definition ()
																																	);
	

	cout << "Pushing bask contains statement" << endl;
	
	vector<SgDeclarationStatement *> & statementList = cudaModule->get_definition()->getDeclarationList();
	statementList.push_back ( containsStatement );
	
	
	cout << "Creating kernelCUDAVersionDecl" << endl;
	
	
	// generating and modifying user kernel and appending it to the contains scope
	SgProcedureHeaderStatement * kernelCUDAVersionDecl = generateCUDAUserKernel ( containsStatement->get_scope() );
	
/*	
	SgProcedureHeaderStatement * kernelMainRoutine = buildMainKernelRoutine ( subroutineName,
																																					 contains->get_scope(),
																																					 computeOpDatArgNumber ( args.size() ),
																																					 args );
*/
	

	cout << "Creating Host subroutine" << endl;
	
	createHostSubroutine (kernelName, args, sourceFile, containsStatement->get_scope() );
	
}


/*
 * ====================================================================================================
 * Public functions
 * ====================================================================================================
 */

OpParLoop::OpParLoop (SgProject *project)
{
  this->project = project;
}

/*
 * Over-riding implementation of the AST vertex traversal function
 */
void
OpParLoop::visit (SgNode* node)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

  switch (node->variantT ())
  {
			
		// I need to remember all subroutine definition because I need to recover actual kernel definitions later
		// to copy them in the new files
		case V_SgProcedureHeaderStatement:
		{

			inputSubroutines.push_back ( (SgProcedureHeaderStatement *) node );
			
//			Rose_STL_Container < SgStatement * > stuff = 
//					((SgProcedureHeaderStatement *) node)->get_definition()->get_body()->get_statements();
			
			break;
		}
	
    case V_SgFunctionCallExp:
    {
      /*
       * Function call found in the AST
       */
      SgFunctionCallExp* functionCallExp = isSgFunctionCallExp (node);
      ROSE_ASSERT (functionCallExp != NULL);

      const string
          calleeName =
              functionCallExp->getAssociatedFunctionDeclaration ()->get_name ().getString ();

      /*
       * The prefix of all parallel loop calls in Fortran contains 'op_par_loop'.
       * The suffix of the call, however, changes depending on the number of expected
       * parameters. Therefore, any match of this prefix indicates a call of interest
       * to the translator
       */
      if (boost::starts_with (calleeName, OP_PAR_LOOP_PREFIX))
      {
        /*
         * Retrieve the arguments of the call
         */
        SgExpressionPtrList& args =
            functionCallExp->get_args ()->get_expressions ();

        /*
         * Arguments to 'op_par_loop' are grouped into batches of 4
         * arguments, plus another two formal parameters.
         * Remember how many groups there are as this is needed to
         * generate the correct number of local variables in certain
         * host functions
         */
        numberOfArgumentGroups = (args.size () - 2) / 4;

        Debug::getInstance ()->debugMessage ("Found call to '" + calleeName
            + "' with " + boost::lexical_cast <string> (args.size ())
            + " arguments", 2);

				/*
				 * I need the kernel definition to copy it later in the implementation of the module
				 */
				SgNode *firstArg = args.front();
				this->setUserKernelDeclaration ( (SgProcedureHeaderStatement *) 
																				(isSgFunctionRefExp (firstArg))->getAssociatedFunctionDeclaration () );

				
        /*
         * An 'op_par_loop_2' always has 10 arguments
         */
        if (args.size () == 10)
        {
          /*
           * The first argument to an 'op_par_loop' call should be a reference to
           * the kernel function. Cast it and proceed, otherwise throw an exception
           */
          SgFunctionRefExp* functionRefExp = isSgFunctionRefExp (args.front ());
          try
          {
            if (functionRefExp != NULL)
            {
              const string
                  kernelName =
                      functionRefExp->getAssociatedFunctionDeclaration ()->get_name ().getString ();

              Debug::getInstance ()->debugMessage ("Found kernel '"
                  + kernelName + "'", 2);

              /*
               * Generate the CUDA file for this kernel
               */
              SgSourceFile * sourceFile = createSourceFile (kernelName);

							/*
							 * Generate and fill the CUDA module
							 */
							createCudaModule ( kernelName, *sourceFile, args );
							
            }
            else
            {
              throw args.front ();
            }
          }
          catch (SgNode *exceptionNode)
          {
            Debug::getInstance ()->debugMessage (
                "First argument to 'op_par_loop' is not a kernel. The argument has type '"
                    + exceptionNode->class_name () + "'", 1);
            exit (1);
          }
        }
      }
      break;
    }

    case V_SgSourceFile:
    {
      SgSourceFile* sourceFile = isSgSourceFile (node);
      ROSE_ASSERT (sourceFile != NULL);

      Debug::getInstance ()->debugMessage ("Found file "
          + sourceFile->getFileName (), 8);

      SgGlobal* globalFileScope = sourceFile->get_globalScope ();


      break;
    }

    default:
    {
      break;
    }
  }
}

void
OpParLoop::unparse ()
{
  Debug::getInstance ()->verboseMessage ("Generating CUDA files");

  for (std::vector <SgSourceFile*>::iterator it = kernelOutputFiles.begin (); it
      != kernelOutputFiles.end (); ++it)
  {
    Debug::getInstance ()->debugMessage ("Unparsing AST to "
        + (*it)->getFileName (), 1);

    /*
     * Get the source project associated with this file
     */
    SgProject* project = (*it)->get_project ();

    /*
     * Ensure the AST is consistent before unparsing
     */
    AstTests::runAllTests (project);

    /*
     * Generate the file
     */
    project->unparse ();
  }
}

void deepVisitAndSetGeneration::visit ( SgNode * nextNode )
{
	SgLocatedNode * castedNode;
	if ( (castedNode = isSgLocatedNode ( nextNode )) != NULL )
		castedNode->setOutputInCodeGeneration();
}
