#include <boost/algorithm/string/predicate.hpp>
#include <FortranCUDAInitialiseConstantsSubroutine.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <RoseHelper.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

namespace ConstantSuffixes
{
  /*
   * ======================================================
   * Following are suffixes of symbolic constants needed
   * in each indirect loop. They are suffixes because
   * Fortran does not have a namespace concept, so we
   * prepend the name of the user device subroutine to make
   * them unique.
   *
   * Although these constants are used across all modules
   * implementing an OP_PAR_LOOP (for each indirect loop),
   * we cannot have a single module in which they are
   * included and initialised. This is because currently
   * the compilers generating the final binaries (e.g.
   * PGI) do not support calling functions/subroutines
   * outside of a single Fortran module.
   * ======================================================
   */

  std::string const separator = "_";

  std::string const air_const = "air_const";
  std::string const alpha = "alpha";
  std::string const cfl = "cfl";
  std::string const eps = "eps";
  std::string const gam = "gam";
  std::string const gm1 = "gm1";
  std::string const mach = "mach";
  std::string const qinf = "qinf";
}

void
FortranCUDAInitialiseConstantsSubroutine::declareConstants ()
{
  using boost::iequals;
  using std::string;
  using std::map;

  constantVariableNames[ConstantSuffixes::air_const] = variableNamePrefix
      + ConstantSuffixes::separator + ConstantSuffixes::air_const;

  constantVariableNames[ConstantSuffixes::alpha] = variableNamePrefix
      + ConstantSuffixes::separator + ConstantSuffixes::alpha;

  constantVariableNames[ConstantSuffixes::cfl] = variableNamePrefix
      + ConstantSuffixes::separator + ConstantSuffixes::cfl;

  constantVariableNames[ConstantSuffixes::eps] = variableNamePrefix
      + ConstantSuffixes::separator + ConstantSuffixes::eps;

  constantVariableNames[ConstantSuffixes::gam] = variableNamePrefix
      + ConstantSuffixes::separator + ConstantSuffixes::gam;

  constantVariableNames[ConstantSuffixes::gm1] = variableNamePrefix
      + ConstantSuffixes::separator + ConstantSuffixes::gm1;

  constantVariableNames[ConstantSuffixes::mach] = variableNamePrefix
      + ConstantSuffixes::separator + ConstantSuffixes::mach;

  constantVariableNames[ConstantSuffixes::qinf] = variableNamePrefix
      + ConstantSuffixes::separator + ConstantSuffixes::qinf;

  for (map <string, string>::const_iterator it = constantVariableNames.begin (); it
      != constantVariableNames.end (); ++it)
  {
    if (iequals (it->first, ConstantSuffixes::qinf) == false)
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

      variableDeclarations->add (it->second, variableDeclaration);
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

      variableDeclarations->add (it->second, variableDeclaration);
    }
  }
}

void
FortranCUDAInitialiseConstantsSubroutine::createStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildAssignStatement;
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

  /*
   * ======================================================
   * New statement: gam = 1.4
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (variableNamePrefix
          + ConstantSuffixes::separator + ConstantSuffixes::gam)),
      buildFloatVal (1.4));

  appendStatement (assignmentStatement1, subroutineScope);

  /*
   * ======================================================
   * New statement: gm1 = 1.4 - 1.0
   * ======================================================
   */

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (variableNamePrefix
          + ConstantSuffixes::separator + ConstantSuffixes::gm1)),
      buildFloatVal (0.4));

  appendStatement (assignmentStatement2, subroutineScope);

  /*
   * ======================================================
   * New statement: cfl = 0.9
   * ======================================================
   */

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (variableNamePrefix
          + ConstantSuffixes::separator + ConstantSuffixes::cfl)),
      buildFloatVal (0.9));

  appendStatement (assignmentStatement3, subroutineScope);

  /*
   * ======================================================
   * New statement: eps = 0.05
   * ======================================================
   */

  SgExprStatement * assignmentStatement4 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (variableNamePrefix
          + ConstantSuffixes::separator + ConstantSuffixes::eps)),
      buildFloatVal (0.05));

  appendStatement (assignmentStatement4, subroutineScope);

  /*
   * ======================================================
   * New statement: mach = 0.4
   * ======================================================
   */

  SgExprStatement * assignmentStatement5 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (variableNamePrefix
          + ConstantSuffixes::separator + ConstantSuffixes::mach)),
      buildFloatVal (0.4));

  appendStatement (assignmentStatement5, subroutineScope);

  /*
   * ======================================================
   * New statement: alpha = 3.0 * atan(1.0) / 45.0
   * ======================================================
   */

  SgExprStatement * assignmentStatement6 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (variableNamePrefix
          + ConstantSuffixes::separator + ConstantSuffixes::alpha)),
      buildFloatVal (0.052360));

  appendStatement (assignmentStatement6, subroutineScope);

  /*
   * ======================================================
   * New statement: qinf(1) = r
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression7 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (variableNamePrefix
          + ConstantSuffixes::separator + ConstantSuffixes::qinf)),
      buildIntVal (1));

  SgExprStatement * assignmentStatement7 = buildAssignStatement (
      arrayExpression7, buildFloatVal (1.000000));

  appendStatement (assignmentStatement7, subroutineScope);

  /*
   * ======================================================
   * New statement: qinf(2) = r * u
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression8 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (variableNamePrefix
          + ConstantSuffixes::separator + ConstantSuffixes::qinf)),
      buildIntVal (2));

  SgExprStatement * assignmentStatement8 = buildAssignStatement (
      arrayExpression8, buildFloatVal (0.473286));

  appendStatement (assignmentStatement8, subroutineScope);

  /*
   * ======================================================
   * New statement: qinf(3) = 0.0
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression9 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (variableNamePrefix
          + ConstantSuffixes::separator + ConstantSuffixes::qinf)),
      buildIntVal (3));

  SgExprStatement * assignmentStatement9 = buildAssignStatement (
      arrayExpression9, buildFloatVal (0.0));

  appendStatement (assignmentStatement9, subroutineScope);

  /*
   * ======================================================
   * New statement: qinf(4) = r * e
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression10 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (variableNamePrefix
          + ConstantSuffixes::separator + ConstantSuffixes::qinf)),
      buildIntVal (4));

  SgExprStatement * assignmentStatement10 = buildAssignStatement (
      arrayExpression10, buildFloatVal (2.612000));

  appendStatement (assignmentStatement10, subroutineScope);
}

void
FortranCUDAInitialiseConstantsSubroutine::createLocalVariableDeclarations ()
{

}

void
FortranCUDAInitialiseConstantsSubroutine::createFormalParameterDeclarations ()
{

}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

void
FortranCUDAInitialiseConstantsSubroutine::generateSubroutine ()
{
  using SageBuilder::buildProcedureHeaderStatement;
  using SageBuilder::buildVoidType;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  appendStatement (subroutineHeaderStatement, moduleScope);

  addTextForUnparser (subroutineHeaderStatement, "attributes(host) ",
      AstUnparseAttribute::e_before);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  createStatements ();
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
  Subroutine <SgProcedureHeaderStatement> (subroutineName
      + "_initialiseConstants"), variableNamePrefix (subroutineName),
      moduleScope (moduleScope)
{
  declareConstants ();
}
