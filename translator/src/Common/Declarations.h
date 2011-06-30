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
       * Actual arguments passed to every OP_DECL_SETSubroutineHeader in the
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
       * Actual arguments passed to every OP_DECL_DATSubroutineHeader in the
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
      getOpSetDeclaration (
          std::string const & opSETSubroutineHeaderName)
          throw (std::string const &)
      {
        using boost::iequals;
        using std::vector;

        for (vector <OpSetDeclaration *>::iterator it =
            OpSetDeclarations.begin (); it != OpSetDeclarations.end (); ++it)
        {
          if (iequals ((*it)->getVariableName (), opSETSubroutineHeaderName))
          {
            return *it;
          }
        }

        throw opSETSubroutineHeaderName;
      }

      OpMapDeclaration *
      getOpMapDeclaration (std::string const & opMAPName)
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
      getOpDatDeclaration (
          std::string const & opDATSubroutineHeaderName)
          throw (std::string const &)
      {
        using boost::iequals;
        using std::vector;

        for (vector <OpDatDeclaration *>::iterator it =
            OpDatDeclarations.begin (); it != OpDatDeclarations.end (); ++it)
        {
          if (iequals ((*it)->getVariableName (), opDATSubroutineHeaderName))
          {
            return *it;
          }
        }

        throw opDATSubroutineHeaderName;
      }

      OpGblDeclaration *
      getOpGblDeclaration (std::string const & opGBLName)
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
