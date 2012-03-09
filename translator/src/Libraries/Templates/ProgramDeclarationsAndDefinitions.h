


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
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class stores all declarations and definitions of the program
 * under analysis
 */

#pragma once
#ifndef PROGRAM_DECLARATIONS_AND_DEFINTIONS_H
#define PROGRAM_DECLARATIONS_AND_DEFINTIONS_H

#include <rose.h>
#include <string>
#include <map>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <Debug.h>
#include <Exceptions.h>

class ParallelLoop;
class OpSetDefinition;
class OpSubSetDefinition;
class OpMapDefinition;
class OpDatDefinition;
class OpMatDefinition;
class OpSparsityDefinition;
class OpGblDefinition;
class OpConstDefinition;

template <typename TSubroutineHeader>
  class ProgramDeclarationsAndDefinitions
  {
    protected:

      /*
       * ======================================================
       * References to all subroutines found during the visit
       * of the input files
       * ======================================================
       */
      std::map <std::string, TSubroutineHeader *> subroutinesInSourceCode;

      /*
       * ======================================================
       * OP_DECL_SET declarations
       * ======================================================
       */
      std::map <std::string, OpSetDefinition *> OpSetDefinitions;

      /*
       * ======================================================
       * OP_DECL_SUBSET declarations
       * ======================================================
       */
      std::map <std::string, OpSubSetDefinition *> OpSubSetDefinitions;

      /*
       * ======================================================
       * OP_DECL_MAP declarations
       * ======================================================
       */
      std::map <std::string, OpMapDefinition *> OpMapDefinitions;

      /*
       * ======================================================
       * OP_DECL_DAT declarations
       * ======================================================
       */
      std::map <std::string, OpDatDefinition *> OpDatDefinitions;

      /*
       * ======================================================
       * OP_DECL_SPARSITY declarations
       * ======================================================
       */
      std::map <std::string, OpSparsityDefinition *> OpSparsityDefinitions;

      /*
       * ======================================================
       * OP_DECL_MAT declarations
       * ======================================================
       */
      std::map <std::string, OpMatDefinition *> OpMatDefinitions;

      /*
       * ======================================================
       * OP_DECL_GBL declarations
       * ======================================================
       */
      std::map <std::string, OpGblDefinition *> OpGblDefinitions;

      /*
       * ======================================================
       * OP_DECL_CONST declarations
       * ======================================================
       */
      std::map <std::string, OpConstDefinition *> OpConstDefinitions;

      /*
       * ======================================================
       * A mapping from a user subroutine name to our internal
       * representation of an OP_PAR_LOOP
       * ======================================================
       */
      std::map <std::string, ParallelLoop *> parallelLoops;

      /*
       * ======================================================
       * The source file AST currently being scanned
       * ======================================================
       */

      std::string currentSourceFile;

      /*
       * ======================================================
       * Source files encountered during AST scan
       * ======================================================
       */

      std::map <std::string, SgSourceFile *> givenSourceFiles;

    protected:

      bool
      isOpSet (std::string const & variableName) const
      {
        return OpSetDefinitions.find (variableName) != OpSetDefinitions.end ();
      }

      bool
      isOpMap (std::string const & variableName) const
      {
        return OpMapDefinitions.find (variableName) != OpMapDefinitions.end ();
      }

      bool
      isOpDat (std::string const & variableName) const
      {
        return OpDatDefinitions.find (variableName) != OpDatDefinitions.end ();
      }

      bool
      isOpSparsity (std::string const & variableName) const
      {
        return OpSparsityDefinitions.find (variableName) != OpSparsityDefinitions.end ();
      }

      bool
      isOpMat (std::string const & variableName) const
      {
        return OpMatDefinitions.find (variableName) != OpMatDefinitions.end ();
      }

      bool
      isOpGbl (std::string const & variableName) const
      {
        return OpGblDefinitions.find (variableName) != OpGblDefinitions.end ();
      }

      bool
      isOpConst (std::string const & variableName) const
      {
        return OpConstDefinitions.find (variableName)
            != OpConstDefinitions.end ();
      }

    public:

      OpSetDefinition *
      getOpSetDefinition (std::string const & opSetName)
          throw (std::string const &)
      {
        using std::map;
        using std::string;

        map <string, OpSetDefinition *>::const_iterator it =
            OpSetDefinitions.find (opSetName);

        if (it == OpSetDefinitions.end ())
        {
          throw opSetName;
        }
        else
        {
          return it->second;
        }
      }

      bool
      isOpSubSet (std::string const & variableName) const
      {
		  return OpSubSetDefinitions.find (variableName)
		  != OpSubSetDefinitions.end ();
      }

      OpSubSetDefinition *
      getOpSubSetDefinition (std::string const & opSubSetName)
          throw (std::string const &)
      {
        using std::map;
        using std::string;

        map <string, OpSubSetDefinition *>::const_iterator it =
            OpSubSetDefinitions.find (opSubSetName);

        if (it == OpSubSetDefinitions.end ())
        {
          throw opSubSetName;
        }
        else
        {
          return it->second;
        }
      }

      OpMapDefinition *
      getOpMapDefinition (std::string const & opMapName)
          throw (std::string const &)
      {
        using std::map;
        using std::string;

        map <string, OpMapDefinition *>::const_iterator it =
            OpMapDefinitions.find (opMapName);

        if (it == OpMapDefinitions.end ())
        {
          throw opMapName;
        }
        else
        {
          return it->second;
        }
      }

      OpDatDefinition *
      getOpDatDefinition (std::string const & opDatName)
          throw (std::string const &)
      {
        using std::map;
        using std::string;

        map <string, OpDatDefinition *>::const_iterator it =
            OpDatDefinitions.find (opDatName);

        if (it == OpDatDefinitions.end ())
        {
          throw opDatName;
        }
        else
        {
          return it->second;
        }
      }

      OpSparsityDefinition *
      getOpSparsityDefinition (std::string const & opSparsityName)
          throw (std::string const &)
      {
        using std::map;
        using std::string;

        map <string, OpSparsityDefinition *>::const_iterator it =
            OpSparsityDefinitions.find (opSparsityName);

        if (it == OpSparsityDefinitions.end ())
        {
          throw opSparsityName;
        }
        else
        {
          return it->second;
        }
      }

      OpMatDefinition *
      getOpMatDefinition (std::string const & opMatName)
          throw (std::string const &)
      {
        using std::map;
        using std::string;

        map <string, OpMatDefinition *>::const_iterator it =
            OpMatDefinitions.find (opMatName);

        if (it == OpMatDefinitions.end ())
        {
          throw opMatName;
        }
        else
        {
          return it->second;
        }
      }

      OpGblDefinition *
      getOpGblDefinition (std::string const & opGblName)
          throw (std::string const &)
      {
        using std::map;
        using std::string;
              
        map <string, OpGblDefinition *>::const_iterator it =
            OpGblDefinitions.find (opGblName);

        if (it == OpGblDefinitions.end ())
        {
          throw opGblName;
        }
        else
        {
          return it->second;
        }
      }

      OpConstDefinition *
      getOpConstDefinition (std::string const & opConstName)
          throw (std::string const &)
      {
        using std::map;
        using std::string;

        map <string, OpConstDefinition *>::const_iterator it =
            OpConstDefinitions.find (opConstName);

        if (it == OpConstDefinitions.end ())
        {
          throw opConstName;
        }
        else
        {
          return it->second;
        }
      }

      std::map <std::string, OpConstDefinition *>::const_iterator
      firstOpConstDefinition ()
      {
        return OpConstDefinitions.begin ();
      }

      std::map <std::string, OpConstDefinition *>::const_iterator
      lastOpConstDefinition ()
      {
        return OpConstDefinitions.end ();
      }
	  
	  std::map <std::string, OpSubSetDefinition*>::const_iterator
	  firstOpSubSetDefinition ()
	  {
		  return OpSubSetDefinitions.begin ();
	  }
	  
	  std::map <std::string, OpSubSetDefinition*>::const_iterator
	  lastOpSubSetDefinition ()
	  {
		  return OpSubSetDefinitions.end ();
	  }

	  std::map <std::string, OpMapDefinition*>::const_iterator
	  firstOpMapDefinition ()
	  {
		  return OpMapDefinitions.begin ();
	  }
	  
	  std::map <std::string, OpMapDefinition*>::const_iterator
	  lastOpMapDefinition ()
	  {
		  return OpMapDefinitions.end ();
	  }

      bool
      isOpConstDefinition (std::string const & variableName)
      {
        return OpConstDefinitions.count (variableName) > 0;
      }

      TSubroutineHeader *
      getSubroutine (std::string subroutineName)
      {
        using std::map;
        using std::string;

        typename map <string, TSubroutineHeader *>::const_iterator it =
            subroutinesInSourceCode.find (subroutineName);

        if (it == subroutinesInSourceCode.end ())
        {
          throw Exceptions::CodeGeneration::UnknownSubroutineException (
              "Unable to find subroutine '" + subroutineName + "'");
        }
        else
        {
          return it->second;
        }
      }

      SgType *
      getUserKernelFormalParameterType (unsigned int argumentPosition,
          std::string const subroutineName)
      {
        return subroutinesInSourceCode[subroutineName]->get_args ()[argumentPosition
            - 1]->get_type ();
      }

      typename std::map <std::string, ParallelLoop *>::const_iterator
      firstParallelLoop ()
      {
        return parallelLoops.begin ();
      }

      typename std::map <std::string, ParallelLoop *>::const_iterator
      lastParallelLoop ()
      {
        return parallelLoops.end ();
      }
	  
	  ParallelLoop*
	  getParallelLoop (std::string subroutineName)
	  {
		  using std::map;
		  using std::string;
		  
		  typename map <string, ParallelLoop *>::const_iterator it =
		  parallelLoops.find (subroutineName);
		  
		  if (it == parallelLoops.end ())
		  {
			  throw Exceptions::CodeGeneration::UnknownSubroutineException (
																			"Unable to find parallelLoop for subroutine '" + subroutineName + "'");
		  }
		  else
		  {
			  return it->second;
		  }
	  }

      SgSourceFile *
      getSourceFile (std::string const & fileName)
      {
        if (givenSourceFiles.find (fileName) == givenSourceFiles.end ())
        {
          throw Exceptions::ASTParsing::NoSourceFileException ("Cannot find '"
              + fileName + "'");
        }

        return givenSourceFiles[fileName];
      }
  };

#endif
