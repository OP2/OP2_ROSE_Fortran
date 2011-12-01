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
       * Other declarations
       * ======================================================
       */

      std::vector <std::string> booleanDeclarations;

      std::vector <std::string> shortDeclarations;

      std::vector <std::string> integerDeclarations;

      std::vector <std::string> longDeclarations;

      std::vector <std::string> floatDeclarations;

      std::vector <std::string> doubleDeclarations;

      std::map <std::string, SgArrayType *> arrayDeclarations;

      std::map <std::string, SgPointerType *> pointerDeclarations;

      std::map <std::string, SgVariableDeclaration *> declarations;

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

      void
      handleBaseTypeDeclaration (SgType * type,
          std::string const & variableName)
      {
        if (isSgTypeBool (type) != NULL)
        {
          Debug::getInstance ()->debugMessage ("'" + variableName
              + "' is a boolean", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

          booleanDeclarations.push_back (variableName);
        }
        else if (isSgTypeShort (type) != NULL)
        {
          Debug::getInstance ()->debugMessage ("'" + variableName
              + "' is a short", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

          shortDeclarations.push_back (variableName);
        }
        else if (isSgTypeInt (type) != NULL)
        {
          Debug::getInstance ()->debugMessage ("'" + variableName
              + "' is an integer", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

          integerDeclarations.push_back (variableName);
        }
        else if (isSgTypeLong (type) != NULL)
        {
          Debug::getInstance ()->debugMessage ("'" + variableName
              + "' is a long", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

          longDeclarations.push_back (variableName);
        }
        else if (isSgTypeFloat (type) != NULL)
        {
          Debug::getInstance ()->debugMessage ("'" + variableName
              + "' is a float", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

          floatDeclarations.push_back (variableName);
        }
        else if (isSgTypeDouble (type) != NULL)
        {
          Debug::getInstance ()->debugMessage ("'" + variableName
              + "' is a double", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

          doubleDeclarations.push_back (variableName);
        }
        else if (isSgArrayType (type) != NULL)
        {
          Debug::getInstance ()->debugMessage ("'" + variableName
              + "' is an array", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

          arrayDeclarations[variableName] = isSgArrayType (type);
        }
        else if (isSgPointerType (type) != NULL)
        {
          Debug::getInstance ()->debugMessage ("'" + variableName
              + "' is a pointer", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

          pointerDeclarations[variableName] = isSgPointerType (type);
        }
      }

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

      bool
      isTypeBoolean (std::string const & variableName) const
      {
        using std::find;

        return find (booleanDeclarations.begin (), booleanDeclarations.end (),
            variableName) != booleanDeclarations.end ();
      }

      bool
      isTypeShort (std::string const & variableName) const
      {
        using std::find;

        return find (shortDeclarations.begin (), shortDeclarations.end (),
            variableName) != shortDeclarations.end ();
      }

      bool
      isTypeInteger (std::string const & variableName) const
      {
        using std::find;

        return find (integerDeclarations.begin (), integerDeclarations.end (),
            variableName) != integerDeclarations.end ();
      }

      bool
      isTypeLong (std::string const & variableName) const
      {
        using std::find;

        return find (longDeclarations.begin (), longDeclarations.end (),
            variableName) != longDeclarations.end ();
      }

      bool
      isTypeFloat (std::string const & variableName) const
      {
        using std::find;

        return find (floatDeclarations.begin (), floatDeclarations.end (),
            variableName) != floatDeclarations.end ();
      }

      bool
      isTypeDouble (std::string const & variableName) const
      {
        using std::find;

        return find (doubleDeclarations.begin (), doubleDeclarations.end (),
            variableName) != doubleDeclarations.end ();
      }

      bool
      isArrayType (std::string const & variableName) const
      {
        using std::find;

        return find (arrayDeclarations.begin (), arrayDeclarations.end (),
            variableName) != arrayDeclarations.end ();
      }

      bool
      isPointerType (std::string const & variableName) const
      {
        using std::find;

        return find (pointerDeclarations.begin (), pointerDeclarations.end (),
            variableName) != pointerDeclarations.end ();
      }

      SgType *
      getType (std::string const & variableName)
      {
        using namespace SageBuilder;

        if (isTypeShort (variableName))
        {
          return buildShortType ();
        }
        else if (isTypeInteger (variableName))
        {
          return buildIntType ();
        }
        else if (isTypeLong (variableName))
        {
          return buildLongType ();
        }
        else if (isTypeFloat (variableName))
        {
          return buildFloatType ();
        }
        else if (isTypeDouble (variableName))
        {
          return buildDoubleType ();
        }
        else if (arrayDeclarations.find (variableName)
            != arrayDeclarations.end ())
        {
          return arrayDeclarations[variableName];
        }
        else if (pointerDeclarations.find (variableName)
            != pointerDeclarations.end ())
        {
          return pointerDeclarations[variableName];
        }
      }

      SgVariableDeclaration *
      getOriginalDeclaration (std::string const & variableName)
      {
        return declarations[variableName];
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
