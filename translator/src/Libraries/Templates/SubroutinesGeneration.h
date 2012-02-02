


/*  Open source copyright declaration based on BSD open source template:
 *  http://www.opensource.org/licenses/bsd-license.php
 * 
 * Copyright (c) 2011-2012, Adam Betts, Carlo Bertolli
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


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
