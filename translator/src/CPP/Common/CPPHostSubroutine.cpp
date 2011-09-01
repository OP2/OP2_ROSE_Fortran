#include <CPPHostSubroutine.h>
#include <CPPOpenCLReductionSubroutine.h>
#include <CPPStatementsAndExpressionsBuilder.h>

using namespace SageBuilder;


void
CPPHostSubroutine::createFormalParameterDeclarations ()
{
  using boost::iequals;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntType;
  using SageInterface::appendStatement;
  using std::vector;
  using std::string;
  using std::map;


  Debug::getInstance ()->debugMessage (
      "Creating host subroutine formal parameters", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Keep track of the OP_DAT group number
   * ======================================================
   */

  int OP_DAT_ArgumentGroup = 0;

  for (vector <SgExpression *>::const_iterator it = parallelLoop->getActualArguments().begin (); 
      it != parallelLoop->getActualArguments().end(); 
      ++it)
  {
    Debug::getInstance ()->debugMessage ("Argument type: "
        + (*it)->class_name (), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

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

        Debug::getInstance ()->debugMessage ("User subroutine '"
            + userSubroutineName + "'", Debug::OUTER_LOOP_LEVEL, __FILE__,
            __LINE__);

        SgVariableDeclaration
            * variableDeclaration =
                CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                    variableName, 
                    buildPointerType(buildCharType()), 
                    subroutineScope,
                    formalParameters);

        variableDeclarations->add (variableName, variableDeclaration);

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

            string const className = classReference->get_name().getString();

            if (iequals (className, OP2::OP_SET))
            {
              /*
               * ======================================================
               * Found an OP_SET argument
               * ======================================================
               */
              string const & variableName = VariableNames::getOpSetName ();

              SgVariableDeclaration
                  * variableDeclaration =
                      CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                          variableName, 
                          classReference, 
                          subroutineScope,
                          formalParameters);

              variableDeclarations->add (variableName, variableDeclaration);
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

              SgVariableDeclaration
                  * variableDeclaration =
                      CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                          variableName, 
                          classReference, 
                          subroutineScope,
                          formalParameters);

              variableDeclarations->add (variableName, variableDeclaration);
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

              SgVariableDeclaration
                  * variableDeclaration =
                      CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                          variableName, 
                          classReference, 
                          subroutineScope,
                          formalParameters);

              variableDeclarations->add (variableName, variableDeclaration);
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

            Debug::getInstance ()->debugMessage ("Access descriptor found",
                Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

            string const & variableName = VariableNames::getOpAccessName (
                OP_DAT_ArgumentGroup);

            SgVariableDeclaration
                * variableDeclaration =
                    CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                        variableName,
                        buildIntType(),
                        subroutineScope, 
                        formalParameters);

            variableDeclarations->add (variableName, variableDeclaration);

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

        SgVariableDeclaration
            * variableDeclaration =
                CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                    variableName, 
                    buildIntType(),
                    subroutineScope, 
                    formalParameters);

        variableDeclarations->add (variableName, variableDeclaration);

        break;
      }

      default:
      {
        break;
      }
    }
  }
  
}
