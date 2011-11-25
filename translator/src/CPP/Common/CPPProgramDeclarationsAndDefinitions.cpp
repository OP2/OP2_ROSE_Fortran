#include "CPPProgramDeclarationsAndDefinitions.h"
#include "CPPOP2Definitions.h"
#include "CPPParallelLoop.h"
#include "Exceptions.h"
#include "OP2.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

void
CPPProgramDeclarationsAndDefinitions::setOpGblProperties (
    CPPParallelLoop * parallelLoop, std::string const & variableName,
    int OP_DAT_ArgumentGroup)
{

}

void
CPPProgramDeclarationsAndDefinitions::setOpDatProperties (
    CPPParallelLoop * parallelLoop, std::string const & variableName,
    int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  OpDatDefinition * opDatDeclaration = getOpDatDefinition (variableName);

  Debug::getInstance ()->debugMessage ("'" + variableName
      + "' has been declared through OP_DECL_DAT", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  parallelLoop->setOpDatType (OP_DAT_ArgumentGroup,
      opDatDeclaration->getPrimitiveType ());

  parallelLoop->setOpDatDimension (OP_DAT_ArgumentGroup,
      opDatDeclaration->getDimension ());

  parallelLoop->setOpDatVariableName (OP_DAT_ArgumentGroup, variableName);

  if (parallelLoop->isUniqueOpDat (variableName))
  {
    parallelLoop->setUniqueOpDat (variableName);

    parallelLoop->setDuplicateOpDat (OP_DAT_ArgumentGroup, false);

    if (isSgPointerType (opDatDeclaration->getPrimitiveType ()) == NULL)
    {
      throw new Exceptions::ParallelLoop::UnsupportedBaseTypeException (
          "OP_DAT '" + variableName + "' is not a pointer");
    }
  }
  else
  {
    parallelLoop->setDuplicateOpDat (OP_DAT_ArgumentGroup, true);
  }
}

void
CPPProgramDeclarationsAndDefinitions::setParallelLoopAccessDescriptor (
    CPPParallelLoop * parallelLoop, SgExprListExp * actualArguments,
    unsigned int OP_DAT_ArgumentGroup, unsigned int argumentPosition)
{
  using boost::iequals;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage ("Setting access descriptor for OP_DAT "
      + lexical_cast <string> (OP_DAT_ArgumentGroup), Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgEnumVal * accessExpression = isSgEnumVal (
      actualArguments->get_expressions ()[argumentPosition]);

  std::string accessDescriptorName =
      opAccessDescriptors[accessExpression->get_value ()];

  if (iequals (accessDescriptorName, OP2::OP_READ))
  {
    Debug::getInstance ()->debugMessage ("...READ access descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpAccessValue (OP_DAT_ArgumentGroup, READ_ACCESS);
  }
  else if (iequals (accessDescriptorName, OP2::OP_WRITE))
  {
    Debug::getInstance ()->debugMessage ("...WRITE access descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpAccessValue (OP_DAT_ArgumentGroup, WRITE_ACCESS);
  }
  else if (iequals (accessDescriptorName, OP2::OP_INC))
  {
    Debug::getInstance ()->debugMessage ("...INCREMENT access descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpAccessValue (OP_DAT_ArgumentGroup, INC_ACCESS);
  }
  else if (iequals (accessDescriptorName, OP2::OP_RW))
  {
    Debug::getInstance ()->debugMessage ("...READ/WRITE access descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpAccessValue (OP_DAT_ArgumentGroup, RW_ACCESS);
  }
  else if (iequals (accessDescriptorName, OP2::OP_MAX))
  {
    Debug::getInstance ()->debugMessage ("...MAXIMUM access descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpAccessValue (OP_DAT_ArgumentGroup, MAX_ACCESS);
  }
  else if (iequals (accessDescriptorName, OP2::OP_MIN))
  {
    Debug::getInstance ()->debugMessage ("...MINIMUM access descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpAccessValue (OP_DAT_ArgumentGroup, MIN_ACCESS);
  }
  else
  {
    throw Exceptions::ParallelLoop::UnknownAccessException (
        "Unknown access descriptor: '" + accessDescriptorName
            + "' for OP_DAT argument " + lexical_cast <string> (
            OP_DAT_ArgumentGroup));
  }
}

void
CPPProgramDeclarationsAndDefinitions::handleImperialOpGblArgument (
    CPPParallelLoop * parallelLoop, SgExprListExp * actualArguments,
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Setting properties of OP_GBL argument '" + lexical_cast <string> (
          OP_DAT_ArgumentGroup) + "' (Imperial API)", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgVarRefExp
      * opGblReference =
          isSgVarRefExp (
              isSgAddressOfOp (
                  actualArguments->get_expressions ()[CPPImperialOpArgGblCall::indexOpDat])->get_operand ());

  string const opGblName =
      opGblReference->get_symbol ()->get_name ().getString ();

  setOpGblProperties (parallelLoop, opGblName, OP_DAT_ArgumentGroup);

  setParallelLoopAccessDescriptor (parallelLoop, actualArguments,
      OP_DAT_ArgumentGroup, CPPImperialOpArgGblCall::indexAccessDescriptor);
}

void
CPPProgramDeclarationsAndDefinitions::handleOxfordOpGblArgument (
    CPPParallelLoop * parallelLoop, SgExprListExp * actualArguments,
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Setting properties of OP_GBL argument '" + lexical_cast <string> (
          OP_DAT_ArgumentGroup) + "' (Oxford API)", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgAddressOfOp * addressOfOp = isSgAddressOfOp (
      actualArguments->get_expressions ()[CPPOxfordOpArgGblCall::indexOpDat]);

  SgVarRefExp * opGblReference;

  if (addressOfOp == NULL)
  {
    opGblReference = isSgVarRefExp (
        actualArguments->get_expressions ()[CPPOxfordOpArgGblCall::indexOpDat]);
  }
  else
  {
    opGblReference = isSgVarRefExp (addressOfOp->get_operand ());
  }

  string const opGblName =
      opGblReference->get_symbol ()->get_name ().getString ();

  parallelLoop->setOpDatDimension (
      OP_DAT_ArgumentGroup,
      isSgIntVal (
          actualArguments->get_expressions ()[CPPOxfordOpArgGblCall::indexDimension])->get_value ());

  parallelLoop->setUniqueOpDat (opGblName);

  parallelLoop->setOpDatType (OP_DAT_ArgumentGroup,
      getUserKernelFormalParameterType (OP_DAT_ArgumentGroup,
          parallelLoop->getUserSubroutineName ()));

  parallelLoop->setOpDatVariableName (OP_DAT_ArgumentGroup, opGblName);

  parallelLoop->setDuplicateOpDat (OP_DAT_ArgumentGroup, false);

  setParallelLoopAccessDescriptor (parallelLoop, actualArguments,
      OP_DAT_ArgumentGroup, CPPOxfordOpArgGblCall::indexAccessDescriptor);
}

void
CPPProgramDeclarationsAndDefinitions::handleImperialOpDatArgument (
    CPPParallelLoop * parallelLoop, SgExprListExp * actualArguments,
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Setting properties of OP_DAT argument '" + lexical_cast <string> (
          OP_DAT_ArgumentGroup) + "' (Imperial API)", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgVarRefExp * opDatReference = isSgVarRefExp (
      actualArguments->get_expressions ()[CPPImperialOpArgDatCall::indexOpDat]);

  string const opDatName =
      opDatReference->get_symbol ()->get_name ().getString ();

  setOpDatProperties (parallelLoop, opDatName, OP_DAT_ArgumentGroup);

  setParallelLoopAccessDescriptor (parallelLoop, actualArguments,
      OP_DAT_ArgumentGroup, CPPImperialOpArgDatCall::indexAccessDescriptor);

  if (isSgVarRefExp (
      actualArguments->get_expressions ()[CPPImperialOpArgDatCall::indexOpMap])
      == NULL)
  {
    Debug::getInstance ()->debugMessage ("...DIRECT mapping descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpMapValue (OP_DAT_ArgumentGroup, DIRECT);
  }
  else
  {
    Debug::getInstance ()->debugMessage ("...INDIRECT mapping descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpMapValue (OP_DAT_ArgumentGroup, INDIRECT);
  }
}

void
CPPProgramDeclarationsAndDefinitions::handleOxfordOpDatArgument (
    CPPParallelLoop * parallelLoop, SgExprListExp * actualArguments,
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Setting properties of OP_DAT argument '" + lexical_cast <string> (
          OP_DAT_ArgumentGroup) + "' (Oxford API)", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgVarRefExp * opDatReference = isSgVarRefExp (
      actualArguments->get_expressions ()[CPPOxfordOpArgDatCall::indexOpDat]);

  string const opDatName =
      opDatReference->get_symbol ()->get_name ().getString ();

  setOpDatProperties (parallelLoop, opDatName, OP_DAT_ArgumentGroup);

  setParallelLoopAccessDescriptor (parallelLoop, actualArguments,
      OP_DAT_ArgumentGroup, CPPOxfordOpArgDatCall::indexAccessDescriptor);

  if (isSgVarRefExp (
      actualArguments->get_expressions ()[CPPImperialOpArgDatCall::indexOpMap])
      == NULL)
  {
    Debug::getInstance ()->debugMessage ("...DIRECT mapping descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpMapValue (OP_DAT_ArgumentGroup, DIRECT);
  }
  else
  {
    Debug::getInstance ()->debugMessage ("...INDIRECT mapping descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpMapValue (OP_DAT_ArgumentGroup, INDIRECT);
  }
}

void
CPPProgramDeclarationsAndDefinitions::analyseParallelLoopArguments (
    CPPParallelLoop * parallelLoop, SgExprListExp * actualArguments)
{
  using boost::iequals;
  using boost::lexical_cast;
  using std::find;
  using std::string;
  using std::vector;
  using std::map;

  Debug::getInstance ()->debugMessage (
      "Analysing OP_PAR_LOOP actual arguments", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  unsigned int OP_DAT_ArgumentGroup = 1;

  for (unsigned int argument = 0; argument
      < actualArguments->get_expressions ().size (); ++argument)
  {
    /*
     * ======================================================
     * Get the OP_ARG_DAT or OP_ARG_GBL function call in this
     * position
     * ======================================================
     */

    SgFunctionCallExp * opDatCall = isSgFunctionCallExp (
        actualArguments->get_expressions ()[argument]);

    if (opDatCall != NULL)
    {
      SgExprListExp * opDatActualArguments = opDatCall->get_args ();

      if (iequals (
          opDatCall->getAssociatedFunctionSymbol ()->get_name ().getString (),
          OP2::OP_ARG_DAT))
      {
        if (opDatActualArguments->get_expressions ().size ()
            == CPPImperialOpArgDatCall::getNumberOfExpectedArguments ())
        {
          handleImperialOpDatArgument (parallelLoop, opDatActualArguments,
              OP_DAT_ArgumentGroup);
        }
        else
        {
          handleOxfordOpDatArgument (parallelLoop, opDatActualArguments,
              OP_DAT_ArgumentGroup);
        }
      }
      else
      {
        parallelLoop->setOpMapValue (OP_DAT_ArgumentGroup, GLOBAL);

        if (opDatActualArguments->get_expressions ().size ()
            == CPPImperialOpArgGblCall::getNumberOfExpectedArguments ())
        {
          handleImperialOpGblArgument (parallelLoop, opDatActualArguments,
              OP_DAT_ArgumentGroup);
        }
        else
        {
          handleOxfordOpGblArgument (parallelLoop, opDatActualArguments,
              OP_DAT_ArgumentGroup);
        }
      }

      OP_DAT_ArgumentGroup++;
    }
  }

  parallelLoop->setNumberOfOpDatArgumentGroups (OP_DAT_ArgumentGroup - 1);
}

void
CPPProgramDeclarationsAndDefinitions::detectAndHandleOP2Definition (
    SgVariableDeclaration * variableDeclaration,
    std::string const variableName, SgTypedefType * typeDefinition)
{
  using boost::iequals;
  using std::string;

  string const typeName = typeDefinition->get_name ().getString ();

  SgAssignInitializer * assignmentInitializer = isSgAssignInitializer (
      variableDeclaration->get_decl_item (variableName)->get_initializer ());

  if (assignmentInitializer != NULL)
  {
    if (iequals (typeName, OP2::OP_SET))
    {
      Debug::getInstance ()->debugMessage ("OP_SET declaration call found",
          Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

      SgFunctionCallExp * functionCallExpression = isSgFunctionCallExp (
          assignmentInitializer->get_operand ());

      ROSE_ASSERT (functionCallExpression != NULL);

      OpSetDefinition * opSetDeclaration;

      if (functionCallExpression->get_args ()->get_expressions ().size ()
          == CPPImperialOpSetDefinition::getNumberOfExpectedArguments ())
      {
        opSetDeclaration = new CPPImperialOpSetDefinition (
            functionCallExpression->get_args (), variableName);
      }
      else
      {
        ROSE_ASSERT (functionCallExpression->get_args ()->get_expressions ().size ()
            == CPPOxfordOpSetDefinition::getNumberOfExpectedArguments ());

        opSetDeclaration = new CPPOxfordOpSetDefinition (
            functionCallExpression->get_args (), variableName);
      }

      OpSetDefinitions[opSetDeclaration->getVariableName ()] = opSetDeclaration;
    }
    else if (iequals (typeName, OP2::OP_MAP))
    {

      Debug::getInstance ()->debugMessage ("OP_MAP declaration call found",
          Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

      SgAssignInitializer * assignmentInitializer =
          isSgAssignInitializer (variableDeclaration->get_decl_item (
              variableName)->get_initializer ());

      ROSE_ASSERT (assignmentInitializer != NULL);

      SgFunctionCallExp * functionCallExpression = isSgFunctionCallExp (
          assignmentInitializer->get_operand ());

      ROSE_ASSERT (functionCallExpression != NULL);

      OpMapDefinition * opMapDeclaration;

      if (functionCallExpression->get_args ()->get_expressions ().size ()
          == CPPImperialOpSetDefinition::getNumberOfExpectedArguments ())
      {
        opMapDeclaration = new CPPImperialOpMapDefinition (
            functionCallExpression->get_args (), variableName);
      }
      else
      {
        ROSE_ASSERT (functionCallExpression->get_args ()->get_expressions ().size ()
            == CPPOxfordOpMapDefinition::getNumberOfExpectedArguments ());

        opMapDeclaration = new CPPOxfordOpMapDefinition (
            functionCallExpression->get_args (), variableName);
      }

      OpMapDefinitions[opMapDeclaration->getVariableName ()] = opMapDeclaration;
    }
    else if (iequals (typeName, OP2::OP_DAT))
    {
      Debug::getInstance ()->debugMessage ("OP_DAT declaration call found",
          Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

      SgAssignInitializer * assignmentInitializer =
          isSgAssignInitializer (variableDeclaration->get_decl_item (
              variableName)->get_initializer ());

      ROSE_ASSERT (assignmentInitializer != NULL);

      SgFunctionCallExp * functionCallExpression = isSgFunctionCallExp (
          assignmentInitializer->get_operand ());

      ROSE_ASSERT (functionCallExpression != NULL);

      OpDatDefinition * opDatDeclaration;

      if (functionCallExpression->get_args ()->get_expressions ().size ()
          == CPPImperialOpDatDefinition::getNumberOfExpectedArguments ())
      {
        opDatDeclaration = new CPPImperialOpDatDefinition (
            functionCallExpression->get_args (), variableName);
      }
      else
      {
        ROSE_ASSERT (functionCallExpression->get_args ()->get_expressions ().size ()
            == CPPOxfordOpDatDefinition::getNumberOfExpectedArguments ());

        opDatDeclaration = new CPPOxfordOpDatDefinition (
            functionCallExpression->get_args (), variableName);
      }

      OpDatDefinitions[opDatDeclaration->getVariableName ()] = opDatDeclaration;
    }
  }
}

void
CPPProgramDeclarationsAndDefinitions::visit (SgNode * node)
{
  using boost::filesystem::path;
  using boost::filesystem::system_complete;
  using boost::iequals;
  using std::string;

  switch (node->variantT ())
  {
    case V_SgSourceFile:
    {
      SgSourceFile * sourceFile = isSgSourceFile (node);

      path p = system_complete (path (sourceFile->getFileName ()));

      currentSourceFile = p.filename ();

      Debug::getInstance ()->debugMessage ("Source file '" + currentSourceFile
          + "' detected", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__ );

      givenSourceFiles[currentSourceFile] = sourceFile;

      break;
    }

    case V_SgVariableDeclaration:
    {
      SgVariableDeclaration * variableDeclaration = isSgVariableDeclaration (
          node);

      string const
          variableName =
              variableDeclaration->get_variables ().front ()->get_name ().getString ();

      SgType * type =
          variableDeclaration->get_decl_item (variableName)->get_type ();

      if (isSgTypedefType (type) != NULL)
      {
        detectAndHandleOP2Definition (variableDeclaration, variableName,
            isSgTypedefType (type));
      }

      break;
    }

    case V_SgFunctionDeclaration:
    {
      SgFunctionDeclaration * functionDeclaration = isSgFunctionDeclaration (
          node);

      string const functionName = functionDeclaration->get_name ().getString ();

      Debug::getInstance ()->debugMessage (
          "Found declaration: " + functionName, Debug::INNER_LOOP_LEVEL,
          __FILE__, __LINE__);

      subroutinesInSourceCode[functionName] = functionDeclaration;

      break;
    }

    case V_SgEnumDeclaration:
    {
      SgEnumDeclaration * enumDeclaration = isSgEnumDeclaration (node);

      Debug::getInstance ()->debugMessage ("Found type definition: "
          + enumDeclaration->get_name ().getString (), Debug::INNER_LOOP_LEVEL,
          __FILE__, __LINE__);

      if (iequals (OP2::OP_ACCESS, enumDeclaration->get_name ().getString ()))
      {
        unsigned int index = 0;

        for (std::vector <SgInitializedName *>::const_iterator it =
            enumDeclaration->get_enumerators ().begin (); it
            != enumDeclaration->get_enumerators ().end (); ++it)
        {
          std::string enumField = (*it)->get_name ().getString ();

          if (iequals (enumField, OP2::OP_READ))
          {
            opAccessDescriptors[index] = OP2::OP_READ;
          }
          else if (iequals (enumField, OP2::OP_RW))
          {
            opAccessDescriptors[index] = OP2::OP_RW;
          }
          else if (iequals (enumField, OP2::OP_WRITE))
          {
            opAccessDescriptors[index] = OP2::OP_WRITE;
          }
          else if (iequals (enumField, OP2::OP_MAX))
          {
            opAccessDescriptors[index] = OP2::OP_MAX;
          }
          else if (iequals (enumField, OP2::OP_MIN))
          {
            opAccessDescriptors[index] = OP2::OP_MIN;
          }
          else if (iequals (enumField, OP2::OP_INC))
          {
            opAccessDescriptors[index] = OP2::OP_INC;
          }
          else
          {
            throw Exceptions::ParallelLoop::UnknownAccessException (
                "Unhandled access descriptor '" + enumField + "'");
          }

          index++;
        }
      }

      break;
    }

    case V_SgFunctionCallExp:
    {
      /*
       * ======================================================
       * Function call found in the AST. Get its actual arguments
       * and the callee name
       * ======================================================
       */
      SgFunctionCallExp * functionCallExp = isSgFunctionCallExp (node);

      string const
          calleeName =
              functionCallExp->getAssociatedFunctionSymbol ()->get_name ().getString ();

      if (iequals (calleeName, OP2::OP_DECL_CONST))
      {
        Debug::getInstance ()->debugMessage ("OP_CONST declaration call found",
            Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

        OpConstDefinition * opConstDeclaration;

        if (functionCallExp->get_args ()->get_expressions ().size ()
            == CPPImperialOpConstDefinition::getNumberOfExpectedArguments ())
        {
          opConstDeclaration = new CPPImperialOpConstDefinition (
              functionCallExp->get_args ());
        }
        else
        {
          ROSE_ASSERT(functionCallExp->get_args()->get_expressions().size()
              == CPPOxfordOpConstDefinition::getNumberOfExpectedArguments());

          opConstDeclaration = new CPPOxfordOpConstDefinition (
              functionCallExp->get_args ());
        }

        OpConstDefinitions[opConstDeclaration->getVariableName ()]
            = opConstDeclaration;
      }
      else if (iequals (calleeName, OP2::OP_PAR_LOOP))
      {
        SgExprListExp * actualArguments = functionCallExp->get_args ();

        SgFunctionRefExp * functionRefExpression = isSgFunctionRefExp (
            actualArguments->get_expressions ().front ());

        ROSE_ASSERT (functionRefExpression != NULL);

        string const
            userSubroutineName =
                functionRefExpression->getAssociatedFunctionDeclaration ()->get_name ().getString ();

        Debug::getInstance ()->debugMessage ("Found '" + calleeName
            + "' with (host) user subroutine '" + userSubroutineName + "'",
            Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

        if (parallelLoops.find (userSubroutineName) == parallelLoops.end ())
        {
          /*
           * ======================================================
           * If this kernel has not been previously encountered then
           * build a new parallel loop representation
           * ======================================================
           */

          CPPParallelLoop * parallelLoop = new CPPParallelLoop (
              functionCallExp, currentSourceFile);

          parallelLoops[userSubroutineName] = parallelLoop;

          analyseParallelLoopArguments (parallelLoop, actualArguments);

          parallelLoop->checkArguments ();
        }
        else
        {
          Debug::getInstance ()->debugMessage ("Parallel loop for '"
              + userSubroutineName + "' already created",
              Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);
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

CPPProgramDeclarationsAndDefinitions::CPPProgramDeclarationsAndDefinitions (
    SgProject * project)
{
  traverse (project, preorder);
}
