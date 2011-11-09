#include <FortranProgramDeclarationsAndDefinitions.h>
#include <FortranOP2Definitions.h>
#include <FortranParallelLoop.h>
#include <CommonNamespaces.h>
#include <Globals.h>
#include <Exceptions.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

void
FortranProgramDeclarationsAndDefinitions::setOpGblProperties (
    FortranParallelLoop * parallelLoop, std::string const & variableName,
    int OP_DAT_ArgumentGroup)
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

    parallelLoop->setOpDatDimension (OP_DAT_ArgumentGroup, 1);
  }
  else
  {
    Debug::getInstance ()->debugMessage ("'" + variableName
        + "' has been declared through OP_DECL_GBL", Debug::FUNCTION_LEVEL,
        __FILE__, __LINE__);

    parallelLoop->setOpDatDimension (OP_DAT_ArgumentGroup,
        opGblDeclaration->getDimension ());
  }

  parallelLoop->setUniqueOpDat (variableName);

  parallelLoop->setOpDatType (OP_DAT_ArgumentGroup,
      opGblDeclaration->getPrimitiveType ());

  parallelLoop->setOpDatVariableName (OP_DAT_ArgumentGroup, variableName);

  parallelLoop->setDuplicateOpDat (OP_DAT_ArgumentGroup, false);
}

void
FortranProgramDeclarationsAndDefinitions::setOpDatProperties (
    FortranParallelLoop * parallelLoop, std::string const & variableName,
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

    if (isSgArrayType (opDatDeclaration->getPrimitiveType ()) == NULL)
    {
      throw Exceptions::ParallelLoop::UnsupportedBaseTypeException ("OP_DAT '"
          + variableName + "' is not an array");
    }
  }
  else
  {
    parallelLoop->setDuplicateOpDat (OP_DAT_ArgumentGroup, true);
  }
}

void
FortranProgramDeclarationsAndDefinitions::setParallelLoopAccessDescriptor (
    FortranParallelLoop * parallelLoop, SgExprListExp * actualArguments,
    unsigned int OP_DAT_ArgumentGroup, unsigned int argumentPosition)
{
  using boost::iequals;
  using boost::lexical_cast;
  using std::string;

  SgVarRefExp * accessExpression = isSgVarRefExp (
      actualArguments->get_expressions ()[argumentPosition]);

  string const accessValue =
      accessExpression->get_symbol ()->get_name ().getString ();

  if (iequals (accessValue, OP2::OP_READ))
  {
    Debug::getInstance ()->debugMessage ("...READ access descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpAccessValue (OP_DAT_ArgumentGroup, READ_ACCESS);
  }
  else if (iequals (accessValue, OP2::OP_WRITE))
  {
    Debug::getInstance ()->debugMessage ("...WRITE access descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpAccessValue (OP_DAT_ArgumentGroup, WRITE_ACCESS);
  }
  else if (iequals (accessValue, OP2::OP_INC))
  {
    Debug::getInstance ()->debugMessage ("...INCREMENT access descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpAccessValue (OP_DAT_ArgumentGroup, INC_ACCESS);
  }
  else if (iequals (accessValue, OP2::OP_RW))
  {
    Debug::getInstance ()->debugMessage ("...READ/WRITE access descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpAccessValue (OP_DAT_ArgumentGroup, RW_ACCESS);
  }
  else if (iequals (accessValue, OP2::OP_MAX))
  {
    Debug::getInstance ()->debugMessage ("...MAXIMUM access descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpAccessValue (OP_DAT_ArgumentGroup, MAX_ACCESS);
  }
  else if (iequals (accessValue, OP2::OP_MIN))
  {
    Debug::getInstance ()->debugMessage ("...MINIMUM access descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpAccessValue (OP_DAT_ArgumentGroup, MIN_ACCESS);
  }
  else
  {
    throw Exceptions::ParallelLoop::UnknownAccessException (
        "Unknown access descriptor: '" + accessValue + "' for OP_DAT argument "
            + lexical_cast <string> (OP_DAT_ArgumentGroup));
  }
}

void
FortranProgramDeclarationsAndDefinitions::analyseParallelLoopArguments (
    FortranParallelLoop * parallelLoop, SgExprListExp * actualArguments)
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

  for (unsigned int offset = parallelLoop->NUMBER_OF_NON_OP_DAT_ARGUMENTS; offset
      < actualArguments->get_expressions ().size (); offset
      += parallelLoop->NUMBER_OF_ARGUMENTS_PER_OP_DAT)
  {
    /*
     * ======================================================
     * Get the OP_DAT variable name
     * ======================================================
     */

    unsigned int opDatArgumentPosition = offset
        + parallelLoop->POSITION_OF_OP_DAT;

    SgVarRefExp * opDatReference;

    if (isSgDotExp (actualArguments->get_expressions ()[opDatArgumentPosition])
        != NULL)
    {
      opDatReference
          = isSgVarRefExp (
              isSgDotExp (
                  actualArguments->get_expressions ()[opDatArgumentPosition])->get_rhs_operand ());
    }
    else
    {
      opDatReference = isSgVarRefExp (
          actualArguments->get_expressions ()[opDatArgumentPosition]);
    }

    string const opDatName =
        opDatReference->get_symbol ()->get_name ().getString ();

    unsigned int opMapArgumentPosition = offset
        + parallelLoop->POSITION_OF_MAPPING;

    /*
     * ======================================================
     * Get the OP_MAP name
     * ======================================================
     */

    SgVarRefExp * opMapReference;

    if (isSgDotExp (actualArguments->get_expressions ()[opMapArgumentPosition])
        != NULL)
    {
      opMapReference
          = isSgVarRefExp (
              isSgDotExp (
                  actualArguments->get_expressions ()[opMapArgumentPosition])->get_rhs_operand ());
    }
    else
    {
      opMapReference = isSgVarRefExp (
          actualArguments->get_expressions ()[opMapArgumentPosition]);
    }

    string const mappingValue =
        opMapReference->get_symbol ()->get_name ().getString ();

    Debug::getInstance ()->debugMessage ("OP_DAT '" + opDatName + "'",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    if (iequals (mappingValue, OP2::OP_GBL))
    {
      /*
       * ======================================================
       * OP_GBL signals that the OP_DAT is a global variable
       * ======================================================
       */
      Debug::getInstance ()->debugMessage ("...GLOBAL mapping descriptor",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      setOpGblProperties (parallelLoop, opDatName, OP_DAT_ArgumentGroup);

      parallelLoop->setOpMapValue (OP_DAT_ArgumentGroup, GLOBAL);
    }
    else
    {
      setOpDatProperties (parallelLoop, opDatName, OP_DAT_ArgumentGroup);

      if (iequals (mappingValue, OP2::OP_ID))
      {
        /*
         * ======================================================
         * OP_ID signals identity mapping and therefore direct
         * access to the data
         * ======================================================
         */
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

    setParallelLoopAccessDescriptor (parallelLoop, actualArguments,
        OP_DAT_ArgumentGroup, offset + parallelLoop->POSITION_OF_ACCESS);

    OP_DAT_ArgumentGroup++;
  }

  parallelLoop->setNumberOfOpDatArgumentGroups (OP_DAT_ArgumentGroup - 1);
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

        string const subroutineName =
            procedureHeaderStatement->get_name ().getString ();

        subroutinesInSourceCode[subroutineName] = procedureHeaderStatement;

        subroutineToFileName[subroutineName] = currentSourceFile;

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

        SgExprListExp * actualArguments = functionCallExp->get_args ();

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

          if (actualArguments->get_expressions ().size ()
              == FortranOpGblDefinition::getNumberOfExpectedArguments ())
          {
            opGblDeclaration = new FortranOpGblDefinition (actualArguments);
          }
          else
          {
            ROSE_ASSERT (actualArguments->get_expressions().size() == FortranOpGblScalarDefinition::getNumberOfExpectedArguments());

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

            FortranParallelLoop * parallelLoop = new FortranParallelLoop (
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

      case V_SgVariableDeclaration:
      {
        SgVariableDeclaration * variableDeclaration = isSgVariableDeclaration (
            node);

        for (SgInitializedNamePtrList::iterator it =
            variableDeclaration->get_variables ().begin (); it
            != variableDeclaration->get_variables ().end (); it++)
        {
          string const variableName = (*it)->get_name ().getString ();

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

FortranProgramDeclarationsAndDefinitions::FortranProgramDeclarationsAndDefinitions (
    SgProject * project)
{
  Debug::getInstance ()->debugMessage (
      "Obtaining declarations and definitions", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__ );

  traverseInputFiles (project, preorder);
}
