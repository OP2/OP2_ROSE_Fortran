#include <FortranProgramDeclarationsAndDefinitions.h>
#include <FortranOP2Definitions.h>
#include <CommonNamespaces.h>
#include <Globals.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranProgramDeclarationsAndDefinitions::handleOpGblDeclaration (
    FortranParallelLoop * parallelLoop, std::string const & variableName,
    int opDatArgumentGroup)
{
  OpGblDefinition * opGblDeclaration = getOpGblDefinition (variableName);

  FortranOpGblDefinition * fortanOpGBL =
      dynamic_cast <FortranOpGblDefinition *> (opGblDeclaration);

  if (fortanOpGBL == NULL)
  {
    Debug::getInstance ()->debugMessage ("'" + variableName
        + "' has been declared through OP_DECL_GBL (SCALAR)",
        Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    /*
     * ======================================================
     * Since this is a scalar, set the dimension to 1
     * ======================================================
     */

    parallelLoop->setOpDatDimension (opDatArgumentGroup, 1);
  }
  else
  {
    Debug::getInstance ()->debugMessage ("'" + variableName
        + "' has been declared through OP_DECL_GBL", Debug::FUNCTION_LEVEL,
        __FILE__, __LINE__);

    parallelLoop->setOpDatDimension (opDatArgumentGroup,
        opGblDeclaration->getDimension ());
  }

  parallelLoop->setUniqueOpDat (variableName);

  parallelLoop->setOpDatType (opDatArgumentGroup,
      opGblDeclaration->getPrimitiveType ());

  parallelLoop->setOpDatVariableName (opDatArgumentGroup, variableName);

  parallelLoop->setDuplicateOpDat (opDatArgumentGroup, false);
}

void
FortranProgramDeclarationsAndDefinitions::handleOpDatDeclaration (
    FortranParallelLoop * parallelLoop, std::string const & variableName,
    int opDatArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  OpDatDefinition * opDatDeclaration = getOpDatDefinition (variableName);

  Debug::getInstance ()->debugMessage ("'" + variableName
      + "' has been declared through OP_DECL_DAT", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  parallelLoop->setOpDatType (opDatArgumentGroup,
      opDatDeclaration->getPrimitiveType ());

  parallelLoop->setOpDatDimension (opDatArgumentGroup,
      opDatDeclaration->getDimension ());

  parallelLoop->setOpDatVariableName (opDatArgumentGroup, variableName);

  if (parallelLoop->isUniqueOpDat (variableName))
  {
    parallelLoop->setUniqueOpDat (variableName);

    parallelLoop->setDuplicateOpDat (opDatArgumentGroup, false);

    if (isSgArrayType (opDatDeclaration->getPrimitiveType ()) == NULL)
    {
      Debug::getInstance ()->errorMessage ("OP_DAT '" + variableName
          + "' is not an array", __FILE__, __LINE__);
    }
  }
  else
  {
    parallelLoop->setDuplicateOpDat (opDatArgumentGroup, true);
  }
}

void
FortranProgramDeclarationsAndDefinitions::retrieveOpDatDeclarations (
    FortranParallelLoop * parallelLoop, SgExpressionPtrList & actualArguments)
{
  using boost::iequals;
  using boost::lexical_cast;
  using std::find;
  using std::string;
  using std::vector;
  using std::map;

  Debug::getInstance ()->debugMessage ("Retrieving OP_DAT declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  int opDatArgumentGroup = 0;

  for (vector <SgExpression *>::iterator it = actualArguments.begin (); it
      != actualArguments.end (); ++it)
  {
    switch ((*it)->variantT ())
    {
      case V_SgVarRefExp:
      {
        /*
         * ======================================================
         * The argument of the OP_PAR_LOOP is a variable
         * reference (expression)
         * ======================================================
         */

        SgVarRefExp * variableReference = isSgVarRefExp (*it);

        switch (variableReference->get_type ()->variantT ())
        {
          case V_SgClassType:
          {
            SgClassType * classReference = isSgClassType (
                variableReference->get_type ());

            string const variableName =
                variableReference->get_symbol ()->get_name ().getString ();

            string const className = classReference->get_name ().getString ();

            if (iequals (className, OP2::OP_DAT))
            {
              opDatArgumentGroup++;

              /*
               * ======================================================
               * Found an OP_DAT variable. To retrieve its declaration,
               * we must check whether it was declared via OP_DECL_DAT
               * or OP_DECL_GBL
               * ======================================================
               */

              try
              {
                handleOpDatDeclaration (parallelLoop, variableName,
                    opDatArgumentGroup);
              }
              catch (const std::string &)
              {
                handleOpGblDeclaration (parallelLoop, variableName,
                    opDatArgumentGroup);
              }
            }

            else if (iequals (className, OP2::OP_MAP))
            {
              if (iequals (variableName, OP2::OP_ID))
              {
                /*
                 * ======================================================
                 * OP_ID signals identity mapping and therefore direct
                 * access to the data
                 * ======================================================
                 */
                Debug::getInstance ()->debugMessage (
                    "...DIRECT mapping descriptor", Debug::OUTER_LOOP_LEVEL,
                    __FILE__, __LINE__);

                parallelLoop->setOpMapValue (opDatArgumentGroup, DIRECT);
              }
              else
              {
                if (iequals (variableName, OP2::OP_GBL))
                {
                  /*
                   * ======================================================
                   * OP_GBL signals that the OP_DAT is a global variable
                   * ======================================================
                   */
                  Debug::getInstance ()->debugMessage (
                      "...GLOBAL mapping descriptor", Debug::OUTER_LOOP_LEVEL,
                      __FILE__, __LINE__);

                  parallelLoop->setOpMapValue (opDatArgumentGroup, GLOBAL);
                }

                else
                {
                  Debug::getInstance ()->debugMessage (
                      "...INDIRECT mapping descriptor",
                      Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

                  parallelLoop->setOpMapValue (opDatArgumentGroup, INDIRECT);
                }
              }
            }

            break;
          }

          case V_SgTypeInt:
          {
            string const variableName =
                variableReference->get_symbol ()->get_name ().getString ();

            if (iequals (variableName, OP2::OP_READ))
            {
              Debug::getInstance ()->debugMessage ("...READ access descriptor",
                  Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

              parallelLoop->setOpAccessValue (opDatArgumentGroup, READ_ACCESS);
            }
            else if (iequals (variableName, OP2::OP_WRITE))
            {
              Debug::getInstance ()->debugMessage (
                  "...WRITE access descriptor", Debug::OUTER_LOOP_LEVEL,
                  __FILE__, __LINE__);

              parallelLoop->setOpAccessValue (opDatArgumentGroup, WRITE_ACCESS);
            }
            else if (iequals (variableName, OP2::OP_INC))
            {
              Debug::getInstance ()->debugMessage (
                  "...INCREMENT access descriptor", Debug::OUTER_LOOP_LEVEL,
                  __FILE__, __LINE__);

              parallelLoop->setOpAccessValue (opDatArgumentGroup, INC_ACCESS);
            }
            else if (iequals (variableName, OP2::OP_RW))
            {
              Debug::getInstance ()->debugMessage (
                  "...READ/WRITE access descriptor", Debug::OUTER_LOOP_LEVEL,
                  __FILE__, __LINE__);

              parallelLoop->setOpAccessValue (opDatArgumentGroup, RW_ACCESS);
            }
            else if (iequals (variableName, OP2::OP_MAX))
            {
              Debug::getInstance ()->debugMessage (
                  "...MAXIMUM access descriptor", Debug::OUTER_LOOP_LEVEL,
                  __FILE__, __LINE__);

              parallelLoop->setOpAccessValue (opDatArgumentGroup, MAX_ACCESS);
            }
            else if (iequals (variableName, OP2::OP_MIN))
            {
              Debug::getInstance ()->debugMessage (
                  "...MINIMUM access descriptor", Debug::OUTER_LOOP_LEVEL,
                  __FILE__, __LINE__);

              parallelLoop->setOpAccessValue (opDatArgumentGroup, MIN_ACCESS);
            }
            else
            {
              Debug::getInstance ()->errorMessage (
                  "Unknown access descriptor: '" + variableName
                      + "' for OP_DAT argument #" + lexical_cast <string> (
                      opDatArgumentGroup), __FILE__, __LINE__);
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

void
FortranProgramDeclarationsAndDefinitions::visit (SgNode * node)
{
  using boost::filesystem::path;
  using boost::filesystem::system_complete;
  using boost::iequals;
  using boost::starts_with;
  using std::string;

  if (isSgSourceFile (node))
  {
    path p = system_complete (path (isSgSourceFile (node)->getFileName ()));

    currentSourceFile = p.filename ();

    Debug::getInstance ()->debugMessage ("Source file '" + currentSourceFile
        + "' detected", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__ );
  }
  else if (Globals::getInstance ()->isInputFile (currentSourceFile))
  {
    /*
     * ======================================================
     * Only process this portion of the AST if we recognise
     * this source file as one passed on the command line. In
     * Fortran, .rmod files are sometimes generated whose
     * traversal should be avoided
     * ======================================================
     */

    switch (node->variantT ())
    {
      case V_SgModuleStatement:
      {
        SgModuleStatement * moduleStatement = isSgModuleStatement (node);

        fileNameToModuleName[currentSourceFile]
            = moduleStatement->get_name ().getString ();

        Debug::getInstance ()->debugMessage ("Module '"
            + moduleStatement->get_name ().getString () + "' in file '"
            + currentSourceFile + "'", Debug::OUTER_LOOP_LEVEL, __FILE__,
            __LINE__ );

        break;
      }

      case V_SgProcedureHeaderStatement:
      {
        /*
         * ======================================================
         * We need to store all subroutine definitions since we
         * later have to copy and modify the user kernel subroutine
         * ======================================================
         */
        SgProcedureHeaderStatement * procedureHeaderStatement =
            isSgProcedureHeaderStatement (node);

        subroutinesInSourceCode.push_back (procedureHeaderStatement);

        subroutineToFileName[procedureHeaderStatement->get_name ().getString ()]
            = currentSourceFile;

        Debug::getInstance ()->debugMessage (
            "Found procedure header statement '"
                + procedureHeaderStatement->get_name ().getString ()
                + "' in file '" + currentSourceFile + "'",
            Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

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

        SgExpressionPtrList & actualArguments =
            functionCallExp->get_args ()->get_expressions ();

        string const
            calleeName =
                functionCallExp->getAssociatedFunctionSymbol ()->get_name ().getString ();

        Debug::getInstance ()->debugMessage ("Found function call '"
            + calleeName + "'", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

        if (iequals (calleeName, OP2::OP_DECL_SET))
        {
          /*
           * ======================================================
           * An OP_SET variable declared through an OP_DECL_SET call
           * ======================================================
           */

          FortranOpSetDefinition * opSetDeclaration =
              new FortranOpSetDefinition (actualArguments);

          OpSetDefinitions[opSetDeclaration->getVariableName ()]
              = opSetDeclaration;
        }
        else if (iequals (calleeName, OP2::OP_DECL_MAP))
        {
          /*
           * ======================================================
           * An OP_MAP variable declared through an OP_DECL_MAP call
           * ======================================================
           */

          FortranOpMapDefinition * opMapDeclaration =
              new FortranOpMapDefinition (actualArguments);

          OpMapDefinitions[opMapDeclaration->getVariableName ()]
              = opMapDeclaration;
        }
        else if (iequals (calleeName, OP2::OP_DECL_DAT))
        {
          /*
           * ======================================================
           * An OP_DAT variable declared through an OP_DECL_DAT call
           * ======================================================
           */

          FortranOpDatDefinition * opDatDeclaration =
              new FortranOpDatDefinition (actualArguments);

          if (isSgArrayType (opDatDeclaration->getPrimitiveType ()) == NULL)
          {
            Debug::getInstance ()->errorMessage ("OP_DAT variable '"
                + opDatDeclaration->getVariableName ()
                + "' is not an array type. Currently not supported.", __FILE__,
                __LINE__);
          }

          OpDatDefinitions[opDatDeclaration->getVariableName ()]
              = opDatDeclaration;
        }
        else if (iequals (calleeName, OP2::OP_DECL_GBL))
        {
          /*
           * ======================================================
           * An OP_DAT variable declared through an OP_DECL_GBL call
           * ======================================================
           */

          OpGblDefinition * opGblDeclaration;

          if (actualArguments.size ()
              == FortranOpGblDefinition::getNumberOfExpectedArguments ())
          {
            opGblDeclaration = new FortranOpGblDefinition (actualArguments);
          }
          else
          {
            ROSE_ASSERT (actualArguments.size() == FortranOpGblScalarDefinition::getNumberOfExpectedArguments());

            opGblDeclaration = new FortranOpGblScalarDefinition (
                actualArguments);
          }

          OpGblDefinitions[opGblDeclaration->getVariableName ()]
              = opGblDeclaration;
        }
        else if (iequals (calleeName, OP2::OP_DECL_CONST))
        {
          /*
           * ======================================================
           * A constant declared through an OP_DECL_CONST call
           * ======================================================
           */

          FortranOpConstDefinition * opConstDeclaration =
              new FortranOpConstDefinition (actualArguments);

          OpConstDefinitions[opConstDeclaration->getVariableName ()]
              = opConstDeclaration;
        }
        else if (starts_with (calleeName, OP2::OP_PAR_LOOP))
        {
          /*
           * ======================================================
           * The first argument to an 'OP_PAR_LOOP' call should be
           * a reference to the kernel function. Cast it and proceed,
           * otherwise throw an exception
           * ======================================================
           */

          SgExpressionPtrList & actualArguments =
              functionCallExp->get_args ()->get_expressions ();

          SgFunctionRefExp * functionRefExpression = isSgFunctionRefExp (
              actualArguments.front ());

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

            FortranParallelLoop * parallelLoop = new FortranParallelLoop (
                functionCallExp);

            parallelLoops[userSubroutineName] = parallelLoop;

            retrieveOpDatDeclarations (parallelLoop, actualArguments);

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

      case V_SgVariableDeclaration:
      {
        SgVariableDeclaration * variableDeclaration = isSgVariableDeclaration (
            node);

        for (SgInitializedNamePtrList::iterator it =
            variableDeclaration->get_variables ().begin (); it
            != variableDeclaration->get_variables ().end (); it++)
        {
          string const variableName = (*it)->get_name ().getString ();

          SgType * type =
              variableDeclaration->get_decl_item (variableName)->get_type ();

          declarations[variableName] = variableDeclaration;
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
 * ======================================================
 * Public functions
 * ======================================================
 */

FortranProgramDeclarationsAndDefinitions::FortranProgramDeclarationsAndDefinitions (
    SgProject * project)
{
  Debug::getInstance ()->debugMessage (
      "Obtaining declarations and definitions", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__ );

  traverseInputFiles (project, preorder);
}
