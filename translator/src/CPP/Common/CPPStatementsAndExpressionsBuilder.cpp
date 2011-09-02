#include <CPPStatementsAndExpressionsBuilder.h>
//#include <CPPTypesBuilder.h>
#include <CommonNamespaces.h>
#include <RoseHelper.h>
#include <Debug.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
CPPStatementsAndExpressionsBuilder::setCPPAttributes (
    SgVariableDeclaration * variableDeclaration, int remainingArguments,
    va_list attributeArguments)
{
  /*
   * ======================================================
   * The variable declaration is always undefined
   * ======================================================
   */
  variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

  for (int i = 0; i < remainingArguments; ++i)
  {
    int attribute = va_arg(attributeArguments, int);

    //TODO: add custom OpenCL modifiers in ROSE source (where?)
    switch (attribute)
    {
      /*
       case OPENCL_LOCAL:
       variableDeclaration->get_declarationModifier().get_typeModifier().setOpenCLLocal();
       break;
       case OPENCL_GLOBAL:
       variableDeclaration->get_declarationModifier().get_typeModifier().setOpenCLGlobal();
       break;
       case OPENCL_CONSTANT:
       variableDeclaration->get_declarationModifier().get_typeModifier().setOpenCLConstant();
       break;
       */
      /*
       case ALLOCATABLE:
       {
       variableDeclaration->get_declarationModifier ().get_typeModifier ().setAllocatable ();
       break;
       }
       */

      case CONSTANT:
      {
        SageInterface::addTextForUnparser (variableDeclaration,
            OpenCL::CPP::constantModifier, AstUnparseAttribute::e_before);
        //variableDeclaration->get_declarationModifier ().get_typeModifier ().setConstant ();
        break;
      }

      case DEVICE:
      {
        SageInterface::addTextForUnparser (variableDeclaration,
            OpenCL::CPP::globalModifier, AstUnparseAttribute::e_before);
        //variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();
        break;
      }

      case SHARED:
      {
        SageInterface::addTextForUnparser (variableDeclaration,
            OpenCL::CPP::localModifier, AstUnparseAttribute::e_before);
        //variableDeclaration->get_declarationModifier ().get_typeModifier ().setAttribute( OpenCL::CPP::localModifier );
        break;
      }
        /*
         case VALUE:
         {
         variableDeclaration->get_declarationModifier ().get_typeModifier ().setValue ();
         break;
         }

         case INTENT_IN:
         {
         variableDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();
         break;
         }

         case INTENT_OUT:
         {
         variableDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_out ();
         break;
         }

         case INTENT_INOUT:
         {
         variableDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_inout ();
         break;
         }*/

      default:
      {
        Debug::getInstance ()->errorMessage ("Unhandled CPP attribute",
            __FILE__, __LINE__);
      }
    }
  }
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

SgVariableDeclaration *
CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
    std::string const & variableName, SgType * type, SgScopeStatement * scope,
    int remainingArguments, ...)
{
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;

  SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
      variableName, type, NULL, scope);

  appendStatement (variableDeclaration, scope);

  va_list CPPAttributes;

  va_start (CPPAttributes, remainingArguments);

  setCPPAttributes (variableDeclaration, remainingArguments, CPPAttributes);

  va_end (CPPAttributes);

  return variableDeclaration;
}

SgVariableDeclaration *
CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
    std::string const & variableName, SgType * type, SgScopeStatement * scope,
    SgFunctionParameterList * formalParameters, int remainingArguments, ...)
{
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;

  SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
      variableName, type, NULL, scope);

  formalParameters->append_arg (
      *(variableDeclaration->get_variables ().begin ()));

  appendStatement (variableDeclaration, scope);

  va_list CPPAttributes;

  va_start (CPPAttributes, remainingArguments);

  setCPPAttributes (variableDeclaration, remainingArguments, CPPAttributes);

  va_end (CPPAttributes);

  return variableDeclaration;
}
