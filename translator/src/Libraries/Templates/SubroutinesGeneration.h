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

#include "ParallelLoop.h"
#include "Debug.h"
#include "Exceptions.h"
#include <vector>
#include <map>
#include <string>
#include <rose.h>

template <typename TDeclarations, typename THostSubroutine>
  class SubroutinesGeneration
  {
    protected:

      /*
       * ======================================================
       * The project to which the generated file should be
       * attached
       * ======================================================
       */

      SgProject * project;

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
       * The name of the file in which the new subroutines are
       * output
       * ======================================================
       */

      std::string newFileName;

      /*
       * ======================================================
       * The actual file internal to ROSE which contains the
       * generated code
       * ======================================================
       */

      SgSourceFile * sourceFile;

      /*
       * ======================================================
       * A mapping from a user subroutine name to its
       * generated host subroutine
       * ======================================================
       */

      std::map <std::string, THostSubroutine *> hostSubroutines;

      /*
       * ======================================================
       * The names of source files passed by the user which
       * are modified by our compiler
       * ======================================================
       */

      std::vector <std::string> dirtyFiles;

      /*
       * ======================================================
       * The scope of the module into which the generated code
       * is output
       * ======================================================
       */

      SgScopeStatement * moduleScope;

    private:

      void
      createSourceFile ()
      {
        using namespace SageBuilder;
        using std::string;

        Debug::getInstance ()->debugMessage ("Generating file '" + newFileName
            + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

        FILE * inputFile = fopen (newFileName.c_str (), "w+");
        if (inputFile != NULL)
        {
          Debug::getInstance ()->debugMessage ("Creating source file '"
              + newFileName + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

          fclose (inputFile);
        }
        else
        {
          throw Exceptions::CodeGeneration::FileCreationException (
              "Could not create source file '" + newFileName + "'");
        }

        sourceFile = isSgSourceFile (buildFile (newFileName, newFileName,
            project));
      }

    protected:

      void
      determineWhichInputFilesToBeUnparsed ()
      {
        using std::map;
        using std::string;
        using std::vector;

        for (map <string, ParallelLoop *>::const_iterator it =
            declarations->firstParallelLoop (); it
            != declarations->lastParallelLoop (); ++it)
        {
          ParallelLoop * parallelLoop = it->second;

          /*
           * ======================================================
           * All the files in which this parallel loop has been
           * found should be output in the code generation
           * ======================================================
           */

          for (vector <string>::const_iterator it =
              parallelLoop->getFirstFileName (); it
              != parallelLoop->getLastFileName (); ++it)
          {
            dirtyFiles.push_back (*it);
          }
        }
      }

      SubroutinesGeneration (SgProject * project, TDeclarations * declarations,
          std::string const & newFileName) :
        project (project), declarations (declarations), newFileName (
            newFileName)
      {
        createSourceFile ();
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
