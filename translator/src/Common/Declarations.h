/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class stores all OP2 declared variables and
 * subroutines encountered while parsing the user-supplied
 * source code
 */

#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include <OP2Variables.h>
#include <boost/algorithm/string.hpp>
#include <Debug.h>

template <typename T>
  class Declarations
  {
    protected:

      /*
       * ======================================================
       * References to all subroutines found during the visit
       * of the input files
       * ======================================================
       */
      std::vector <T *> subroutinesInSourceCode;

      /*
       * ======================================================
       * Actual arguments passed to every OP_DECL_SET in the
       * program
       * ======================================================
       */
      std::vector <OpSetDeclaration *> OpSetDeclarations;

      /*
       * ======================================================
       * Actual arguments passed to every OP_DECL_MAP in the
       * program
       * ======================================================
       */
      std::vector <OpMapDeclaration *> OpMapDeclarations;

      /*
       * ======================================================
       * Actual arguments passed to every OP_DECL_DAT in the
       * program
       * ======================================================
       */
      std::vector <OpDatDeclaration *> OpDatDeclarations;

      /*
       * ======================================================
       * Actual arguments passed to every OP_DECL_GBL in the
       * program
       * ======================================================
       */
      std::vector <OpGblDeclaration *> OpGblDeclarations;

    public:

      OpSetDeclaration *
      get_OP_SET_Declaration (std::string const & opSETName)
          throw (std::string const &)
      {
        using boost::iequals;
        using std::vector;

        for (vector <OpSetDeclaration *>::iterator it =
            OpSetDeclarations.begin (); it != OpSetDeclarations.end (); ++it)
        {
          if (iequals ((*it)->getVariableName (), opSETName))
          {
            return *it;
          }
        }

        throw opSETName;
      }

      OpMapDeclaration *
      get_OP_MAP_Declaration (std::string const & opMAPName)
          throw (std::string const &)
      {
        using boost::iequals;
        using std::vector;

        for (vector <OpMapDeclaration *>::iterator it =
            OpMapDeclarations.begin (); it != OpMapDeclarations.end (); ++it)
        {
          if (iequals ((*it)->getVariableName (), opMAPName))
          {
            return *it;
          }
        }

        throw opMAPName;
      }

      OpDatDeclaration *
      get_OP_DAT_Declaration (std::string const & opDATName)
          throw (std::string const &)
      {
        using boost::iequals;
        using std::vector;

        for (vector <OpDatDeclaration *>::iterator it =
            OpDatDeclarations.begin (); it != OpDatDeclarations.end (); ++it)
        {
          if (iequals ((*it)->getVariableName (), opDATName))
          {
            return *it;
          }
        }

        throw opDATName;
      }

      OpGblDeclaration *
      get_OP_GBL_Declaration (std::string const & opGBLName)
          throw (std::string const &)
      {
        using boost::iequals;
        using std::vector;

        for (vector <OpGblDeclaration *>::iterator it =
            OpGblDeclarations.begin (); it != OpGblDeclarations.end (); ++it)
        {
          if (iequals ((*it)->getVariableName (), opGBLName))
          {
            return *it;
          }
        }

        throw opGBLName;
      }

      typename std::vector <T *>::const_iterator
      firstSubroutineInSourceCode ()
      {
        return subroutinesInSourceCode.begin ();
      }

      typename std::vector <T *>::const_iterator
      lastSubroutineInSourceCode ()
      {
        return subroutinesInSourceCode.end ();
      }
  };

#endif
