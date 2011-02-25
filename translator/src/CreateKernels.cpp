#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>

#include "OP2CommonDefinitions.h"
#include "Debug.h"
#include "CreateKernels.h"
#include "OpParLoop.h"

/*
 * ====================================================================================================
 * Private functions
 * ====================================================================================================
 */

SgContainsStatement *
CreateKernels::buildContainsStatement (Sg_File_Info * fileInfo,
    SgScopeStatement * scope)
{
  SgContainsStatement * contains = new SgContainsStatement (fileInfo);
  contains->set_parent (scope);
  contains->set_definingDeclaration (contains);

  return contains;
}

void
CreateKernels::addImplicitStatement (SgSourceFile& sourceFile,
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
CreateKernels::declareCUDAConfigurationParameters (SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

  // build: 	integer(4) :: bsize = BSIZE_DEFAULT
  //					integer(4) :: gsize
  //					integer(4) :: reduct_bytes = 0
  //					integer(4) :: reduct_size = 0
  //					real(8) :: reduct_shared
  //					integer(4) :: const_bytes = 0
  //					gsize = int ((set%size - 1) / bsize + 1)
  //					reduct_shared = reduct_size * (BSIZE_DEFAULT / 2)

  // build integer(4) type
  SgModifierType * intFour = buildFortranKindType (buildIntType (),
      buildIntVal (4));

  // build reference to opaque MACRO BSIZE_DEFAULT
  string const bsizeMacroName = "BSIZE_DEFAULT";
  SgVarRefExp * bsizeMacroRef = buildOpaqueVarRefExp (bsizeMacroName, scope);

  SgAssignInitializer * bsizeInit = buildAssignInitializer (bsizeMacroRef,
      buildIntType ());

  string const bsizeName = "bsize";
  SgVariableDeclaration * bSizeDecl = buildVariableDeclaration (bsizeName,
      intFour, bsizeInit, scope);

  bSizeDecl->get_declarationModifier ().get_accessModifier ().setUndefined ();

  declaredCUDAConfigurationParameters.push_back (bSizeDecl);
  appendStatement (bSizeDecl, scope);

  string const gsizeName = "gsize";
  SgVariableDeclaration * gSizeDecl = buildVariableDeclaration (gsizeName,
      intFour, NULL, scope);

  gSizeDecl->get_declarationModifier ().get_accessModifier ().setUndefined ();

  declaredCUDAConfigurationParameters.push_back (gSizeDecl);
  appendStatement (gSizeDecl, scope);

  string const redBytesName = "reduct_bytes";

  SgAssignInitializer * redBytesInit = buildAssignInitializer (buildIntVal (0),
      buildIntType ());

  SgVariableDeclaration * redBytesDecl = buildVariableDeclaration (
      redBytesName, intFour, redBytesInit, scope);

  redBytesDecl->get_declarationModifier ().get_accessModifier ().setUndefined ();

  declaredCUDAConfigurationParameters.push_back (redBytesDecl);
  appendStatement (redBytesDecl, scope);

  string const redSizeName = "reduct_size";

  SgAssignInitializer * redSizeInit = buildAssignInitializer (buildIntVal (0),
      buildIntType ());

  SgVariableDeclaration * redSizeDecl = buildVariableDeclaration (redSizeName,
      intFour, redSizeInit, scope);

  redSizeDecl->get_declarationModifier ().get_accessModifier ().setUndefined ();

  declaredCUDAConfigurationParameters.push_back (redSizeDecl);
  appendStatement (redSizeDecl, scope);

  SgModifierType * realEight = buildFortranKindType (buildFloatType (),
      buildIntVal (8));

  string const redSharedName = "reduct_shared";
  SgVariableDeclaration * redSharedDecl = buildVariableDeclaration (
      redSharedName, realEight, redSizeInit, scope);

  redSharedDecl->get_declarationModifier ().get_accessModifier ().setUndefined ();

  declaredCUDAConfigurationParameters.push_back (redSharedDecl);
  appendStatement (redSharedDecl, scope);

  string const constBytesName = "const_bytes";

  SgAssignInitializer * constBytesInit = buildAssignInitializer (
      buildIntVal (0), buildIntType ());

  SgVariableDeclaration * constBytesDecl = buildVariableDeclaration (
      constBytesName, intFour, constBytesInit, scope);

  constBytesDecl->get_declarationModifier ().get_accessModifier ().setUndefined ();

  declaredCUDAConfigurationParameters.push_back (constBytesDecl);
  appendStatement (constBytesDecl, scope);

  // initialise gsize:	gsize = int ((set%size - 1) / bsize + 1)

  // transform the expression to a variable reference
  SgExpression * iterationSetVarRef = buildVarRefExp (iterationSetFormalArg);

  // build size field reference and set%size expression
  SgVarRefExp * setSizeField = buildOpaqueVarRefExp ("size", scope);
  SgExpression * setSizeAccess = buildDotExp (iterationSetVarRef, setSizeField);

  // build: set%size - 1
  SgExpression * setSizeMinusOne = buildSubtractOp (setSizeAccess, buildIntVal (
      1));

  // build: (set%size - 1) / bsize
  SgVarRefExp * bSizeVarRef = buildVarRefExp (bSizeDecl);
  SgExpression * divisionExpression = buildDivideOp (setSizeMinusOne,
      bSizeVarRef);

  // build: (set%size - 1) / bsize  + 1
  SgExpression * intParam = buildAddOp (divisionExpression, buildIntVal (1));

  // build: int ((set%size - 1) / bsize + 1)
  SgExprListExp * intParams = buildExprListExp (intParam);
  string const intName = "int";

  // build function type for intrinsic int
  // Notice that we do not care about the fact that the function is already defined (also via an intrinsic):
  // we just tell rose all the information needed to build a new function
  SgFunctionParameterTypeList * intInputTypes = buildFunctionParameterTypeList (
      buildDoubleType ());
  SgFunctionType * intType = buildFunctionType (buildIntType (), intInputTypes);
  SgFunctionRefExp * intRef = buildFunctionRefExp (*(new SgName (intName)),
      intType, scope);
  // build function call
  SgFunctionSymbol * intIntrinsicSymbol = intRef->get_symbol_i ();
  SgFunctionCallExp * intIntrinsicCall = buildFunctionCallExp (
      intIntrinsicSymbol, intParams);

  // build: gsize = int ((set%size - 1) / bsize + 1)
  SgVarRefExp * gSizeVarRef = buildVarRefExp (gSizeDecl);
  SgExpression * gSizeAssign = buildAssignOp (gSizeVarRef, intIntrinsicCall);

  // expression -> statement
  SgExprStatement * gSizeAssignStmt = buildExprStatement (gSizeAssign);

  // appending the new statement
  appendStatement (gSizeAssignStmt, scope);

  // initialise reduct_shared:	reduct_shared = reduct_size * (BSIZE_DEFAULT / 2)

  // build: reduct_size * (BSIZE_DEFAULT / 2)
  SgExpression * bsizeDivTwo = buildDivideOp (bsizeMacroRef, buildIntVal (2));
  SgVarRefExp * redSizeRef = buildVarRefExp (redSizeDecl);
  SgExpression * redSizePerBsizeDivTwo = buildMultiplyOp (redSizeRef,
      bsizeDivTwo);

  // build: reduct_shared = reduct_size * (BSIZE_DEFAULT / 2)
  SgVarRefExp * redSharedRef = buildVarRefExp (redSharedDecl);
  SgExpression * redSharedAssign = buildAssignOp (redSharedRef,
      redSizePerBsizeDivTwo);

  // expression -> statement
  SgExprStatement * redSharedAssignStmt = buildExprStatement (redSharedAssign);

  // appending the new statement
  appendStatement (redSharedAssignStmt, scope);
}

void
CreateKernels::declareC2FortranVariables (SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

  // the number of variables that we are going to create are equal to
  // the number of op_par_loop arguments * 2 (because we have 1 C pointer
  // and 1 Fortran device variable for each argument op_dat)
  //	vector < SgVariableDeclaration * > * declaredC2FortranVariables =
  //		new vector < SgVariableDeclaration * > ( numberOfArgumentGroups*2 );


  // build:	real(8), dimension(:), allocatable, device :: argument0
  //				real(8), dimension(:), allocatable, device :: argumentN-1
  for (int i = 0; i < numberOfArgumentGroups; ++i)
  {
    string const name = "argument" + boost::lexical_cast <string> (i);

    // building array type with : shape specifier
    //		SgColonShapeExp * colonExp = new SgColonShapeExp ();
    //		Sg_File_Info* colonFileInfo = new Sg_File_Info ();
    //		colonExp->set_startOfConstruct (colonFileInfo);
    //		colonExp->set_endOfConstruct (colonFileInfo);

    // retrieve base type from the op_decl_dat call
    SgType * baseType = actualArgumentsTypes[i];

    // check if it is a kind=* variable
    //		SgArrayType * arrayType = isSgArrayType ( baseType );
    //		SgModifierType * modType = isSgModifierType ( arrayType->get_base_type() );
    //		if ( modType != NULL ) {
    //
    //			std::cout << "inside modtype" << std::endl;
    //			// change attributes here
    //			modType->get_typeModifier().setDevice();
    //			modType->get_typeModifier().setAllocatable();
    //
    //			baseType = modType;
    ////			exit ( 0 );
    //		} else {
    //			std::cout << "class is " << arrayType->get_base_type()->class_name() << std::endl;
    ////			exit ( 0 );
    //		}
    //
    //		SgArrayType * arrayColonType = buildArrayType ( baseType, colonExp );
    //
    //		/*
    //		 * The dimension of the array is the colon shape
    //		 */
    //		SgExprListExp* dimensionExprList = buildExprListExp ( colonExp );
    //		arrayColonType->set_dim_info ( dimensionExprList );
    //
    //		/*
    //		 * The fortrnArg array only has one dimension
    //		 */
    //		arrayColonType->set_rank (1);

    SgVariableDeclaration * fortranArgVar = buildVariableDeclaration (name,
        baseType, NULL, scope);

    //	if ( modType == NULL ) {
    // not a kind=* fortran variable: we have to set attributes on the variable declaration

    fortranArgVar->get_declarationModifier ().get_typeModifier ().setDevice ();
    fortranArgVar->get_declarationModifier ().get_typeModifier ().setAllocatable ();
    //	}

    fortranArgVar->get_declarationModifier ().get_accessModifier ().setUndefined ();

    declaredC2FortranVariables.push_back (fortranArgVar);
    appendStatement (fortranArgVar, scope);
  }

  // build:	real(8), dimension(:), pointer :: c2fPtr0
  //				real(8), dimension(:), pointer :: c2fPtrN-1
  for (int i = 0; i < numberOfArgumentGroups; ++i)
  {
    string const name = "c2fPtr" + boost::lexical_cast <string> (i);

    // building array type with : shape specifier
    SgColonShapeExp * colonExp = new SgColonShapeExp ();
    Sg_File_Info* colonFileInfo = new Sg_File_Info ();
    colonExp->set_startOfConstruct (colonFileInfo);
    colonExp->set_endOfConstruct (colonFileInfo);

    // retrieve base type from the op_decl_dat call
    SgType * baseType = actualArgumentsTypes[i];

    // build pointer to the base type
    SgPointerType * pointerToArrayColon = buildPointerType (baseType);

    // now we can build the variable
    SgVariableDeclaration * fortranArgVar = buildVariableDeclaration (name,
        pointerToArrayColon, NULL, scope);

    // eliminate "public" attribute (Rose bug)
    fortranArgVar->get_declarationModifier ().get_accessModifier ().setUndefined ();

    declaredC2FortranVariables.push_back (fortranArgVar);
    appendStatement (fortranArgVar, scope);
  }
}

void
CreateKernels::createHostDeviceLocals (SgScopeStatement* scope,
    SgExpressionPtrList& args)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

  // the number of local host variables are 1 (CReturn) + the number of op_dat parameters of
  // the op_par_loop, each corresponding to a dataISize local variable


  SgVariableDeclaration * CreturnDeclaration = buildVariableDeclaration (
      "CRet", buildIntType (), NULL, scope);
  CreturnDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

  declaredHostRoutineLocals.push_back (CreturnDeclaration);

  appendStatement (CreturnDeclaration, scope);

  for (int i = 0; i < numberOfArgumentGroups; ++i)
  {
    string const name = "data" + boost::lexical_cast <string> (i) + "Size";
    SgVariableDeclaration * dataSizeDeclaration = buildVariableDeclaration (
        name, buildIntType (), NULL, scope);
    dataSizeDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

    declaredHostRoutineLocals.push_back (dataSizeDeclaration);
    appendStatement (dataSizeDeclaration, scope);
  }

  // build: 	real(8), dimension(:), allocatable, device :: argument0
  //					real(8), dimension(:), allocatable, device :: argument1
  //					real(8), dimension(:), pointer :: c2fPtr0
  //					real(8), dimension(:), pointer :: c2fPtr1
  declareC2FortranVariables (scope);

  // build: 	integer(4) :: bsize = BSIZE_DEFAULT
  //					integer(4) :: gsize
  //					integer(4) :: reduct_bytes = 0
  //					integer(4) :: reduct_size = 0
  //					real(8) :: reduct_shared
  //					integer(4) :: const_bytes = 0
  //					gsize = int ((set%size - 1) / bsize + 1)
  //					reduct_shared = reduct_size * (BSIZE_DEFAULT / 2)
  declareCUDAConfigurationParameters (scope);
}

void
CreateKernels::createIndirectionDeclaration (
    SgFunctionParameterList* parameters, SgScopeStatement* scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

  /*
   * This is the prefix for a data type representing indirection as
   * used by Mike Giles in his documentation
   */
  string const namePrefix = "idx";

  /*
   * Static so that the number of declarations of this type is
   * remembered
   */
  static int nameSuffix = 0;

  /*
   * The variable name
   */
  string const name = namePrefix + boost::lexical_cast <string> (nameSuffix);

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

  indirectionDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

  indirectionFormalArgs.push_back (indirectionDeclaration);
  appendStatement (indirectionDeclaration, scope);
}

void
CreateKernels::createOpAccessDeclaration (SgFunctionParameterList* parameters,
    SgScopeStatement* scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

  /*
   * This is the prefix for OP_ACCESS data types used by Mike Giles
   * in his documentation
   */
  string const namePrefix = "access";

  /*
   * Static so that the number of declarations of this type is
   * remembered
   */
  static int nameSuffix = 0;

  /*
   * The variable name
   */
  string const name = namePrefix + boost::lexical_cast <string> (nameSuffix);

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

  opAccessDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

  accessFormalArgs.push_back (opAccessDeclaration);
  appendStatement (opAccessDeclaration, scope);
}

void
CreateKernels::createOpDatDeclaration (SgFunctionParameterList* parameters,
    SgScopeStatement* scope, SgType* opDatType)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

  /*
   * This is the prefix for OP_DAT data types used by Mike Giles
   * in his documentation
   */
  string const namePrefix = "arg";

  /*
   * Static so that the number of declarations of this type is
   * remembered
   */
  static int nameSuffix = 0;

  /*
   * The variable name
   */
  string const name = namePrefix + boost::lexical_cast <string> (nameSuffix);

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

  opDatDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

  opDatFormalArgs.push_back (opDatDeclaration);
  appendStatement (opDatDeclaration, scope);
}

void
CreateKernels::createOpMapDeclaration (SgFunctionParameterList* parameters,
    SgScopeStatement* scope, SgType* opMapType)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

  /*
   * This is the prefix for OP_MAP data types used by Mike Giles
   * in his documentation
   */
  string const namePrefix = "ptr";

  /*
   * Static so that the number of declarations of this type is
   * remembered
   */
  static int nameSuffix = 0;

  /*
   * The variable name
   */
  string const name = namePrefix + boost::lexical_cast <string> (nameSuffix);

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

  opMapDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

  mapFormalArgs.push_back (opMapDeclaration);
  appendStatement (opMapDeclaration, scope);
}

void
CreateKernels::createOpSetDeclaration (SgFunctionParameterList* parameters,
    SgScopeStatement* scope, SgType* opSetType)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

  string const name = "iterationSet";

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

  opSetDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

  iterationSetFormalArg = opSetDeclaration;
  appendStatement (opSetDeclaration, scope);
}

void
CreateKernels::createSubroutineName (SgFunctionParameterList* parameters,
    SgScopeStatement* scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

  /*
   * The name of the subroutine
   */
  string const name = "subroutineName";

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
  SgExprListExp* dimensionExprList = buildExprListExp (asterisk);
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
  parameters->append_arg (*(charArrayDeclaration->get_variables ().begin ()));

  /*
   * Set the attributes of the formal parameter
   */
  charArrayDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();

  charArrayDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (charArrayDeclaration, scope);
}

void
CreateKernels::createMainRoutineStatements (SgScopeStatement * scope,
    SgExpressionPtrList& args)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

  // build:		data0Size = ( arg0%dim * (arg0%set)%size)
  //					data1Size = ( arg1%dim * (arg1%set)%size)

  // iterator over op_dat formal parameters
  vector <SgVariableDeclaration *>::iterator argIt = opDatFormalArgs.begin ();

  // iterator over local variables
  // the first local variable declaration is CRet, then followed by dataISize,
  // thus we discard the first element
  vector <SgVariableDeclaration *>::iterator localVarIt =
      declaredHostRoutineLocals.begin ();

  localVarIt++;

  SgStatement * lastAppendedStatement = NULL;

  for (int i = 0; i < numberOfArgumentGroups; i++, argIt++, localVarIt++)
  {

    // transform the expression to a variable reference
    SgExpression * opDatArgRef = buildVarRefExp (*argIt);

    // build: argI%set
    SgVarRefExp * setField = buildOpaqueVarRefExp ("set", scope);
    SgExpression * argISetField = buildDotExp (opDatArgRef, setField);

    // build: (argI%set)%size
    SgVarRefExp * sizeField = buildOpaqueVarRefExp ("size", scope);
    SgExpression * argISetSizeField = buildDotExp (argISetField, sizeField);

    // build: argI%dim
    SgVarRefExp * dimField = buildOpaqueVarRefExp ("dim", scope);
    SgExpression * argIDimField = buildDotExp (opDatArgRef, dimField);

    // build: ( arg0%dim * (arg0%set)%size)
    SgExpression * multiplExpr = buildMultiplyOp (argIDimField,
        argISetSizeField);

    // get variable reference to dataISize local variable
    //		ROSE_ASSERT ( **localVarIt != NULL );
    SgVarRefExp * dataISizeRef = buildVarRefExp (*localVarIt);

    // build: data0Size = ( arg0%dim * (arg0%set)%size)
    SgExpression * assignDataISize = buildAssignOp (dataISizeRef, multiplExpr);

    // finally append the statement
    SgStatement * assignDataISizeStmt = buildExprStatement (assignDataISize);
    appendStatement (assignDataISizeStmt, scope);

    if (i == numberOfArgumentGroups - 1) // last iteration
      lastAppendedStatement = assignDataISizeStmt;
  }

  // the first set of numberOfArgumentGroups variables in declaredC2FortranVariables
  // is the one of the argumentI variables, which is the ones that we are going to use now
  vector <SgVariableDeclaration *>::iterator c2fVarIt =
      declaredC2FortranVariables.begin ();

  // build: 	allocate ( argument0 ( data0Size ) )
  //					allocate ( argumentN-11 ( dataN-11Size ) )

  // we need the dataISize variables again
  localVarIt = declaredHostRoutineLocals.begin ();

  // avoid to consider the CRet variable
  localVarIt++;

  for (int i = 0; i < numberOfArgumentGroups; i++, c2fVarIt++, localVarIt++)
  {

    // build function type for allocate subroutine
    string const allocateName = "allocate";

    // build: argumentI ( dataISize )
    // We have to use a trick: we treat argumentI as an array (as it is) and we dereference it with dataISize

    // transform the expression argumentI to a variable reference
    SgVarRefExp * argumentIVarRef = buildVarRefExp (*c2fVarIt);
    SgVarRefExp * dataISizeVarRef = buildVarRefExp (*localVarIt);

    SgExpression * fakeArrayAccess = buildPntrArrRefExp (argumentIVarRef,
        dataISizeVarRef);

    SgExprListExp * fakeArrayAccessList = buildExprListExp (fakeArrayAccess);

    // building array type with : shape specifier
    SgColonShapeExp * colonExp = new SgColonShapeExp ();
    Sg_File_Info* colonFileInfo = new Sg_File_Info ();
    colonExp->set_startOfConstruct (colonFileInfo);
    colonExp->set_endOfConstruct (colonFileInfo);

    // for now, the type is fixed to real(8) = double precision, but we have to get it
    // from the declaration op_decl_dat -> declaration of input data

    // building real(kind=8): NOT WORKING FOR NOW (it seems that real(8) and dimension(:) do
    // not like each other in the unparser
    //		SgModifierType * realEight = buildFortranKindType ( buildFloatType(), buildIntVal ( 8 ) );

    // Unfortunately, we cannot use CALL with a Fortran function: we have to use addTextForUnaparser for now
    string const allocateStringfied = "allocate";
    string const allocateParamsStringfied =
        fakeArrayAccessList->unparseToString ();
    string const allocateStmtStringfied = allocateStringfied + "("
        + allocateParamsStringfied + ")\n";

    addTextForUnparser (lastAppendedStatement, allocateStmtStringfied,
        AstUnparseAttribute::e_after);
  }

  // build: 	call c_f_pointer ( arg0%dat, c2fPtr0, (/data0Size/) )
  //					call c_f_pointer ( arg1%dat, c2fPtr1, (/data1Size/) )

  // iterator over op_dat formal parameters
  argIt = opDatFormalArgs.begin ();

  // we need the dataISize variables again
  localVarIt = declaredHostRoutineLocals.begin ();

  // avoid to consider the CRet variable
  localVarIt++;

  string const c2fFunName = "c_f_pointer";

  // The good news is that the c2fVarIt should point exactly to the first c2fPtrI
  for (int i = 0; i < numberOfArgumentGroups; i++, c2fVarIt++, argIt++, localVarIt++)
  {
    // build: argI%dat

    // transform the expression to a variable reference
    SgVarRefExp * opDatArgRef = buildVarRefExp (*argIt);

    // argI%dat
    SgVarRefExp * datField = buildOpaqueVarRefExp ("dat", scope);
    SgExpression * argIDatField = buildDotExp (opDatArgRef, datField);

    // transform the expression c2fPtrI to a variable reference
    SgVarRefExp * c2fPtrIVarRef = buildVarRefExp (*c2fVarIt);

    // there is not another known way to build (/size/)
    SgInitializedNamePtrList singleVarName = (*localVarIt)->get_variables ();
    SgInitializedNamePtrList::iterator singleVarNameIt = singleVarName.begin ();

    string const sizeShapeName = "(/"
        + (*singleVarNameIt)->get_name ().getString () + "/)";
    SgExpression * shapeExpr = buildOpaqueVarRefExp (sizeShapeName, scope);

    SgExprListExp * c2fFunActualParameters = buildExprListExp (argIDatField,
        c2fPtrIVarRef, shapeExpr);

    // TODO: transform types to the correct ones!
    SgFunctionParameterTypeList * c2fFunInputTypes =
        buildFunctionParameterTypeList (buildIntType (), buildIntType (),
            buildIntType ());

    SgFunctionType * c2fFunType = buildFunctionType (buildVoidType (),
        c2fFunInputTypes);
    SgFunctionRefExp * c2fFunRef = buildFunctionRefExp (*(new SgName (
        c2fFunName)), c2fFunType, scope);
    // build function call
    SgFunctionSymbol * c2fFunSymbol = c2fFunRef->get_symbol_i ();
    SgFunctionCallExp * c2fFunCall = buildFunctionCallExp (c2fFunSymbol,
        c2fFunActualParameters);

    SgStatement * c2fFunCallStmt = buildExprStatement (c2fFunCall);

    appendStatement (c2fFunCallStmt, scope);
  }

  // build:	argument0 = c2fPtr0
  //				argument1 = c2fPtr1
  // As both variable declarations are stored in the same vector, we need two iteratos

  // first come the argumentI variables
  vector <SgVariableDeclaration *>::iterator argumentIVarIt =
      declaredC2FortranVariables.begin ();

  // then, after <numberOfArgumentGroups> argumentI variables, we have the c2fPtrI variables
  vector <SgVariableDeclaration *>::iterator c2fPtrVarIt =
      declaredC2FortranVariables.begin ();
  c2fPtrVarIt += numberOfArgumentGroups; // exploiting pointer arithmetics!


  for (int i = 0; i < numberOfArgumentGroups; i++, argumentIVarIt++, c2fPtrVarIt++)
  {
    SgVarRefExp * argumentIRef = buildVarRefExp (*argumentIVarIt);
    SgVarRefExp * c2FPtrIRef = buildVarRefExp (*c2fPtrVarIt);

    SgExpression * assignC2FPtrIToArgumentI = buildAssignOp (argumentIRef,
        c2FPtrIRef);
    SgStatement * assignC2FPtrIToArgumentIStmt = buildExprStatement (
        assignC2FPtrIToArgumentI);

    appendStatement (assignC2FPtrIToArgumentIStmt, scope);
  }

  // build: call op_cuda_save_soln<<<gsize,bsize,reduct_shared>>> ( set%size, argument0, argument1  )

  // build: argument0, argument1, set%size
  argumentIVarIt = declaredC2FortranVariables.begin ();
  SgExprListExp * kPars = buildExprListExp ();

  // build: set%size
  SgExpression * iterationSetVarRef = buildVarRefExp (iterationSetFormalArg);
  SgVarRefExp * setSizeField = buildOpaqueVarRefExp ("size", scope);
  SgExpression * setFieldSizeExpr = buildDotExp (iterationSetVarRef,
      setSizeField);
  kPars->append_expression (setFieldSizeExpr);

  // build: argumentI
  for (int i = 0; i < numberOfArgumentGroups; i++, argumentIVarIt++)
  {
    SgVarRefExp * argumentIRef = buildVarRefExp (*argumentIVarIt);
    kPars->append_expression (argumentIRef);
  }

  SgExprStatement * kCall = buildFunctionCallStmt ("op_cuda_" + kernelName
      + "<<<gsize,bsize,reduct_shared>>>", buildVoidType (), kPars, scope);

  appendStatement (kCall, scope);

  // building copy back
  argumentIVarIt = declaredC2FortranVariables.begin ();
  c2fPtrVarIt = declaredC2FortranVariables.begin ();
  c2fPtrVarIt += numberOfArgumentGroups; // exploiting pointer arithmetics!

  for (int i = 0; i < numberOfArgumentGroups; i++, argumentIVarIt++, c2fPtrVarIt++)

  {
    SgVarRefExp * argumentIRef = buildVarRefExp (*argumentIVarIt);
    SgVarRefExp * c2FPtrIRef = buildVarRefExp (*c2fPtrVarIt);

    SgExpression * assignArgumentIToC2FPtrI = buildAssignOp (c2FPtrIRef,
        argumentIRef);
    SgStatement * assignArgumentIToC2FPtrIStmt = buildExprStatement (
        assignArgumentIToC2FPtrI);

    appendStatement (assignArgumentIToC2FPtrIStmt, scope);
  }
}

SgProcedureHeaderStatement *
CreateKernels::createHostSubroutine (std::string kernelName,
    SgExpressionPtrList& args, SgSourceFile& sourceFile,
    SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;
  using namespace OP2;

  /*
   * Create host subroutine parameters for 'op_par_loop_2'
   */
  SgFunctionParameterList* hostParameters = buildFunctionParameterList ();

  /*
   * Create host subroutine and append it to statements inside module
   */

  // generating name" op_par_loop_<kernel_name>
  string const hostSubroutineName = OP_PAR_LOOP_PREFIX + "_"
      + kernelName.c_str ();

  SgProcedureHeaderStatement* subroutineStatement =
      buildProcedureHeaderStatement (hostSubroutineName.c_str (),
          buildVoidType (), hostParameters,
          SgProcedureHeaderStatement::e_subroutine_subprogram_kind, scope);

  // first let's add the attribute statement
  addTextForUnparser (subroutineStatement, "attributes(host) ",
      AstUnparseAttribute::e_before);

  // then let's append the subroutine defining declaration
  appendStatement (subroutineStatement, scope);

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
            string const className = classReference->get_name ().getString ();

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

  // creating local variables of the host subroutine
  createHostDeviceLocals (subroutineScope, args);

  // creating the main statements of the host subroutine (actual kernel calls)
  createMainRoutineStatements (subroutineScope, args);

  return subroutineStatement;
}

/*
 * Build a module
 */
SgModuleStatement *
CreateKernels::buildClassDeclarationAndDefinition (std::string name,
    SgScopeStatement* scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  // This function builds a class declaration and definition 
  // (both the defining and nondefining declarations as required).

  // Build a file info object marked as a transformation
  Sg_File_Info* fileInfo =
      Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode ();
  //	fileInfo->setOutputFileGeneration();
  fileInfo->setOutputInCodeGeneration ();

  assert(fileInfo != NULL);

  // This is the class definition (the fileInfo is the position of the opening brace)
  SgClassDefinition* classDefinition = new SgClassDefinition (fileInfo);
  assert(classDefinition != NULL);

  // Set the end of construct explictly (where not a transformation this is the location of the closing brace)
  classDefinition->set_endOfConstruct (fileInfo);

  // This is the defining declaration for the class (with a reference to the class definition)
  SgModuleStatement * classDeclaration = new SgModuleStatement (fileInfo,
      name.c_str (), SgClassDeclaration::e_struct, NULL, classDefinition);
  assert(classDeclaration != NULL);

  // Set the defining declaration in the defining declaration!
  classDeclaration->set_definingDeclaration (classDeclaration);

  // Set the non defining declaration in the defining declaration (both are required)
  SgClassDeclaration* nondefiningClassDeclaration = new SgClassDeclaration (
      fileInfo, name.c_str (), SgClassDeclaration::e_struct, NULL, NULL);
  assert(classDeclaration != NULL);
  nondefiningClassDeclaration->set_type (SgClassType::createType (
      nondefiningClassDeclaration));

  // Set the internal reference to the non-defining declaration
  classDeclaration->set_firstNondefiningDeclaration (
      nondefiningClassDeclaration);
  classDeclaration->set_type (nondefiningClassDeclaration->get_type ());

  // Set the defining and no-defining declarations in the non-defining class declaration!
  nondefiningClassDeclaration->set_firstNondefiningDeclaration (
      nondefiningClassDeclaration);
  nondefiningClassDeclaration->set_definingDeclaration (classDeclaration);

  // Set the nondefining declaration as a forward declaration!
  nondefiningClassDeclaration->setForward ();

  // Don't forget the set the declaration in the definition (IR node constructors are side-effect free!)!
  classDefinition->set_declaration (classDeclaration);

  // set the scope explicitly (name qualification tricks can imply it is not always the parent IR node!)
  classDeclaration->set_scope (scope);
  nondefiningClassDeclaration->set_scope (scope);

  // some error checking
  assert(classDeclaration->get_definingDeclaration() != NULL);
  assert(classDeclaration->get_firstNondefiningDeclaration() != NULL);
  assert(classDeclaration->get_definition() != NULL);

  // DQ (9/8/2007): Need to add function symbol to global scope!
  SgClassSymbol* classSymbol = new SgClassSymbol (classDeclaration);
  scope->insert_symbol (classDeclaration->get_name (), classSymbol);
  ROSE_ASSERT(scope->lookup_class_symbol(classDeclaration->get_name()) != NULL);

  return classDeclaration;
}

void
CreateKernels::createAndAppendUserKernelFormalParameters (
    SgFunctionParameterList * newParList,
    SgFunctionParameterList * originalParams,
    SgScopeStatement * subroutineScope)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  // now creating declarations of formal parameters
  ROSE_ASSERT ( newParList != NULL );
  ROSE_ASSERT ( originalParams != NULL );

  SgInitializedNamePtrList list = originalParams->get_args ();

  SgInitializedNamePtrList::iterator varMod;

  for (varMod = list.begin (); varMod != list.end (); varMod++)
  {

    SgVariableDeclaration * newVarDec = buildVariableDeclaration (*(new SgName (
        (*varMod)->get_name ().getString ())), (*varMod)->get_typeptr (), NULL,
        subroutineScope);

    newVarDec->get_declarationModifier ().get_accessModifier ().setUndefined ();
    newVarDec->get_declarationModifier ().get_typeModifier ().setDevice ();

    newParList->append_arg (*(newVarDec->get_variables ().begin ()));

    // appending variable declaration in body
    appendStatement (newVarDec, subroutineScope);
  }
}

bool
CreateKernels::checkAlreadyDeclared (SgVariableDeclaration * possiblyAvoidCopy,
    SgFunctionParameterList * newParList)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  // not yet found implicit none statement => not yet declared formal parameters
  if (newParList == NULL)
    return false;

  SgInitializedNamePtrList list = newParList->get_args ();

  SgInitializedNamePtrList::iterator varMod;

  for (varMod = list.begin (); varMod != list.end (); varMod++)
  {
    if (strcmp (
        possiblyAvoidCopy->get_definition ()->get_vardefn ()->get_name ().str (),
        (*varMod)->get_name ().str ()) == 0)
    {
      // found a variable declaration in original kernel which is a formal parameter
      // => do not re-declare it
      return true;
    }
  }

  // if not formal par is equal to the found var decl, I have to append the statement
  return false;
}

SgProcedureHeaderStatement *
CreateKernels::generateCUDAUserKernel (SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

  // I need to find the definition inside the correct file (otherwise the definition would be empty)
  vector <SgProcedureHeaderStatement *>::iterator it =
      inputSubroutines.begin ();

  while (kernelName.compare ((*it)->get_name ().getString ()) != 0)
  {
    it++;
  }

  SgProcedureHeaderStatement * subroutine;
  SgProcedureHeaderStatement * realRoutine;
  SgFunctionParameterList * newParList = NULL;
  SgFunctionParameterList * originalParams = NULL;

  if (kernelName.compare ((*it)->get_name ().getString ()) == 0)
  {
    realRoutine = *it;

    string const USER_KERNEL_TRANSFORMED = "_FORTRAN_CUDA";
    string const newKernelName = (realRoutine->get_name ()).str ()
        + USER_KERNEL_TRANSFORMED;

    originalParams = realRoutine->get_parameterList ();
    newParList = buildFunctionParameterList ();

    subroutine = buildProcedureHeaderStatement (newKernelName.c_str (),
        buildVoidType (), newParList,
        SgProcedureHeaderStatement::e_subroutine_subprogram_kind, scope);

    /*
     * now visiting the subtree to:
     * 1. append the statement to the statements to be unparse (visiting the statement list)
     *		(TODO: may not work for nested bodies: the alternative is to use a copy routine that currently does
     *		 not work)
     * 2. set its OutputInCodeGeneration flag to true (using a special visitor)
     */

    SgScopeStatement * subroutineScope =
        subroutine->get_definition ()->get_body ();
    Rose_STL_Container< SgStatement * > toFill = subroutine->get_definition()->get_body()->get_statements();

    Rose_STL_Container < SgStatement * > filling = realRoutine->get_definition()->get_body()->get_statements();

    Rose_STL_Container < SgStatement * >::iterator copyIt;

    SgVariableDeclaration * possiblyAvoidCopy = NULL;

    for ( copyIt = filling.begin(); copyIt != filling.end(); copyIt++ )
    {

      // copy if: 1. it is not a variable declaration OR
      //			    2. it is a variable declaration BUT it is not a formal parameter declaration
      if ( ( ( possiblyAvoidCopy = isSgVariableDeclaration ( *copyIt ) ) == NULL ) ||
          ( ( ( possiblyAvoidCopy = isSgVariableDeclaration ( *copyIt ) ) != NULL ) &&
              checkAlreadyDeclared ( possiblyAvoidCopy, newParList ) == false ) )
      {

        // adding the statement to the statement list
        toFill.push_back ( *copyIt );

        // appending statement too
        appendStatement ( *copyIt, subroutineScope );

        if ( isSgImplicitStatement ( *copyIt ) != NULL )
        {
          // immediately after the implicit none statement, we append the new variable declarations
          createAndAppendUserKernelFormalParameters ( newParList, originalParams, subroutineScope );
        }
      }
    }
  }

  deepVisitAndSetGeneration * newVisit = new deepVisitAndSetGeneration ();
  newVisit->traverse ( (SgNode *) subroutine, preorder );

  // now modifying the formal parameter declaration adding the "device" attribute
  //		SgFunctionParameterList * parameterList = subroutine->get_parameterList();
  //		(SgFunctionParameterList * ) deepCopy ( subroutine->get_parameterList() );


  // scan the parameter list and the statements: when a same name is found, we add the device attribute
  // typedef in rose is   Rose_STL_Container<SgInitializedName*> --> SgInitializedNamePtrList
  // TODO: this is somehow too general, if we consider that all fortran formal parameters are
  // declared at the beginning of the declaration section and their number is known (!)

  //		SgInitializedNamePtrList list = newParList->get_args();
  //
  //		SgInitializedNamePtrList::iterator varMod;
  //
  //		for ( varMod = list.begin(); varMod != list.end(); varMod++ ) {
  //
  //			(*varMod)->get_declaration()->get_definingDeclaration()->get_declarationModifier().get_typeModifier().setDevice();


  // checking every statement inside the subroutine
  //			copyIt = toFill.begin();
  //			while ( copyIt != toFill.end() ) {
  //				// check is copyIt is pointing to a SgVariableDeclaration
  //				SgVariableDeclaration * isVar;
  //				if ( (isVar = isSgVariableDeclaration ( *copyIt ) ) != NULL ) {
  //					// check if the name corresponds to the one we are looking for
  //					if ( isVar->get_definition()->get_vardefn()->get_name().getString() ==
  //							(*varMod)->get_name().getString() ) {
  //
  //						// found the variable: adding the DEVICE attribute
  //						(*varMod)->get_declaration()->get_declarationModifier().get_typeModifier().setDevice();
  //						// if I have found the variable, I can stop searching
  //
  //						break;
  //					}
  //				}
  //				copyIt++;
  //			}
  //}
  //}
  // first let's add the attribute statement
  addTextForUnparser ( subroutine, "attributes(device) ", AstUnparseAttribute::e_before );

  // then let's add the statement to the module
  appendStatement ( subroutine, scope );

  return subroutine;
}

void
CreateKernels::lookupArgumentsTypes (std::vector <SgType *> * argTypes,
    SgExpressionPtrList& args, int loopDatArgNumber,
    SgVariableDeclaration * setSizeFormalPar)
{
  using namespace std;
  using namespace SageBuilder;

  // build: user type arrays with bounds 0:op_set%size * op_dat%dim - 1
  for (int i = 0; i < loopDatArgNumber; i++)
  {

    // build setsize * op_dat%dim (for now I fixed the dimension)
    // access to actualArgumentsDimValues to get the op_dat dimension

    int datDimension = actualArgumentsDimValues[i];

    SgExpression * multiplySubscr = buildMultiplyOp (buildVarRefExp (
        setSizeFormalPar), buildIntVal (datDimension));

    ROSE_ASSERT ( multiplySubscr != NULL );

    // building (op_set%size * op_dat%dim)  -1
    SgExpression * upperBound = buildSubtractOp (multiplySubscr,
        buildIntVal (1));

    ROSE_ASSERT ( upperBound != NULL );

    // We need a file_info because we need to tell rose that the subscript expression
    // must be generated in the output file. If we use the same file_info of the
    // source file we end up with an empty output file
    Sg_File_Info * newFileInfo = new Sg_File_Info ();

    // 0: op_set%size * op_dat%dim - 1
    //(for now we know that the set is the same for all arguments = args[2].size
    SgSubscriptExpression * arraySubScript = new SgSubscriptExpression (
        newFileInfo, buildIntVal (0), upperBound, buildIntVal (1));

    // as we are not using a builder from the SageBuilder interface we need to set things properly..
    arraySubScript->set_endOfConstruct (newFileInfo);
    arraySubScript->setCompilerGenerated ();
    arraySubScript->setOutputInCodeGeneration ();

    // the correct type can be retrieved from the stored information
    SgType * baseType = actualArgumentsTypes[i];

    SgArrayType * guessArrayType = NULL;
    if ((guessArrayType = isSgArrayType (baseType)) != NULL)
    {
      // if the user-defined type is already an array (as it is always or nearly the case), we need to modify
      // the array subscript. If we copy the array type, we won't get the attributes, hence we copy
      // only the base type and we re-create the array type
      baseType = guessArrayType->get_base_type ();
    }

    // build array type (real(8) for now) with the correct subscript
    SgArrayType * arrayType = buildArrayType (baseType, arraySubScript);

    SgExprListExp * dimensionExprList = buildExprListExp (arraySubScript);

    arrayType->set_dim_info (dimensionExprList);
    arrayType->set_rank (1);

    argTypes->push_back (arrayType);
  }
}

/*
 * Builds the parameter of the user kernel subroutine called by the main kernel subroutine
 */
SgExprListExp *
CreateKernels::buildUserKernelParams (
    SgFunctionParameterList * mainKernelParameters,
    SgVarRefExp * iterSetVarRef, SgScopeStatement * subroutineScope)
{
  using namespace std;
  using namespace SageBuilder;

  Debug::getInstance ()->debugMessage (
      "Inside 'OpParLoop::buildUserKernelParams'", 0);

  // obtain argument list from input parameters list
  SgInitializedNamePtrList mainKernelArgs = mainKernelParameters->get_args ();

  // the size of user kernel call is equal to the number of parameters to the main user kernel function
  // minus 1 (the first argument is the setsize)
  vector <SgExpression *> vectParsExps;// = new vector < SgExpression * > ( mainKernelArgs.size() -1 );

  SgInitializedNamePtrList::iterator inPrIterator = mainKernelArgs.begin ();
  // the first argument is always setsize, so we discard it
  inPrIterator++;

  // we visit each argument and build a corresponding varRefExpr
  for (int i = 0; inPrIterator != mainKernelArgs.end (); i++, inPrIterator++)
  {

    // generating: q(setIter * 4:setIter * 4 + 4 - 1)

    // Generating subscript

    // generating subscript: setIter * 4:setIter * 4 + 4 - 1
    //(for now we know that the set is the same for all arguments = args[2].size

    // setIter * 4
    SgIntVal * opDatDim = buildIntVal (actualArgumentsDimValues[i]);

    SgExpression * lowerBound = buildMultiplyOp (iterSetVarRef, opDatDim);

    // setIter * 4 + 4
    SgExpression * setIterMulDimPlusDim = buildAddOp (lowerBound, opDatDim);

    // setIter * 4 + 4 - 1
    SgExpression * upperBound = buildSubtractOp (setIterMulDimPlusDim,
        buildIntVal (1));

    //setIter * 4:setIter * 4 + 4 - 1 (we need file info stuff because we are not using SageBuilder,
    // as there is not such a building function)
    Sg_File_Info * newFileInfo = new Sg_File_Info ();

    SgSubscriptExpression * arraySubScript = new SgSubscriptExpression (
        newFileInfo, lowerBound, upperBound, buildIntVal (1));

    // as we are not using a builder from the SageBuilder interface we need to set things properly..
    arraySubScript->set_endOfConstruct (newFileInfo);
    arraySubScript->setCompilerGenerated ();
    arraySubScript->setOutputInCodeGeneration ();

    SgExpression * formalParRef = buildVarRefExp ((*inPrIterator)->get_name (),
        subroutineScope);

    SgExpression * parAccess =
        buildPntrArrRefExp (formalParRef, arraySubScript);

    vectParsExps.push_back (parAccess);
  }

  // do not know why this is not done inside the build function below
  Sg_File_Info * userKernelParamsFileInfo = new Sg_File_Info ();

  SgExprListExp * userKernelParams = buildExprListExp_nfi (vectParsExps);
  userKernelParams->set_parent (subroutineScope);
  userKernelParams->set_startOfConstruct (userKernelParamsFileInfo);
  userKernelParams->set_endOfConstruct (userKernelParamsFileInfo);
  userKernelParams->set_file_info (userKernelParamsFileInfo);
  userKernelParams->setCompilerGenerated ();
  userKernelParams->setOutputInCodeGeneration ();

  return userKernelParams;
}

void
CreateKernels::buildMainKernelRoutine (SgScopeStatement * scope,
    SgExpressionPtrList& args, SgName deviceKernelName, Sg_File_Info & fileInfo)
{
  using namespace std;
  using namespace SageInterface;
  using namespace SageBuilder;

  // generating main kernel calling the user one and called by the host routine

  // 1. generating name for main kernel
  string const moduleName = "op_cuda_" + kernelName;

  // 2. generting parameters for main kernel
  SgFunctionParameterList * mainKernelParameters =
      buildFunctionParameterList ();

  // 3. building subroutines
  SgProcedureHeaderStatement * mainHostSubroutine =
      buildProcedureHeaderStatement (moduleName.c_str (), buildVoidType (),
          mainKernelParameters,
          SgProcedureHeaderStatement::e_subroutine_subprogram_kind, scope);

  // first let's add the attribute statement
  addTextForUnparser (mainHostSubroutine, "attributes(global) ",
      AstUnparseAttribute::e_before);

  // then let's add the statement to the module
  appendStatement (mainHostSubroutine, scope);

  // obtain the subroutine scope to append parameter declaration, local variable declarations and body
  SgScopeStatement * subroutineScope =
      mainHostSubroutine->get_definition ()->get_body ();

  // 4. create function body
  string const varNamePrefix = "arg";

  // a. creating setsize formal argument
  string const setSizeName = "setsize";
  SgVariableDeclaration * setSizeFormalPar = buildVariableDeclaration (
      *(new SgName (setSizeName)), buildIntType (), NULL, subroutineScope);

  setSizeFormalPar->get_declarationModifier ().get_accessModifier ().setUndefined ();
  setSizeFormalPar->get_declarationModifier ().get_typeModifier ().setValue ();

  // not needed
  //setSizeFormalPar->get_declarationModifier ().get_typeModifier().setIntent_in();

  // appending to formal arguments
  mainKernelParameters->append_arg (
      *(setSizeFormalPar->get_variables ().begin ()));

  // appending variable declaration in body
  appendStatement (setSizeFormalPar, subroutineScope);

  // b. Building remaining formal parameters types from program
  vector <SgType *> * argTypes = new vector <SgType *> ;

  lookupArgumentsTypes (argTypes, args, numberOfArgumentGroups,
      setSizeFormalPar);

  //cout << "Got argtypes: Number of OP_ARG = " << loopDatArgNumber << endl;


  // check that the size of argTypes is exactly the expected number of arguments to avoid wrong accesses to argTypes below
  ROSE_ASSERT ( argTypes->size() == numberOfArgumentGroups );

  // b. create formal parameters (we have a OP_DAT argument starting from 3 and then every 4 arguments
  // (4 is stored inside a macro (ARG_LINE_LENGTH) copied inside variable argLineLength in constructor
  // retriving op_dat original type from argument: must generate real(8), dimension(0:setsize * 4 -1)
  // where 4 is the carinality of each input op_dat element associate to each set element (can be found
  // in the op_decl_dat call)

  for (unsigned int i = 0; i < numberOfArgumentGroups; i++)
  {
    // building string with i value inside
    stringstream os;
    os << i;
    string const i_str = os.str ();

    // building variable name (arg_i)
    string const varName = varNamePrefix + "_" + i_str;

    SgVariableDeclaration * newFormalPar = buildVariableDeclaration (
        *(new SgName (varName)), (*argTypes)[i], NULL, subroutineScope);

    // getting actual InitializedName for variable definition from variables vector inside the new formal parameter
    SgInitializedName * varInitName =
        *(newFormalPar->get_variables ().begin ());

    // appending formal argument to the subroutine list of arguments
    mainKernelParameters->append_arg (varInitName);

    // add intent(in) to this parameter attributes
    newFormalPar->get_declarationModifier ().get_typeModifier ().setIntent_in ();

    // add device to this parameter attributes
    newFormalPar->get_declarationModifier ().get_typeModifier ().setDevice ();

    newFormalPar->get_declarationModifier ().get_accessModifier ().setUndefined ();

    // appending new formal parameter to subroutine
    appendStatement (newFormalPar, subroutineScope);
  }

  // Create set iteration variable
  string const iterSetName = "setIter";
  SgVariableDeclaration * iterSetVar = buildVariableDeclaration (*(new SgName (
      iterSetName)), buildIntType (), NULL, subroutineScope);

  // add device to setIter variable
  iterSetVar->get_declarationModifier ().get_typeModifier ().setDevice ();

  iterSetVar->get_declarationModifier ().get_accessModifier ().setUndefined ();

  // append setIter variable
  appendStatement (iterSetVar, subroutineScope);

  // Initialise set iteration variable:
  // setIter = (threadidx%x - 1) + (blockidx%x -1)  * blockdim%x

  // building blockdim%x
  // as blockdim and x are not declared in the input file nor in the generated one,
  // we need to use an opaque expression
  SgVarRefExp * blockDim = buildOpaqueVarRefExp ("blockdim", subroutineScope);
  SgVarRefExp * xField = buildOpaqueVarRefExp ("x", subroutineScope);
  SgExpression * blockDimXAcc = buildDotExp (blockDim, xField);

  ROSE_ASSERT ( blockDim != NULL );
  ROSE_ASSERT ( xField != NULL );
  ROSE_ASSERT ( blockDimXAcc != NULL );

  // building threadidx%x - 1
  SgVarRefExp * threadIdX = buildOpaqueVarRefExp ("threadidx", subroutineScope);
  SgExpression * threadIdXAcc = buildDotExp (threadIdX, xField);
  SgExpression * threadIdXMinusOne = buildSubtractOp (threadIdXAcc,
      buildIntVal (1));

  ROSE_ASSERT ( threadIdX != NULL );
  ROSE_ASSERT ( threadIdXAcc != NULL );
  ROSE_ASSERT ( threadIdXMinusOne != NULL );

  // building blockidx%x - 1
  SgVarRefExp * blockIdX = buildOpaqueVarRefExp ("blockIdX", subroutineScope);
  SgExpression * blockthreadIdXAcc = buildDotExp (blockIdX, xField);
  SgExpression * blockIdXMinusOne = buildSubtractOp (blockthreadIdXAcc,
      buildIntVal (1));

  ROSE_ASSERT ( blockIdX != NULL );
  ROSE_ASSERT ( blockthreadIdXAcc != NULL );
  ROSE_ASSERT ( blockIdXMinusOne != NULL );

  // building (blockidx%x -1)  * blockdim%x
  SgExpression * blockIdXMinusOnePerBlockDimx = buildMultiplyOp (
      blockIdXMinusOne, blockDimXAcc);

  // building (threadidx%x - 1) + (blockidx%x -1)  * blockdim%x
  SgExpression * setIterInitValue = buildAddOp (threadIdXMinusOne,
      blockIdXMinusOnePerBlockDimx);

  //	ROSE_ASSERT ( thIdPlusBlockId != NULL );

  // building  (threadidx%x - 1) +


  ROSE_ASSERT ( setIterInitValue != NULL );

  // getting a reference to variable setIter for later assignment
  SgVarRefExp * setIterRef = buildVarRefExp (*(new SgName (iterSetName)),
      subroutineScope);

  ROSE_ASSERT ( setIterRef != NULL );

  // building final assignment to setIter
  SgExpression * setIterInitAssignement = buildAssignOp (setIterRef,
      setIterInitValue);

  ROSE_ASSERT ( setIterInitAssignement != NULL );

  // now building a corresponding statement
  SgExprStatement * assignStatement = buildExprStatement (
      setIterInitAssignement);

  ROSE_ASSERT ( assignStatement != NULL );

  // appending it to the scope
  appendStatement (assignStatement, subroutineScope);

  // Build do-while body

  // build user kernel subroutine call
  // 1. get function symbol from variable declaration
  //	SgFunctionSymbol * userKernelSymbol = new SgFunctionSymbol ( kernelCUDAVersionDecl );

  SgFunctionRefExp * userKernelRefExp = buildFunctionRefExp (deviceKernelName,
      subroutineScope->get_scope ());

  // 2. build parameters: q(setIter * dim:setIter * dim + dim - 1), qold(setIter * dim:setIter*dim + dim - 1)
  // dim is obtained from the previous parsing
  SgExprListExp * userKernelParams = buildUserKernelParams (
      mainKernelParameters, setIterRef, subroutineScope);

  // 3. build call
  SgFunctionCallExp * userKernelCall = buildFunctionCallExp (userKernelRefExp,
      userKernelParams);

  // 4. build corresponding statement
  SgExprStatement * userKernelCallStmt = buildExprStatement (userKernelCall);

  // build set iteration variable increment: setIter = setIter + blockdim%x * griddim%x
  // (we are only missing griddim%x)
  SgVarRefExp * gridDim = buildOpaqueVarRefExp ("griddim", subroutineScope);
  SgExpression * gridDimXAcc = buildDotExp (gridDim, xField);

  // build multiplication: blockdim%x * griddim%x
  SgExpression * blockDimMult = buildMultiplyOp (blockDimXAcc, gridDimXAcc);

  // build sum: setIter + blockdim%x * griddim%x
  SgExpression * sumIterPlusBlockGridX = buildAddOp (setIterRef, blockDimMult);

  // build assignment: setIter = setIter + blockdim%x * griddim%x
  SgExpression * setIterUpdate = buildAssignOp (setIterRef,
      sumIterPlusBlockGridX);

  // 4. build corresponding statement
  SgExprStatement * setIterUpdateStmt = buildExprStatement (setIterUpdate);

  SgBasicBlock * doWhileBlock = buildBasicBlock (userKernelCallStmt,
      setIterUpdateStmt);

  // Build do while statement (do while ( setIter < setsize ))

  // building reference to setsize parameter
  SgVarRefExp * setSizeFormalParRef = buildVarRefExp (
      *(new SgName (setSizeName)), subroutineScope);

  // build do-while guard
  SgExpression * inequalityExp = buildLessThanOp (setIterRef,
      setSizeFormalParRef);

  // build do-while statement
  SgWhileStmt * setWhileStmt = buildWhileStmt (inequalityExp, doWhileBlock);

  // we need to set the end do statement, because the unparse is not able to infer it automatically
  setWhileStmt->set_has_end_statement (true);

  // append do-while statement
  appendStatement (setWhileStmt, subroutineScope);
}

/*
 * Creates a CUDA source file
 */
SgSourceFile*
CreateKernels::createSourceFile (std::string kernelName)
{
  using namespace SageBuilder;
  using namespace std;

  /*
   * To create a new file (to which the AST is later unparsed), the API expects
   * the name of an existing file and the name of the output file. There is no
   * input file corresponding to our output file, therefore we first create a
   * dummy Fortran file
   */
  string const FORTRAN_FILE_SUFFIX = ".F95";
  string const inputFileName = "BLANK" + FORTRAN_FILE_SUFFIX;

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
  string const CUDA_FILE_NAME_SUFFIX = "_cuda_module.CUF";
  string const outputFileName = kernelName + CUDA_FILE_NAME_SUFFIX;

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

  sourceFile->set_outputFormat (SgFile::e_free_form_output_format);

  /*
   * Store the file so it can be unparsed after AST construction
   */
  kernelOutputFiles.push_back (sourceFile);

  return sourceFile;
}

SgScopeStatement *
CreateKernels::createDirectLoopCUDAModule (SgSourceFile& sourceFile)
{
  using namespace OP2;
  using namespace std;
  using namespace SageInterface;

  SgGlobal * globalScope = sourceFile.get_globalScope ();

  /*
   * Create module name
   */
  string const moduleName = OP_PAR_LOOP_PREFIX + "_" + kernelName + "_module";

  /*
   * Create the module statement
   */
  SgModuleStatement * cudaModule = buildClassDeclarationAndDefinition (
      moduleName, globalScope);

  cudaModule->get_definition ()->setCaseInsensitive (true);

  appendStatement (cudaModule, globalScope);

  /*
   * Add the 'use' statements of this module
   */
  string const isoCBindingLib = "ISO_C_BINDING";
  SgUseStatement* useStatement1 = new SgUseStatement (
      sourceFile.get_file_info (), isoCBindingLib, false);
  useStatement1->set_definingDeclaration (cudaModule);
  appendStatement (useStatement1, cudaModule->get_definition ());

  string const op2CBindingLib = "OP2_C";
  SgUseStatement* useStatement2 = new SgUseStatement (
      sourceFile.get_file_info (), op2CBindingLib, false);
  useStatement2->set_definingDeclaration (cudaModule);
  appendStatement (useStatement2, cudaModule->get_definition ());

  string const cudaLib = "cudafor";
  SgUseStatement* useStatement3 = new SgUseStatement (
      sourceFile.get_file_info (), cudaLib, false);
  useStatement3->set_definingDeclaration (cudaModule);
  appendStatement (useStatement3, cudaModule->get_definition ());

  /*
   * Add the 'contains' statement
   */
  SgContainsStatement * containsStatement = buildContainsStatement (
      sourceFile.get_file_info (), cudaModule->get_definition ());

  vector <SgDeclarationStatement *> & statementList =
      cudaModule->get_definition ()->getDeclarationList ();
  statementList.push_back (containsStatement);

  return containsStatement->get_scope ();
}

/*
 *  Replace the op_par_loop with op_par_loop_<kernel_name>
 */
void
CreateKernels::fixParLoops (SgFunctionCallExp * functionCallExp,
    std::string kernelName, SgProcedureHeaderStatement * hostSubroutine,
    SgScopeStatement * scope, std::string createdCUDAModuleName)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

  SgLocatedNode * functionCallNode = isSgLocatedNode (functionCallExp);

  ROSE_ASSERT ( functionCallNode != NULL );

  // getting file info for this node
  Sg_File_Info * functionCallFileInfo = functionCallNode->get_file_info ();

  // setting transformation node
  functionCallFileInfo->setTransformation ();

  // get the last declaration statement
  SgStatement * lastDeclStmt = findLastDeclarationStatement (scope);

  SgScopeStatement * parent = scope;

  // recursively go back in the scopes until we can find a declaration statement
  // we assume that there must be at least one declaration statement (TODO: fix this)
  while (lastDeclStmt == NULL)
  {
    parent = (SgScopeStatement *) parent->get_parent ();
    //parent = (SgScopeStatement *) parent->get_parent();
    if (parent == NULL)
    {
      Debug::getInstance ()->debugMessage (
          "Error: cannot locate a place to declare the kernel name while trasforming: '"
              + kernelName + "'", 1);

      exit (0);
    }

    lastDeclStmt = findLastDeclarationStatement (parent);
  }

  SgStatement * lookForUse = lastDeclStmt;
  SgUseStatement * gotUseStatement;

  do
  {
    lookForUse = getPreviousStatement (lookForUse);
    gotUseStatement = isSgUseStatement (lookForUse);
  }
  while (gotUseStatement == NULL);

  SgFile * enclosingFile = getEnclosingFileNode (gotUseStatement);

  SgUseStatement* useStatement1 = new SgUseStatement (
      enclosingFile->get_file_info (), createdCUDAModuleName, false);

  SgModuleStatement * gotUseModule = gotUseStatement->get_module ();
  useStatement1->set_definingDeclaration (gotUseModule);

  ROSE_ASSERT ( useStatement1 != NULL );
  ROSE_ASSERT ( gotUseStatement != NULL );

  insertStatement (gotUseStatement, useStatement1);

  /*
   * Build the type, which is a character array
   */
  SgExpression * index = buildIntVal (kernelName.size ());

  // not used for now
  //	std::string cCharName = "c_char";
  //	SgModifierType * cCharType = buildFortranKindType ( buildCharType (), buildWcharVal_nfi ( 0, cCharName ) );


  SgArrayType* charArray = buildArrayType (buildCharType (), index);

  /*
   * The dimension of the array is the asterisk
   */
  SgExprListExp* dimensionExprList = buildExprListExp (index);
  charArray->set_dim_info (dimensionExprList);

  /*
   * The character array only has one dimension
   */
  charArray->set_rank (1);

  SgAssignInitializer * kernelNameInit = buildAssignInitializer (
      buildStringVal (kernelName), charArray);

  /*
   * Create the declaration and append it
   */
  string const METANAME = "Name";
  SgVariableDeclaration * kernelMetaName = buildVariableDeclaration (
      *(new SgName (kernelName + METANAME)), charArray, kernelNameInit,
      getScope (lastDeclStmt));

  insertStatementAfter (lastDeclStmt, kernelMetaName);

  // modifying the call op_par_loop_* with a call to the newly built CUDA host subroutine
  SgFunctionRefExp * cudaOpParLoopRef = buildFunctionRefExp (hostSubroutine);
  functionCallExp->set_function (cudaOpParLoopRef);

  // modifiying the first parameter from kernel reference to kernel name

  // obtaining the arguments of the call
  SgExprListExp * args = functionCallExp->get_args ();
  ROSE_ASSERT ( args != NULL );

  SgExpressionPtrList &exprs = args->get_expressions ();

  // remove the first parameter corresponding to the kernel reference
  exprs.erase (exprs.begin ());

  // insert a reference to the variable containing the kernel name
  exprs.insert (exprs.begin (), buildVarRefExp (kernelMetaName));
}

void
CreateKernels::retrieveArgumentsTypes (SgExpressionPtrList& args)
{
  using namespace std;
  using namespace OP2;

  /*
   * Scan OP_DAT arguments and get the types by comparing the
   * name of the OP_DAT variable with the declaredOpDatVars vector content.
   * Note that the first OP_DAT argument starts at BASE_OPDAT_ARG_INDEX.
   * Then we have an OP_DAT argument every NUMBER_OF_OPDAT_ARGUMENTS
   */
  for (vector <SgExpression *>::iterator it = args.begin ()
      + BASE_OPDAT_ARG_INDEX; it != args.end (); it
      += NUMBER_OF_OPDAT_ARGUMENTS)
  {
    SgExpression * argument = *it;
    Debug::getInstance ()->debugMessage ("Argument type: "
        + argument->class_name (), 8);

    switch (argument->variantT ())
    {
      case V_SgVarRefExp:
      {
        /*
         * The argument of the OP_PAR_LOOP is a variable reference (expression)
         */
        SgVarRefExp * variableReference = isSgVarRefExp (argument);

        Debug::getInstance ()->debugMessage (
            "Variable reference expression type: "
                + variableReference->get_type ()->class_name (), 9);

        switch (variableReference->get_type ()->variantT ())
        {
          case V_SgClassType:
          {
            SgClassType* classReference = isSgClassType (
                variableReference->get_type ());
            string const className = classReference->get_name ().getString ();

            Debug::getInstance ()->debugMessage ("Class reference to: "
                + className, 10);

            if (strcmp (className.c_str (), OP_DAT_NAME.c_str ()) == 0)
            {
              /*
               * Found an OP_DAT variable.
               * We can now get the type of the corresponding input data from the
               *  name of the op_dat variable
               */
              SgExpressionPtrList declarationArgs =
                  opDeclaredVariables->getDeclaredOpDatArgs (
                      &(variableReference->get_symbol ()->get_name ()));

              SgVarRefExp * opDatInputArg = isSgVarRefExp (
                  declarationArgs[OP_DECL_DAT_INPUT_REF]);

              SgType * opDatInputType = opDatInputArg->get_type ();

              actualArgumentsTypes.push_back (opDatInputType);

              SgIntVal * opDatDimension = isSgIntVal (
                  declarationArgs[OP_DECL_DAT_DIMENSION_VAL]);

              if (opDatDimension != NULL)
              {
                actualArgumentsDimValues.push_back (
                    opDatDimension->get_value ());
              }
            }

            break;
          }

          default:
          {
            break;
          }
        }

        break;
      }

      default:
      {
        break;
      }
    }
  }
}

/*
 * ====================================================================================================
 * Public functions
 * ====================================================================================================
 */

/*
 * Over-riding implementation of the AST vertex traversal function
 */
void
CreateKernels::visit (SgNode * node)
{
  using namespace boost;
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;
  using namespace OP2;

  switch (node->variantT ())
  {
    case V_SgProcedureHeaderStatement:
    {
      /*
       * Remember all subroutine definitions so as to later recover
       * actual kernel definitions when copying them into the new files
       */

      SgProcedureHeaderStatement * procedureHeaderStatement =
          isSgProcedureHeaderStatement (node);
      inputSubroutines.push_back (procedureHeaderStatement);

      Debug::getInstance ()->debugMessage ("Found procedure header statement '"
          + procedureHeaderStatement->get_name ().getString () + "'", 8);

      break;
    }

    case V_SgFunctionCallExp:
    {
      /*
       * Function call found in the AST
       */
      SgFunctionCallExp* functionCallExp = isSgFunctionCallExp (node);

      string const
          calleeName =
              functionCallExp->getAssociatedFunctionDeclaration ()->get_name ().getString ();

      /*
       * The prefix of all parallel loop calls in Fortran contains 'op_par_loop'.
       * The suffix of the call, however, changes depending on the number of expected
       * parameters. Therefore, any match of this prefix indicates a call of interest
       * to the translator
       */
      if (starts_with (calleeName, OP_PAR_LOOP_PREFIX))
      {
        /*
         * Retrieve the arguments of the call
         */
        SgExpressionPtrList& args =
            functionCallExp->get_args ()->get_expressions ();

        /*
         * Arguments to OP_PAR_LOOP are grouped into batches of 4
         * arguments, plus another two formal parameters.
         * Remember how many groups there are as this is needed to
         * generate the correct number of local variables in certain
         * host functions
         */
        numberOfArgumentGroups = (args.size () - 2) / 4;

        Debug::getInstance ()->debugMessage ("Found call to '" + calleeName
            + "' with " + lexical_cast <string> (args.size ()) + " arguments",
            2);

        /*
         * An 'op_par_loop_2' always has 10 arguments
         */
        if (args.size () == 10)
        {
          /*
           * The first argument to an 'op_par_loop' call should be a reference to
           * the kernel function. Cast it and proceed, otherwise throw an exception
           */
          SgFunctionRefExp * functionRefExp =
              isSgFunctionRefExp (args.front ());
          try
          {
            if (functionRefExp != NULL)
            {
              kernelName
                  = functionRefExp->getAssociatedFunctionDeclaration ()->get_name ().getString ();

              Debug::getInstance ()->debugMessage ("Found kernel '"
                  + kernelName + "'", 2);

              /*
               * Generate the CUDA file for this kernel
               */
              SgSourceFile * sourceFile = createSourceFile (kernelName);

              /*
               * Retrieve arguments data types
               */
              retrieveArgumentsTypes (args);

              // TODO: substitute with a check of the input arguments to understand if it
              // is a direct or indirect loop
              // and remove the compare with op_par_loop_2 above
              if (true)
              {
                /*
                 * Generate and fill the CUDA module
                 */
                SgScopeStatement * moduleScope = createDirectLoopCUDAModule (
                    *sourceFile);

                // generating and modifying user kernel and appending it to the contains scope
                SgProcedureHeaderStatement * kernelCUDAVersionDecl =
                    generateCUDAUserKernel (moduleScope);

                buildMainKernelRoutine (moduleScope, args,
                    kernelCUDAVersionDecl->get_name (),
                    *(sourceFile->get_file_info ()));

                SgProcedureHeaderStatement * hostSubroutine =
                    createHostSubroutine (kernelName, args, *sourceFile,
                        moduleScope);

                SgExprStatement * exprStat = isSgExprStatement (
                    node->get_parent ());

                string const createdCUDAModuleName = OP_PAR_LOOP_PREFIX + "_"
                    + kernelName + "_module";
                fixParLoops (functionCallExp, kernelName, hostSubroutine,
                    exprStat->get_scope (), createdCUDAModuleName);
              }
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
      SgSourceFile * sourceFile = isSgSourceFile (node);

      Debug::getInstance ()->debugMessage ("Found file "
          + sourceFile->getFileName (), 8);
      break;
    }

    default:
    {
      break;
    }
  }
}

void
CreateKernels::unparse ()
{
  using namespace std;

  Debug::getInstance ()->verboseMessage ("Generating CUDA files");

  for (vector <SgSourceFile*>::iterator it = kernelOutputFiles.begin (); it
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

void
deepVisitAndSetGeneration::visit (SgNode * nextNode)
{
  SgLocatedNode * castedNode;
  switch (nextNode->variantT ())
  {
    default:
      if ((castedNode = isSgLocatedNode (nextNode)) != NULL)
        castedNode->setOutputInCodeGeneration ();
  }
}
