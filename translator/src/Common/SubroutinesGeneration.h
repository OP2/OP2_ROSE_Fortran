/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models the engine which controls subroutine
 * generation. It has 2 template parameters:
 *
 * 1) TDeclarations: the declarations found in the program
 * under analysis which are of interest to our compiler
 *
 * 2) THostSubroutine: the host subroutine for a particular
 * language
 */

#ifndef SUBROUTINES_GENERATION_H
#define SUBROUTINES_GENERATION_H

#include <Debug.h>

template <typename TDeclarations, typename THostSubroutine>
  class SubroutinesGeneration
  {
    protected:

      /*
       * ======================================================
       * The generated output files for each OP_PAR_LOOP
       * ======================================================
       */
      std::vector <SgSourceFile *> generatedFiles;

      /*
       * ======================================================
       * Used to retrieve base types and dimensions of OP_DAT
       * variables (and all other OP2 declarations, if needed)
       * and subroutines defined in the source files to be
       * processed
       * ======================================================
       */
      TDeclarations * declarations;

      /*
       * ======================================================
       * A mapping from a user subroutine name to its
       * generated host subroutine
       * ======================================================
       */
      std::map <std::string, THostSubroutine *> hostSubroutines;

      /*
       * ======================================================
       * The suffix of the file in which the new
       * subroutines are output
       * ======================================================
       */
      std::string fileSuffix;

      /*
       * ======================================================
       * The scope of the module into which the generated code
       * is output
       * ======================================================
       */
      SgScopeStatement * moduleScope;

    protected:

      /*
       * ======================================================
       * Generates output files for each OP_PAR_LOOP
       * ======================================================
       */
      void
      unparse ()
      {
        using std::vector;

        Debug::getInstance ()->debugMessage ("Generating new files",
            Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

        for (vector <SgSourceFile *>::const_iterator it =
            generatedFiles.begin (); it != generatedFiles.end (); ++it)
        {
          Debug::getInstance ()->debugMessage ("Unparsing to '"
              + (*it)->getFileName () + "'", Debug::FUNCTION_LEVEL, __FILE__,
              __LINE__);

          /*
           * ======================================================
           * Unparse the created files after checking consistency
           * of ASTs
           * ======================================================
           */
          SgProject * project = (*it)->get_project ();

          AstTests::runAllTests (project);

          project->unparse ();
        }
      }

      SubroutinesGeneration (
          TDeclarations * declarations,
          std::string const & fileSuffix) :
        declarations (declarations),
        fileSuffix (fileSuffix)
      {
      }
  };

#endif
