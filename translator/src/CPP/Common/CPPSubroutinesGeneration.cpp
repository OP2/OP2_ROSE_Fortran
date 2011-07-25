#include <CPPSubroutinesGeneration.h>
#include <FortranTypesBuilder.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
CPPSubroutinesGeneration::patchCallsToParallelLoops ()
{
}

SgSourceFile &
CPPSubroutinesGeneration::createSourceFile ()
{
  using SageBuilder::buildFile;
  using std::string;

  /*
   * ======================================================
   * To create a new file (to which the AST is later unparsed),
   * the API expects the name of an existing file and the
   * name of the output file. There is no input file corresponding
   * to our output file, therefore we first create a dummy
   * Fortran file. This will cause the unparser to generate
   * a warning about its internal stack state, but it can
   * suitably be ignored
   * ======================================================
   */
  string const inputFileName = "BLANK.cpp";

  FILE * inputFile = fopen (inputFileName.c_str (), "w+");
  if (inputFile != NULL)
  {
    Debug::getInstance ()->debugMessage ("Creating dummy source file '"
        + inputFileName + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    fclose (inputFile);
  }
  else
  {
    Debug::getInstance ()->errorMessage ("Could not create dummy C++ file '"
        + inputFileName + "'");
  }

  /*
   * ======================================================
   * Now generate the target backend file. The suffix of the
   * file changes according to the backend because the PGI
   * Fortran compiler requires the ".CUF" suffix to
   * correctly compile CUDA code
   * ======================================================
   */

  string outputFileName = "rose_" + fileSuffix;

  Debug::getInstance ()->debugMessage ("Generating file '" + outputFileName
      + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgSourceFile * sourceFile = isSgSourceFile (buildFile (inputFileName,
      outputFileName, NULL));

  /*
   * ======================================================
   * Later unparse C++ code
   * ======================================================
   */
  sourceFile->set_Cxx_only (true);

  /*
   * ======================================================
   * Store the file so it can be unparsed after AST
   * construction
   * ======================================================
   */
  generatedFiles.push_back (sourceFile);

  return *sourceFile;
}

void
CPPSubroutinesGeneration::visit (SgNode * node)
{
  using boost::starts_with;
  using std::pair;
  using std::string;

  switch (node->variantT ())
  {
    case V_SgFunctionCallExp:
    {
      /*
       * ======================================================
       * Function call found in AST
       * ======================================================
       */
      SgFunctionCallExp * functionCallExp = isSgFunctionCallExp (node);

      string const
          calleeName =
              functionCallExp->getAssociatedFunctionDeclaration ()->get_name ().getString ();

      /*
       * ======================================================
       * The prefix of all parallel loop calls in Fortran contains
       * 'OP_PAR_LOOP'. The suffix of the call, however, changes
       * depending on the number of expected parameters. Therefore,
       * any match of this prefix indicates a call of interest
       * to the translator
       * ======================================================
       */
      if (starts_with (calleeName, OP2::OP_PAR_LOOP))
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
           * build the target code and modify the calls in the original
           * Fortran source code
           * ======================================================
           */

          CPPParallelLoop * parallelLoop = new CPPParallelLoop (
              functionCallExp, userSubroutineName, declarations);

          parallelLoops[userSubroutineName] = parallelLoop;

          /*
           * ======================================================
           * Create the subroutines
           * ======================================================
           */

          CPPHostSubroutine * hostSubroutine = createSubroutines (parallelLoop,
              userSubroutineName);

          hostSubroutines[userSubroutineName] = hostSubroutine;
        }
      }

      break;
    }

    case V_SgSourceFile:
    {
      SgSourceFile * sourceFile = isSgSourceFile (node);

      Debug::getInstance ()->debugMessage ("Found file "
          + sourceFile->getFileName (), Debug::OUTER_LOOP_LEVEL, __FILE__,
          __LINE__);

      break;
    }

    default:
    {
      break;
    }
  }
}

CPPSubroutinesGeneration::CPPSubroutinesGeneration (
    CPPProgramDeclarationsAndDefinitions * declarations,
    std::string const & fileSuffix) :
  SubroutinesGeneration <CPPProgramDeclarationsAndDefinitions, CPPParallelLoop,
      CPPHostSubroutine> (declarations, fileSuffix)
{
  SgSourceFile & sourceFile = createSourceFile ();

  moduleScope = sourceFile.get_globalScope ();
}
