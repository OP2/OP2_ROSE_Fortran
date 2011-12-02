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
class OpMapDefinition;
class OpDatDefinition;
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
