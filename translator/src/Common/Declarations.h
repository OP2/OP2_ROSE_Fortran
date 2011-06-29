/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class stores all OP2 declared variables and
 * subroutines encountered while parsing the user-supplied
 * source code
 */

#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include <rose.h>
#include <OP2Variables.h>
#include <boost/algorithm/string.hpp>
#include <CommonNamespaces.h>
#include <Debug.h>

template <typename T>
  class Declarations: public AstSimpleProcessing
  {
    private:

      /*
       * ======================================================
       * The project which the source-to-source translator
       * operates upon
       * ======================================================
       */
      SgProject * project;

      /*
       * ======================================================
       * References to all subroutines found during the visit
       * of the input files
       * ======================================================
       */
      std::vector <T> subroutinesInSourceCode;

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

    private:

      /*
       * ======================================================
       * Traverses the supplied files to discover
       * OP_DECL_SET, OP_DECL_MAP, OP_DECL_DAT, OP_DECL_GBL calls
       * and store their actual arguments
       * ======================================================
       */

      virtual void
      visit (SgNode * node)
      {
        using boost::iequals;
        using boost::starts_with;
        using std::string;

        switch (node->variantT ())
        {
          case V_SgProcedureHeaderStatement:
          {
            /*
             * ======================================================
             * We need to store all subroutine definitions since we
             * later have to copy and modify the user kernel subroutine
             * ======================================================
             */
            SgProcedureHeaderStatement * procedureHeaderStatement =
                isSgProcedureHeaderStatement (node);

            subroutinesInSourceCode.push_back (procedureHeaderStatement);

            Debug::getInstance ()->debugMessage (
                "Found procedure header statement '"
                    + procedureHeaderStatement->get_name ().getString () + "'",
                8);

            break;
          }

          case V_SgFunctionCallExp:
          {
            /*
             * ======================================================
             * Function call found in the AST. Get its actual arguments
             * and the callee name
             * ======================================================
             */
            SgFunctionCallExp * functionCallExp = isSgFunctionCallExp (node);

            SgExpressionPtrList & actualArguments =
                functionCallExp->get_args ()->get_expressions ();

            string const
                calleeName =
                    functionCallExp->getAssociatedFunctionSymbol ()->get_name ().getString ();

            if (iequals (calleeName, OP2::OP_DECL_SET))
            {
              /*
               * ======================================================
               * An OP_SET variable declared through an OP_DECL_SET call
               * ======================================================
               */

              OpSetDeclaration * opSetDeclaration = new OpSetDeclaration (
                  actualArguments);

              OpSetDeclarations.push_back (opSetDeclaration);
            }
            else if (iequals (calleeName, OP2::OP_DECL_MAP))
            {
              /*
               * ======================================================
               * An OP_MAP variable declared through an OP_DECL_MAP call
               * ======================================================
               */

              OpMapDeclaration * opMapDeclaration = new OpMapDeclaration (
                  actualArguments);

              OpMapDeclarations.push_back (opMapDeclaration);
            }
            else if (iequals (calleeName, OP2::OP_DECL_DAT))
            {
              /*
               * ======================================================
               * An OP_DAT variable declared through an OP_DECL_DAT call
               * ======================================================
               */

              OpDatDeclaration * opDatDeclaration = new OpDatDeclaration (
                  actualArguments);

              if (isSgArrayType (opDatDeclaration->getActualType ()) == false)
              {
                Debug::getInstance ()->errorMessage ("OP_DAT variable '"
                    + opDatDeclaration->getVariableName ()
                    + "' is not an array type. Currently not supported.");
              }

              OpDatDeclarations.push_back (opDatDeclaration);
            }
            else if (iequals (calleeName, OP2::OP_DECL_GBL))
            {
              /*
               * ======================================================
               * An OP_DAT variable declared through an OP_DECL_GBL call
               * ======================================================
               */

              OpGblDeclaration * opGblDeclaration = new OpGblDeclaration (
                  actualArguments);

              OpGblDeclarations.push_back (opGblDeclaration);
            }

            break;
          }

          case V_SgVariableDeclaration:
          {
            SgVariableDeclaration * variableDeclaration =
                isSgVariableDeclaration (node);

            switch (variableDeclaration->get_variables ().front ()->get_type ()->variantT ())
            {
              case V_SgClassType:
              {
                SgClassType
                    * classType =
                        isSgClassType (
                            variableDeclaration->get_variables ().front ()->get_type ());

                if (starts_with (classType->get_name ().getString (),
                    OP2::OP_DAT))
                {

                }
                else if (starts_with (classType->get_name ().getString (),
                    OP2::OP_DAT_GBL))
                {

                }

                break;
              }

              case V_SgTypedefType:
              {
                SgTypedefType
                    * typedefType =
                        isSgTypedefType (
                            variableDeclaration->get_variables ().front ()->get_type ());

                if (iequals (typedefType->get_name ().getString (), OP2::OP_SET))
                {

                }

                break;
              }

              default:
              {
                break;
              }
            }

            break;
          }

          default:
          {
            break;
          }
        }
      }

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

      typename std::vector <T>::const_iterator
      firstSubroutineInSourceCode ()
      {
        return subroutinesInSourceCode.begin ();
      }

      typename std::vector <T>::const_iterator
      lastSubroutineInSourceCode ()
      {
        return subroutinesInSourceCode.end ();
      }

      Declarations (SgProject * project) :
        project (project)
      {
        traverseInputFiles (project, preorder);
      }
  };

#endif
