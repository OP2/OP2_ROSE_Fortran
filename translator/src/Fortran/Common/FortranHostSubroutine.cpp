#include <boost/algorithm/string/predicate.hpp>
#include <CommonNamespaces.h>
#include <Debug.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <FortranHostSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

void
FortranHostSubroutine::createPlanFunctionCallStatement ()
{
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageInterface::appendStatement;
  using std::map;
  using std::string;

  SgVarRefExp * parameter1 = buildVarRefExp (
      variableDeclarations[VariableNames::getUserSubroutineName ()]);

  SgVarRefExp * indexField = buildOpaqueVarRefExp ("index", subroutineScope);

  SgExpression * parameter2 = buildDotExp (buildVarRefExp (
      variableDeclarations[VariableNames::getOpSetName ()]), indexField);

  SgVarRefExp
      * parameter3 =
          buildVarRefExp (
              variableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::argsNumber]);

  SgVarRefExp
      * parameter4 =
          buildVarRefExp (
              variableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::args]);

  SgVarRefExp
      * parameter5 =
          buildVarRefExp (
              variableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::idxs]);

  SgVarRefExp
      * parameter6 =
          buildVarRefExp (
              variableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::maps]);

  SgVarRefExp
      * parameter7 =
          buildVarRefExp (
              variableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::accesses]);

  SgVarRefExp
      * parameter8 =
          buildVarRefExp (
              variableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::indsNumber]);

  SgVarRefExp
      * parameter9 =
          buildVarRefExp (
              variableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::inds]);

  SgExprListExp * actualParameters = buildExprListExp (parameter1, parameter2,
      parameter3, parameter4, parameter5, parameter6, parameter7, parameter8,
      parameter9);

  SgFunctionSymbol * cplanFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("cplan", subroutineScope);

  SgFunctionCallExp * cplanFunctionCall = buildFunctionCallExp (
      cplanFunctionSymbol, actualParameters);

  SgVarRefExp
      * cplanFunctionReturnReference =
          buildVarRefExp (
              variableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::planRet]);

  SgExpression * assignmentExpression = buildAssignOp (
      cplanFunctionReturnReference, cplanFunctionCall);

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);
}

void
FortranHostSubroutine::createInitialiseExecutionPlanStatements (std::vector <
    SgStatement *> & statements)
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildNotEqualOp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildExprStatement;
  using std::string;
  using std::vector;

  std::cout << "HERE 3\n";

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    SgVarRefExp * opDatFormalArgumentReference = buildVarRefExp (
        variableDeclarations[VariableNames::getOpDatName (i)]);

    SgExpression * indexField = buildDotExp (opDatFormalArgumentReference,
        buildOpaqueVarRefExp ("index", subroutineScope));

    SgVarRefExp
        * opDatArrayReference =
            buildVarRefExp (
                variableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::args]);

    SgExpression * indexExpression = buildIntVal (i);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        opDatArrayReference, indexExpression);

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, indexField);

    statements.push_back (assignmentStatement);
  }

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    SgVarRefExp * opIndexFormalArgumentReference = buildVarRefExp (
        variableDeclarations[VariableNames::getOpIndirectionName (i)]);

    SgVarRefExp
        * opIndirectionArrayReference =
            buildVarRefExp (
                variableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::idxs]);

    SgExpression * indexExpression = buildIntVal (i);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        opIndirectionArrayReference, indexExpression);

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, opIndexFormalArgumentReference);

    statements.push_back (assignmentStatement);
  }

  /*
   * ======================================================
   * The loop starts counting from 1
   * ======================================================
   */
  SgExpression
      * initializationExpression =
          buildAssignOp (
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::iterationCounter]),
              buildIntVal (1));

  /*
   * ======================================================
   * The loop stops counting at the number of OP_DAT argument
   * groups
   * ======================================================
   */
  SgExpression * upperBoundExpression = buildIntVal (
      parallelLoop->getNumberOf_OP_DAT_ArgumentGroups ());

  /*
   * ======================================================
   * The stride of the loop counter is 1
   * ======================================================
   */
  SgExpression * strideExpression = buildIntVal (1);

  /*
   * ======================================================
   * Build the body of the do-loop
   * ======================================================
   */
  SgExpression
      * arrayIndexExpression1 =
          buildPntrArrRefExp (
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::idxs]),
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::iterationCounter]));

  SgSubtractOp * subtractExpression1 = buildSubtractOp (arrayIndexExpression1,
      buildIntVal (1));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      arrayIndexExpression1, subtractExpression1);

  SgBasicBlock * ifBody = buildBasicBlock (assignmentStatement1);

  SgExpression * ifGuardExpression = buildNotEqualOp (arrayIndexExpression1,
      buildIntVal (-1));

  SgIfStmt * ifStatement =
      FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  SgBasicBlock * loopBody = buildBasicBlock (ifStatement);

  SgFortranDo * fortranDoStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initializationExpression, upperBoundExpression, strideExpression,
          loopBody);

  statements.push_back (fortranDoStatement);

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    SgVarRefExp * opMapFormalArgumentReference = buildVarRefExp (
        variableDeclarations[VariableNames::getOpMapName (i)]);

    SgExpression * indexField = buildDotExp (opMapFormalArgumentReference,
        buildOpaqueVarRefExp ("index", subroutineScope));

    SgVarRefExp
        * opMapArrayReference =
            buildVarRefExp (
                variableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::maps]);

    SgExpression * indexExpression = buildIntVal (i);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        opMapArrayReference, indexExpression);

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, indexField);

    statements.push_back (assignmentStatement);
  }

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    SgVarRefExp * opAccessFormalArgumentReference = buildVarRefExp (
        variableDeclarations[VariableNames::getOpAccessName (i)]);

    SgVarRefExp
        * opAccessArrayReference =
            buildVarRefExp (
                variableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::accesses]);

    SgExpression * indexExpression = buildIntVal (i);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        opAccessArrayReference, indexExpression);

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, opAccessFormalArgumentReference);

    statements.push_back (assignmentStatement);
  }

  /*
   * ======================================================
   * Set up a mapping between OP_DAT names and indirection
   * values. At the beginning everything is set to
   * INDS_UNDEFINED
   * ======================================================
   */

  int const INDS_UNDEFINED = -2;
  int const NO_INDS = -1;

  std::map <std::string, int> indsValuesPerOPDat;
  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    indsValuesPerOPDat[parallelLoop->get_OP_DAT_VariableName (i)]
        = INDS_UNDEFINED;
  }

  /*
   * ======================================================
   * Must start at the value defined by Mike Giles in his
   * implementation
   * ======================================================
   */
  unsigned int indValuesGenerator = 0;

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    SgVarRefExp
        * indsArrayReference =
            buildVarRefExp (
                variableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::inds]);

    SgExpression * indexExpression = buildIntVal (i);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        indsArrayReference, indexExpression);

    SgExpression * rhsExpression;

    if (parallelLoop->get_OP_MAP_Value (i) == DIRECT)
    {
      rhsExpression = buildIntVal (NO_INDS);

      indsValuesPerOPDat[parallelLoop->get_OP_DAT_VariableName (i)] = NO_INDS;
    }
    else
    {
      if (indsValuesPerOPDat[parallelLoop->get_OP_DAT_VariableName (i)]
          == INDS_UNDEFINED)
      {
        rhsExpression = buildIntVal (indValuesGenerator);

        indsValuesPerOPDat[parallelLoop->get_OP_DAT_VariableName (i)]
            = indValuesGenerator;

        indValuesGenerator++;
      }
      else
      {
        rhsExpression = buildIntVal (
            indsValuesPerOPDat[parallelLoop->get_OP_DAT_VariableName (i)]);
      }
    }

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, rhsExpression);

    statements.push_back (assignmentStatement);
  }

  SgExprStatement
      * assignmentStatement2 =
          buildAssignStatement (
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::argsNumber]),
              buildIntVal (parallelLoop->getNumberOf_OP_DAT_ArgumentGroups ()));

  statements.push_back (assignmentStatement2);

  SgExprStatement
      * assignmentStatement3 =
          buildAssignStatement (
              buildVarRefExp (
                  variableDeclarations[IndirectLoop::Fortran::HostSubroutine::VariableNames::indsNumber]),
              buildIntVal (
                  parallelLoop->getNumberOfDistinctIndirect_OP_DAT_Arguments ()));

  statements.push_back (assignmentStatement3);
}

SgStatement *
FortranHostSubroutine::createCToFortranPointerCall (SgExpression * parameter1,
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

void
FortranHostSubroutine::createlocalVariableDeclarations ()
{
  using boost::iequals;
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
      parallelLoop->getActualArguments ().begin (); it
      != parallelLoop->getActualArguments ().end (); ++it)
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

        string const & variableName = VariableNames::getUserSubroutineName ();

        variableDeclarations[variableName]
            = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, FortranTypesBuilder::getString (
                    userSubroutineName.length ()),

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
              string const & variableName = VariableNames::getOpSetName ();

              variableDeclarations[variableName]
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

              string const & variableName = VariableNames::getOpMapName (
                  OP_DAT_ArgumentGroup);

              variableDeclarations[variableName]
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

              string const & variableName = VariableNames::getOpDatName (
                  OP_DAT_ArgumentGroup);

              variableDeclarations[variableName]
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

            string const & variableName = VariableNames::getOpAccessName (
                OP_DAT_ArgumentGroup);

            variableDeclarations[variableName]
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

        string const & variableName = VariableNames::getOpIndirectionName (
            OP_DAT_ArgumentGroup);

        variableDeclarations[variableName]
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

FortranHostSubroutine::FortranHostSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    std::string const & kernelSubroutineName, ParallelLoop * parallelLoop) :
  FortranSubroutine (subroutineName + SubroutineNameSuffixes::hostSuffix)
{
  this->userSubroutineName = userSubroutineName;

  this->kernelSubroutineName = kernelSubroutineName;

  this->parallelLoop = parallelLoop;
}
