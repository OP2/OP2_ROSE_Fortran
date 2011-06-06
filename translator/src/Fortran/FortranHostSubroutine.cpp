#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <Debug.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <FortranHostSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

std::string
FortranHostSubroutine::getUserSubroutineFormalParameterName ()
{
  return "subroutineName";
}

std::string
FortranHostSubroutine::get_OP_SET_FormalParameterName ()
{
  return "set";
}

std::string
FortranHostSubroutine::get_OP_INDIRECTION_FormalParameterName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_INDIRECTION
      + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
FortranHostSubroutine::get_OP_MAP_FormalParameterName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_MAP
      + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
FortranHostSubroutine::get_OP_ACCESS_FormalParameterName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_ACCESS
      + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
FortranHostSubroutine::get_OP_DAT_FormalParameterName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
      + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

std::string
FortranHostSubroutine::get_OP_DAT_SizeVariableName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
      + lexical_cast <string> (OP_DAT_ArgumentGroup)
      + IndirectAndDirectLoop::Fortran::VariableSuffixes::Size;
}

std::string
FortranHostSubroutine::get_OP_DAT_DeviceVariableName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
      + lexical_cast <string> (OP_DAT_ArgumentGroup) + "Device";
}

std::string
FortranHostSubroutine::getCToFortranVariableName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return "cFortranPointer" + lexical_cast <string> (OP_DAT_ArgumentGroup);
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
FortranHostSubroutine::createFormalParameterDeclarations ()
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

        string const & variableName = getUserSubroutineFormalParameterName ();

        formalParameterDeclarations[variableName]
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

FortranHostSubroutine::FortranHostSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    std::string const & kernelSubroutineName, ParallelLoop * parallelLoop) :
  Subroutine (subroutineName + SubroutineNameSuffixes::hostSuffix)
{
  this->userSubroutineName = userSubroutineName;

  this->kernelSubroutineName = kernelSubroutineName;

  this->parallelLoop = parallelLoop;
}
