/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * TSubroutineHeaderhis class stores all OP2 declared variables and
 * subroutines encountered while parsing the user-supplied
 * source code
 */

#ifndef DECLARATSubroutineHeaderIONS_H
#define DECLARATSubroutineHeaderIONS_H

#include <OP2Variables.h>
#include <boost/algorithm/string.hpp>
#include <Debug.h>

template <typename TSubroutineHeader>
  class Declarations
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
      std::vector <OpSetDefinition *> OpSetDeclarations;

      /*
       * ======================================================
       * OP_DECL_MAP declarations
       * ======================================================
       */
      std::vector <OpMapDefinition *> OpMapDeclarations;

      /*
       * ======================================================
       * OP_DECL_DAT declarations
       * ======================================================
       */
      std::vector <OpDatDefinition *> OpDatDeclarations;

      /*
       * ======================================================
       * OP_DECL_GBL declarations
       * ======================================================
       */
      std::vector <OpGblDefinition *> OpGblDeclarations;

      /*
       * ======================================================
       * OP_DECL_CONST declarations
       * ======================================================
       */
      std::vector <OpConstDefinition *> OpConstDeclarations;

    public:

      OpSetDefinition *
      getOpSetDeclaration (std::string const & opSETSubroutineHeaderName)
          throw (std::string const &)
      {
        using boost::iequals;
        using std::vector;

        for (vector <OpSetDefinition *>::iterator it =
            OpSetDeclarations.begin (); it != OpSetDeclarations.end (); ++it)
        {
          if (iequals ((*it)->getVariableName (), opSETSubroutineHeaderName))
          {
            return *it;
          }
        }

        throw opSETSubroutineHeaderName;
      }

      OpMapDefinition *
      getOpMapDeclaration (std::string const & opMAPName)
          throw (std::string const &)
      {
        using boost::iequals;
        using std::vector;

        for (vector <OpMapDefinition *>::iterator it =
            OpMapDeclarations.begin (); it != OpMapDeclarations.end (); ++it)
        {
          if (iequals ((*it)->getVariableName (), opMAPName))
          {
            return *it;
          }
        }

        throw opMAPName;
      }

      OpDatDefinition *
      getOpDatDeclaration (std::string const & opDATSubroutineHeaderName)
          throw (std::string const &)
      {
        using boost::iequals;
        using std::vector;

        for (vector <OpDatDefinition *>::iterator it =
            OpDatDeclarations.begin (); it != OpDatDeclarations.end (); ++it)
        {
          if (iequals ((*it)->getVariableName (), opDATSubroutineHeaderName))
          {
            return *it;
          }
        }

        throw opDATSubroutineHeaderName;
      }

      OpGblDefinition *
      getOpGblDeclaration (std::string const & opGBLName)
          throw (std::string const &)
      {
        using boost::iequals;
        using std::vector;

        for (vector <OpGblDefinition *>::iterator it =
            OpGblDeclarations.begin (); it != OpGblDeclarations.end (); ++it)
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
