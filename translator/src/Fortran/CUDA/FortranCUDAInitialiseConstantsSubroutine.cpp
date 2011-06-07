#include <boost/algorithm/string/predicate.hpp>
#include <FortranCUDAInitialiseConstantsSubroutine.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <ROSEHelper.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranCUDAInitialiseConstantsSubroutine::declareConstants (
    SgScopeStatement * moduleScope)
{
  using boost::iequals;
  using std::string;
  using std::map;

  for (map <string, string>::const_iterator it = constantVariableNames.begin (); it
      != constantVariableNames.end (); ++it)
  {
    if (iequals (it->first, IndirectLoop::Fortran::ConstantSuffixes::qinf)
        == false)
    {
      /*
       * ======================================================
       * All the constants are double-precision floats except
       * for the 'qinf' variable
       * ======================================================
       */

      SgVariableDeclaration * variableDeclaration =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              it->second, FortranTypesBuilder::getDoublePrecisionFloat (),
              moduleScope, 1, CONSTANT);

      variableDeclarations[it->second] = variableDeclaration;
    }
    else
    {
      /*
       * ======================================================
       * The 'qinf' variable is an array of double-precision
       * floats
       * ======================================================
       */
      SgVariableDeclaration * variableDeclaration =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              it->second, FortranTypesBuilder::getArray_RankOne (
                  FortranTypesBuilder::getDoublePrecisionFloat (), 1, 4),
              moduleScope, 1, CONSTANT);

      variableDeclarations[it->second] = variableDeclaration;
    }
  }
}

void
FortranCUDAInitialiseConstantsSubroutine::createStatements ()
{

}

void
FortranCUDAInitialiseConstantsSubroutine::createLocalVariableDeclarations ()
{

}

void
FortranCUDAInitialiseConstantsSubroutine::createlocalVariableDeclarations ()
{

}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

void
FortranCUDAInitialiseConstantsSubroutine::generateSubroutine (
    SgScopeStatement * moduleScope)
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
  using SageBuilder::buildFunctionParameterList;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildProcedureHeaderStatement;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;
  using std::map;
  using std::string;
  using std::vector;

  /*
   * ======================================================
   * Build the subroutine declaration
   * ======================================================
   */

  formalParameters = buildFunctionParameterList ();

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  appendStatement (subroutineHeaderStatement, moduleScope);

  addTextForUnparser (subroutineHeaderStatement, "attributes(host) ",
      AstUnparseAttribute::e_before);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  /*
   * ======================================================
   * Build references to local variables and the constants
   * as they are needed in the following expressions and
   * statements
   * ======================================================
   */

  map <string, SgVarRefExp *> variableReferences;

  for (map <string, SgVariableDeclaration *>::iterator it =
      variableDeclarations.begin (); it != variableDeclarations.end (); ++it)
  {
    SgVarRefExp * variableReference = buildVarRefExp (it->second);

    variableReferences[it->first] = variableReference;
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
      variableDeclarations[variableNamePrefix
          + IndirectLoop::Fortran::ConstantSuffixes::constantsSeparator
          + IndirectLoop::Fortran::ConstantSuffixes::gam], buildFloatVal (1.4),
      subroutineScope);

  /*
   * ======================================================
   * New statement: gm1 = 1.4 - 1.0
   * ======================================================
   */

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      variableDeclarations[variableNamePrefix
          + IndirectLoop::Fortran::ConstantSuffixes::constantsSeparator
          + IndirectLoop::Fortran::ConstantSuffixes::gm1], buildFloatVal (0.4),
      subroutineScope);

  /*
   * ======================================================
   * New statement: cfl = 0.9
   * ======================================================
   */

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      variableDeclarations[variableNamePrefix
          + IndirectLoop::Fortran::ConstantSuffixes::constantsSeparator
          + IndirectLoop::Fortran::ConstantSuffixes::cfl], buildFloatVal (0.9),
      subroutineScope);

  /*
   * ======================================================
   * New statement: eps = 0.05
   * ======================================================
   */

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      variableDeclarations[variableNamePrefix
          + IndirectLoop::Fortran::ConstantSuffixes::constantsSeparator
          + IndirectLoop::Fortran::ConstantSuffixes::eps],
      buildFloatVal (0.05), subroutineScope);

  /*
   * ======================================================
   * New statement: mach = 0.4
   * ======================================================
   */

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      variableDeclarations[variableNamePrefix
          + IndirectLoop::Fortran::ConstantSuffixes::constantsSeparator
          + IndirectLoop::Fortran::ConstantSuffixes::mach],
      buildFloatVal (0.4), subroutineScope);

  /*
   * ======================================================
   * New statement: alpha = 3.0 * atan(1.0) / 45.0
   * ======================================================
   */

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      variableDeclarations[variableNamePrefix
          + IndirectLoop::Fortran::ConstantSuffixes::constantsSeparator
          + IndirectLoop::Fortran::ConstantSuffixes::alpha], buildFloatVal (
          0.052360), subroutineScope);

  /*
   * ======================================================
   * New statement: qinf(1) = r
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableReferences[variableNamePrefix
          + IndirectLoop::Fortran::ConstantSuffixes::constantsSeparator
          + IndirectLoop::Fortran::ConstantSuffixes::qinf], buildIntVal (1));

  SgExpression * assignmentExpression1 = buildAssignOp (arrayExpression1,
      buildFloatVal (1.000000));

  appendStatement (buildExprStatement (assignmentExpression1), subroutineScope);

  /*
   * ======================================================
   * New statement: qinf(2) = r * u
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableReferences[variableNamePrefix
          + IndirectLoop::Fortran::ConstantSuffixes::constantsSeparator
          + IndirectLoop::Fortran::ConstantSuffixes::qinf], buildIntVal (2));

  SgExpression * assignmentExpression2 = buildAssignOp (arrayExpression2,
      buildFloatVal (0.473286));

  appendStatement (buildExprStatement (assignmentExpression2), subroutineScope);

  /*
   * ======================================================
   * New statement: qinf(3) = 0.0
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
      variableReferences[variableNamePrefix
          + IndirectLoop::Fortran::ConstantSuffixes::constantsSeparator
          + IndirectLoop::Fortran::ConstantSuffixes::qinf], buildIntVal (3));

  SgExpression * assignmentExpression3 = buildAssignOp (arrayExpression3,
      buildFloatVal (0.0));

  appendStatement (buildExprStatement (assignmentExpression3), subroutineScope);

  /*
   * ======================================================
   * New statement: qinf(4) = r * e
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (
      variableReferences[variableNamePrefix
          + IndirectLoop::Fortran::ConstantSuffixes::constantsSeparator
          + IndirectLoop::Fortran::ConstantSuffixes::qinf], buildIntVal (4));

  SgExpression * assignmentExpression4 = buildAssignOp (arrayExpression4,
      buildFloatVal (2.612000));

  appendStatement (buildExprStatement (assignmentExpression4), subroutineScope);
}

std::map <std::string, std::string>::const_iterator
FortranCUDAInitialiseConstantsSubroutine::getFirstConstantName ()
{
  return constantVariableNames.begin ();
}

std::map <std::string, std::string>::const_iterator
FortranCUDAInitialiseConstantsSubroutine::getLastConstantName ()
{
  return constantVariableNames.end ();
}

FortranCUDAInitialiseConstantsSubroutine::FortranCUDAInitialiseConstantsSubroutine (
    std::string const & subroutineName, SgScopeStatement * moduleScope) :
  FortranSubroutine (subroutineName + "_initialiseConstants")
{
  variableNamePrefix = subroutineName;

  constantVariableNames[IndirectLoop::Fortran::ConstantSuffixes::air_const]
      = subroutineName
          + IndirectLoop::Fortran::ConstantSuffixes::constantsSeparator
          + IndirectLoop::Fortran::ConstantSuffixes::air_const;

  constantVariableNames[IndirectLoop::Fortran::ConstantSuffixes::alpha]
      = subroutineName
          + IndirectLoop::Fortran::ConstantSuffixes::constantsSeparator
          + IndirectLoop::Fortran::ConstantSuffixes::alpha;

  constantVariableNames[IndirectLoop::Fortran::ConstantSuffixes::cfl]
      = subroutineName
          + IndirectLoop::Fortran::ConstantSuffixes::constantsSeparator
          + IndirectLoop::Fortran::ConstantSuffixes::cfl;

  constantVariableNames[IndirectLoop::Fortran::ConstantSuffixes::eps]
      = subroutineName
          + IndirectLoop::Fortran::ConstantSuffixes::constantsSeparator
          + IndirectLoop::Fortran::ConstantSuffixes::eps;

  constantVariableNames[IndirectLoop::Fortran::ConstantSuffixes::gam]
      = subroutineName
          + IndirectLoop::Fortran::ConstantSuffixes::constantsSeparator
          + IndirectLoop::Fortran::ConstantSuffixes::gam;

  constantVariableNames[IndirectLoop::Fortran::ConstantSuffixes::gm1]
      = subroutineName
          + IndirectLoop::Fortran::ConstantSuffixes::constantsSeparator
          + IndirectLoop::Fortran::ConstantSuffixes::gm1;

  constantVariableNames[IndirectLoop::Fortran::ConstantSuffixes::mach]
      = subroutineName
          + IndirectLoop::Fortran::ConstantSuffixes::constantsSeparator
          + IndirectLoop::Fortran::ConstantSuffixes::mach;

  constantVariableNames[IndirectLoop::Fortran::ConstantSuffixes::qinf]
      = subroutineName
          + IndirectLoop::Fortran::ConstantSuffixes::constantsSeparator
          + IndirectLoop::Fortran::ConstantSuffixes::qinf;

  declareConstants (moduleScope);
}
