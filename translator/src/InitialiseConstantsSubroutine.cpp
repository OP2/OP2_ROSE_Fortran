#include <InitialiseConstantsSubroutine.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <ROSEHelper.h>

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

void
InitialiseConstantsSubroutine::generateSubroutine ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildFloatVal;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using SageInterface::appendStatement;
  using std::make_pair;
  using std::map;
  using std::string;
  using std::vector;

  /*
   * ======================================================
   * Build the subroutine local variables
   * ======================================================
   */

  string const variableName_e = "e";
  string const variableName_p = "p";
  string const variableName_r = "r";
  string const variableName_u = "u";

  vector <string> doublePrecisionVariables;

  doublePrecisionVariables.push_back (variableName_e);
  doublePrecisionVariables.push_back (variableName_p);
  doublePrecisionVariables.push_back (variableName_r);
  doublePrecisionVariables.push_back (variableName_u);

  map <string, SgVariableDeclaration *> localDeclarations;

  for (vector <string>::iterator it = doublePrecisionVariables.begin (); it
      != doublePrecisionVariables.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getDoublePrecisionFloat (), subroutineScope);

    localDeclarations.insert (make_pair (*it, variableDeclaration));
  }

  /*
   * ======================================================
   * Build references to local variables and the constants
   * as they are needed in the following expressions and
   * statements
   * ======================================================
   */

  map <string, SgVarRefExp *> variableReferences;

  for (map <string, SgVariableDeclaration *>::iterator it =
      localDeclarations.begin (); it != localDeclarations.end (); ++it)
  {
    SgVarRefExp * variableReference = buildVarRefExp (it->second);

    variableReferences.insert (make_pair (it->first, variableReference));
  }

  for (map <string, SgVariableDeclaration *>::iterator it =
      constantDeclarations.begin (); it != constantDeclarations.end (); ++it)
  {
    SgVarRefExp * variableReference = buildVarRefExp (it->second);

    variableReferences.insert (make_pair (it->first, variableReference));
  }

  /*
   * ======================================================
   * Create the expressions and statements which initialise
   * the constants
   * ======================================================
   */

  /*
   * ======================================================
   * New statement: gam = 1.4
   * ======================================================
   */

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      constantDeclarations[variableNamePrefix + variableNameSuffix_gam],
      buildFloatVal (1.4), subroutineScope);

  /*
   * ======================================================
   * New statement: gm1 = 1.4 - 1.0
   * ======================================================
   */

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      constantDeclarations[variableNamePrefix + variableNameSuffix_gm1],
      buildSubtractOp (buildFloatVal (1.4), buildFloatVal (1.0)),
      subroutineScope);

  /*
   * ======================================================
   * New statement: cfl = 0.9
   * ======================================================
   */

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      constantDeclarations[variableNamePrefix + variableNameSuffix_cfl],
      buildFloatVal (0.9), subroutineScope);

  /*
   * ======================================================
   * New statement: eps = 0.05
   * ======================================================
   */

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      constantDeclarations[variableNamePrefix + variableNameSuffix_eps],
      buildFloatVal (0.05), subroutineScope);

  /*
   * ======================================================
   * New statement: mach = 0.4
   * ======================================================
   */

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      constantDeclarations[variableNamePrefix + variableNameSuffix_mach],
      buildFloatVal (0.4), subroutineScope);

  /*
   * ======================================================
   * New statement: alpha = 3.0 * atan(1.0) / 45.0
   * ======================================================
   */

  SgFunctionSymbol * atanFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("atan", subroutineScope);

  SgExprListExp * atanActualParameters = buildExprListExp (buildFloatVal (1.0));

  SgFunctionCallExp * atanFunctionCall = buildFunctionCallExp (
      atanFunctionSymbol, atanActualParameters);

  SgMultiplyOp * alphaMultiplyExpression = buildMultiplyOp (
      buildFloatVal (3.0), atanFunctionCall);

  SgDivideOp * alphaDivideExpression = buildDivideOp (alphaMultiplyExpression,
      buildFloatVal (4.5));

  SgExpression * alphaAssignmentExpression = buildAssignOp (
      variableReferences[variableNamePrefix + variableNameSuffix_alpha],
      alphaDivideExpression);

  appendStatement (buildExprStatement (alphaAssignmentExpression),
      subroutineScope);

  /*
   * ======================================================
   * New statement: p = 1.0
   * ======================================================
   */

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      localDeclarations[variableName_p], buildFloatVal (1.0), subroutineScope);

  /*
   * ======================================================
   * New statement: r = 1.0
   * ======================================================
   */

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      localDeclarations[variableName_r], buildFloatVal (1.0), subroutineScope);

  /*
   * ======================================================
   * New statement: u = sqrt(gam * p / r) * mach
   * ======================================================
   */

  SgFunctionSymbol * sqrtFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("sqrt", subroutineScope);

  SgMultiplyOp * sqrtMultiplyExpression = buildMultiplyOp (
      variableReferences[variableNamePrefix + variableNameSuffix_gam],
      variableReferences[variableName_p]);

  SgDivideOp * sqrtDivideExpression = buildDivideOp (sqrtMultiplyExpression,
      variableReferences[variableName_r]);

  SgExprListExp * sqrtActualParameters =
      buildExprListExp (sqrtDivideExpression);

  SgFunctionCallExp * sqrtFunctionCall = buildFunctionCallExp (
      sqrtFunctionSymbol, sqrtActualParameters);

  SgMultiplyOp * uMultiplyExpression = buildMultiplyOp (sqrtFunctionCall,
      variableReferences[variableNamePrefix + variableNameSuffix_mach]);

  SgExpression * uAssignmentExpression = buildAssignOp (
      variableReferences[variableName_u], uMultiplyExpression);

  appendStatement (buildExprStatement (uAssignmentExpression), subroutineScope);

  /*
   * ======================================================
   * New statement: e = p / (r * gm1) + 0.5 * u * u
   * ======================================================
   */

  SgMultiplyOp * eRHSOfDivideExpression = buildMultiplyOp (
      variableReferences[variableName_r], variableReferences[variableNamePrefix
          + variableNameSuffix_gm1]);

  SgDivideOp * eLHSOfAdditionExpression = buildDivideOp (
      variableReferences[variableName_p], eRHSOfDivideExpression);

  SgMultiplyOp * eRHSOfAdditionExpression = buildMultiplyOp (
      buildFloatVal (0.5), buildMultiplyOp (variableReferences[variableName_u],
          variableReferences[variableName_u]));

  SgAddOp * eAdditionExpression = buildAddOp (eLHSOfAdditionExpression,
      eRHSOfAdditionExpression);

  SgExpression * eAssignmentExpression = buildAssignOp (
      variableReferences[variableName_e], eAdditionExpression);

  appendStatement (buildExprStatement (eAssignmentExpression), subroutineScope);

  /*
   * ======================================================
   * New statement: qinf(1) = r
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableReferences[variableNamePrefix + variableNameSuffix_qinf],
      buildIntVal (1));

  SgExpression * assignmentExpression1 = buildAssignOp (arrayExpression1,
      variableReferences[variableName_r]);

  appendStatement (buildExprStatement (assignmentExpression1), subroutineScope);

  /*
   * ======================================================
   * New statement: qinf(2) = r * u
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableReferences[variableNamePrefix + variableNameSuffix_qinf],
      buildIntVal (2));

  SgExpression * multiplyExpression2 = buildMultiplyOp (
      variableReferences[variableName_r], variableReferences[variableName_u]);

  SgExpression * assignmentExpression2 = buildAssignOp (arrayExpression2,
      multiplyExpression2);

  appendStatement (buildExprStatement (assignmentExpression2), subroutineScope);

  /*
   * ======================================================
   * New statement: qinf(3) = 0.0
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
      variableReferences[variableNamePrefix + variableNameSuffix_qinf],
      buildIntVal (3));

  SgExpression * assignmentExpression3 = buildAssignOp (arrayExpression3,
      buildFloatVal (0.0));

  appendStatement (buildExprStatement (assignmentExpression3), subroutineScope);

  /*
   * ======================================================
   * New statement: qinf(4) = r * e
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (
      variableReferences[variableNamePrefix + variableNameSuffix_qinf],
      buildIntVal (4));

  SgExpression * multiplyExpression4 = buildMultiplyOp (
      variableReferences[variableName_r], variableReferences[variableName_e]);

  SgExpression * assignmentExpression4 = buildAssignOp (arrayExpression4,
      multiplyExpression4);

  appendStatement (buildExprStatement (assignmentExpression4), subroutineScope);
}

void
InitialiseConstantsSubroutine::declareConstants (SgScopeStatement * moduleScope)
{
  using std::string;
  using std::vector;

  /*
   * ======================================================
   * Double-precision float declarations
   * ======================================================
   */

  vector <string> doublePrecisionVariables;

  doublePrecisionVariables.push_back (variableNamePrefix
      + variableNameSuffix_air_const);
  doublePrecisionVariables.push_back (variableNamePrefix
      + variableNameSuffix_alpha);
  doublePrecisionVariables.push_back (variableNamePrefix
      + variableNameSuffix_cfl);
  doublePrecisionVariables.push_back (variableNamePrefix
      + variableNameSuffix_eps);
  doublePrecisionVariables.push_back (variableNamePrefix
      + variableNameSuffix_gam);
  doublePrecisionVariables.push_back (variableNamePrefix
      + variableNameSuffix_gm1);
  doublePrecisionVariables.push_back (variableNamePrefix
      + variableNameSuffix_mach);

  for (vector <string>::iterator it = doublePrecisionVariables.begin (); it
      != doublePrecisionVariables.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getDoublePrecisionFloat (), moduleScope);

    constantDeclarations.insert (make_pair (*it, variableDeclaration));
  }

  /*
   * ======================================================
   * Array of double-precision float declarations
   * ======================================================
   */

  vector <string> doublePrecisionArrayVariables;

  doublePrecisionArrayVariables.push_back (variableNamePrefix
      + variableNameSuffix_qinf);

  for (vector <string>::iterator it = doublePrecisionArrayVariables.begin (); it
      != doublePrecisionArrayVariables.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getDoublePrecisionFloat (), 1, 4),
            moduleScope);

    constantDeclarations.insert (make_pair (*it, variableDeclaration));
  }

  SgVariableDeclaration * iDecl = SageBuilder::buildVariableDeclaration ("i",
      FortranTypesBuilder::getFourByteInteger (), NULL, moduleScope);
  iDecl->get_declarationModifier ().get_accessModifier ().setUndefined ();

  SgDerivedTypeStatement * typeStatement =
      FortranStatementsAndExpressionsBuilder::buildTypeDeclaration ("newType",
          moduleScope);
  typeStatement->get_definition ()->append_member (iDecl);

  SageInterface::appendStatement (typeStatement, moduleScope);
}

InitialiseConstantsSubroutine::InitialiseConstantsSubroutine (
    std::string const & subroutineName, SgScopeStatement * moduleScope) :
  Subroutine (subroutineName + "_initialiseConstants")
{
  using SageBuilder::buildFunctionParameterList;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildProcedureHeaderStatement;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  /*
   * ======================================================
   * First declare the constants (before the subroutine initialising
   * them) to ensure their visibility
   * ======================================================
   */

  variableNamePrefix = subroutineName;

  declareConstants (moduleScope);

  formalParameters = buildFunctionParameterList ();

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  appendStatement (subroutineHeaderStatement, moduleScope);

  addTextForUnparser (subroutineHeaderStatement, "attributes(host) ",
      AstUnparseAttribute::e_before);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  generateSubroutine ();
}
