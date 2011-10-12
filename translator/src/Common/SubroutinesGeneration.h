/*
 * This class models the engine which controls subroutine
 * generation. It has 2 template parameters:
 *
 * 1) TDeclarations: the declarations found in the program
 * under analysis which are of interest to our compiler
 *
 * 2) THostSubroutine: the type of the header statement for
 * a subroutine in the target language (e.g. C++, Fortran)
 */

#pragma once
#ifndef SUBROUTINES_GENERATION_H
#define SUBROUTINES_GENERATION_H

#include <vector>
#include <map>
#include <string>
#include <Debug.h>
#include <rose.h>

template <typename TDeclarations, typename THostSubroutine>
  class SubroutinesGeneration
  {
    protected:

      /*
       * ======================================================
       * The names of source files passed by the user which
       * are modified by our compiler
       * ======================================================
       */
      std::vector <std::string> dirtyFiles;

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
       * The name of the file in which the new subroutines are
       * output
       * ======================================================
       */
      std::string newFileName;

      /*
       * ======================================================
       * The scope of the module into which the generated code
       * is output
       * ======================================================
       */
      SgScopeStatement * moduleScope;

    protected:

      SubroutinesGeneration (TDeclarations * declarations,
          std::string const & newFileName) :
        declarations (declarations), newFileName (newFileName)
      {
      }

    public:

      std::string const &
      getFileName ()
      {
        return newFileName;
      }

      bool
      isDirty (std::string const & fileName)
      {
        using std::find;

        if (find (dirtyFiles.begin (), dirtyFiles.end (), fileName)
            != dirtyFiles.end ())
        {
          Debug::getInstance ()->debugMessage ("File '" + fileName
              + "' has been modified", Debug::FUNCTION_LEVEL, __FILE__,
              __LINE__);

          return true;
        }

        return false;
      }
  };

#endif
