/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class stores all declarations and definitions of the program
 * under analysis
 */

#ifndef PROGRAM_DECLARATIONS_AND_DEFINTIONS_H
#define PROGRAM_DECLARATIONS_AND_DEFINTIONS_H

#include <OP2Definitions.h>
#include <boost/algorithm/string.hpp>
#include <Debug.h>

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
      std::vector <TSubroutineHeader *> subroutinesInSourceCode;

      /*
       * ======================================================
       * OP_DECL_SET declarations
       * ======================================================
       */
      std::vector <OpSetDefinition *> OpSetDefinitions;

      /*
       * ======================================================
       * OP_DECL_MAP declarations
       * ======================================================
       */
      std::vector <OpMapDefinition *> OpMapDefinitions;

      /*
       * ======================================================
       * OP_DECL_DAT declarations
       * ======================================================
       */
      std::vector <OpDatDefinition *> OpDatDefinitions;

      /*
       * ======================================================
       * OP_DECL_GBL declarations
       * ======================================================
       */
      std::vector <OpGblDefinition *> OpGblDefinitions;

      /*
       * ======================================================
       * OP_DECL_CONST declarations
       * ======================================================
       */
      std::vector <OpConstDefinition *> OpConstDefinitions;

    public:

      OpSetDefinition *
      getOpSetDefinition (std::string const & opSETSubroutineHeaderName)
          throw (std::string const &)
      {
        using boost::iequals;
        using std::vector;

        for (vector <OpSetDefinition *>::iterator it =
            OpSetDefinitions.begin (); it != OpSetDefinitions.end (); ++it)
        {
          if (iequals ((*it)->getVariableName (), opSETSubroutineHeaderName))
          {
            return *it;
          }
        }

        throw opSETSubroutineHeaderName;
      }

      OpMapDefinition *
      getOpMapDefinition (std::string const & opMAPName)
          throw (std::string const &)
      {
        using boost::iequals;
        using std::vector;

        for (vector <OpMapDefinition *>::iterator it =
            OpMapDefinitions.begin (); it != OpMapDefinitions.end (); ++it)
        {
          if (iequals ((*it)->getVariableName (), opMAPName))
          {
            return *it;
          }
        }

        throw opMAPName;
      }

      OpDatDefinition *
      getOpDatDefinition (std::string const & opDATSubroutineHeaderName)
          throw (std::string const &)
      {
        using boost::iequals;
        using std::vector;

        for (vector <OpDatDefinition *>::iterator it =
            OpDatDefinitions.begin (); it != OpDatDefinitions.end (); ++it)
        {
          if (iequals ((*it)->getVariableName (), opDATSubroutineHeaderName))
          {
            return *it;
          }
        }

        throw opDATSubroutineHeaderName;
      }

      OpGblDefinition *
      getOpGblDefinition (std::string const & opGBLName)
          throw (std::string const &)
      {
        using boost::iequals;
        using std::vector;

        for (vector <OpGblDefinition *>::iterator it =
            OpGblDefinitions.begin (); it != OpGblDefinitions.end (); ++it)
        {
          if (iequals ((*it)->getVariableName (), opGBLName))
          {
            return *it;
          }
        }

        throw opGBLName;
      }

      typename std::vector <TSubroutineHeader *>::const_iterator
      firstSubroutineInSourceCode ()
      {
        return subroutinesInSourceCode.begin ();
      }

      typename std::vector <TSubroutineHeader *>::const_iterator
      lastSubroutineInSourceCode ()
      {
        return subroutinesInSourceCode.end ();
      }
  };

#endif
