


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


#include "FortranSubroutinesGeneration.h"
#include "FortranParallelLoop.h"
#include "FortranHostSubroutine.h"
#include "FortranTypesBuilder.h"
#include "FortranProgramDeclarationsAndDefinitions.h"
#include "FortranReductionSubroutines.h"
#include "RoseHelper.h"
#include <boost/algorithm/string/predicate.hpp>
//#include <tr1_impl/complex>

void
FortranSubroutinesGeneration::processOP2ConstantDeclarations ()
{
  Debug::getInstance ()->debugMessage ("Processing OP_DECL_CONST calls",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

void
FortranSubroutinesGeneration::addModuleUseStatement (SgNode * parent,
    std::string const & moduleName)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Adding module use statement",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  if (isSgProgramHeaderStatement (parent))
  {
    Debug::getInstance ()->debugMessage ("Program header statement",
        Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

    SgProgramHeaderStatement * parentStatement = isSgProgramHeaderStatement (
        parent);

    SgScopeStatement * scope = parentStatement->get_definition ()->get_body ();

    if (find (headersWithAddedUseStatements.begin (),
        headersWithAddedUseStatements.end (),
        parentStatement->get_name ().getString ())
        == headersWithAddedUseStatements.end ())
    {
      SgUseStatement * newUseStatement = new SgUseStatement (
          getEnclosingFileNode (parentStatement)->get_file_info (), moduleName,
          false);

      prependStatement (newUseStatement, scope);

      headersWithAddedUseStatements.push_back (
          parentStatement->get_name ().getString ());
    }
  }
  else if (isSgProcedureHeaderStatement (parent))
  {
    Debug::getInstance ()->debugMessage ("Procedure header statement",
        Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

    SgProcedureHeaderStatement * parentStatement =
        isSgProcedureHeaderStatement (parent);

    SgScopeStatement * scope = parentStatement->get_definition ()->get_body ();

    if (find (headersWithAddedUseStatements.begin (),
        headersWithAddedUseStatements.end (),
        parentStatement->get_name ().getString ())
        == headersWithAddedUseStatements.end ())
    {
      Debug::getInstance ()->debugMessage ("Prepending the statement",
        Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

      SgUseStatement * newUseStatement = new SgUseStatement (
          getEnclosingFileNode (parentStatement)->get_file_info (), moduleName,
          false);

      prependStatement (newUseStatement, scope);

      headersWithAddedUseStatements.push_back (
          parentStatement->get_name ().getString ());

      Debug::getInstance ()->debugMessage ("Done prepending",
        Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);
    }
  }
  else
  {
    throw Exceptions::CodeGeneration::UnfoundStatementException (
        "Could not find enclosing scope of OP_PAR_LOOP call");
  }
}

void
FortranSubroutinesGeneration::patchCallsToParallelLoops (
    std::string const & moduleName)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using std::map;
  using std::vector;
  using std::string;
  using std::find;
  using boost::lexical_cast;

  Debug::getInstance ()->debugMessage ("Patching calls to OP_PAR_LOOPs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    FortranParallelLoop * parallelLoop =
        static_cast <FortranParallelLoop *> (it->second);

    Debug::getInstance ()->debugMessage ("Parallel loop for "
        + userSubroutineName, Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    FortranHostSubroutine * hostSubroutine =
        hostSubroutines[userSubroutineName];

    for (vector <SgFunctionCallExp *>::const_iterator it =
        parallelLoop->getFirstFunctionCall (); it
        != parallelLoop->getLastFunctionCall (); ++it)
    {
      SgFunctionCallExp * functionCallExpression = *it;

      ROSE_ASSERT (isSgExprStatement(functionCallExpression->get_parent()));

      /*
       * ======================================================
       * Recursively go back in the scopes until we can find the
       * program header or subroutine header in which the
       * OP_PAR_LOOP call is contained
       * ======================================================
       */

      SgNode * parent = functionCallExpression->get_parent ();

      while (!isSgProcedureHeaderStatement (parent)
          && !isSgProgramHeaderStatement (parent))
      {
        parent = parent->get_parent ();
      }

      /*
       * ======================================================
       * Add module use statement to the scope just obtained
       * ======================================================
       */

      addModuleUseStatement (parent, moduleName);

      /*
       * ======================================================
       * Modify the call to OP_PAR_LOOP with a call to the newly
       * built host subroutine
       * ======================================================
       */

      SgFunctionRefExp * hostSubroutineReference = buildFunctionRefExp (
          hostSubroutine->getSubroutineHeaderStatement ());

      functionCallExpression->set_function (hostSubroutineReference);

      /*
       * ======================================================
       * Modify the first parameter from a kernel reference to
       * a kernel name
       * ======================================================
       */

      SgExpressionPtrList & arguments =
          functionCallExpression->get_args ()->get_expressions ();

      arguments.erase (arguments.begin ());

      SgStringVal * stringExpression = buildStringVal (userSubroutineName);

      SgFunctionSymbol * functionSymbol =
          FortranTypesBuilder::buildNewFortranFunction ("CHAR", moduleScope);

      SgExprListExp * actualParameters = buildExprListExp ();

      actualParameters->append_expression (buildIntVal (0));

      SgFunctionCallExp * functionCallExp = buildFunctionCallExp (
          functionSymbol, actualParameters);

      SgConcatenationOp * conctenationExpression = buildConcatenationOp (
          stringExpression, functionCallExp);

      arguments.insert (arguments.begin (), conctenationExpression);

      /*
       * ======================================================
       * Increment of index depends on op_arg type (standard
       * or generic).
       * \warning: THIS IS UNNECESSARY WITH THE OP_ARGS
       * ======================================================
       */      
//       for (unsigned int offset = parallelLoop->NUMBER_OF_NON_OP_DAT_ARGUMENTS; offset
//           < arguments.size (); )
//       {
//         int accessDescriptorPositionStandardLoop = offset
//           + parallelLoop->NUMBER_OF_ARGUMENTS_PER_OP_DAT - 1;
// 
//         /*
//          * ======================================================
//          * I am looking for either a string (OP_INC, OP_RW, ..)
//          * or an integer, denoting the dimension
//          * ======================================================
//          */
//           
//         SgIntVal * opDatDimension = isSgIntVal (
//           arguments[accessDescriptorPositionStandardLoop]);
// 
//         /*
//          * ======================================================
//          * Found a access string => standard op_args
//          * Not found a dimension => generic op_args
//          * ======================================================
//          */ 
//         if ( opDatDimension == NULL )
//         {
//           Debug::getInstance ()->debugMessage ("Standard op_arg",
//             Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);          
// 
//           offset += parallelLoop->NUMBER_OF_ARGUMENTS_PER_OP_DAT;
// 
//           Debug::getInstance ()->debugMessage ("Standard op_arg, offset = " +
//             lexical_cast<string> ( offset ),
//             Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);
//         }
//         else
//         {
//         /*
//          * ======================================================
//          * Generic loops have additional dimension and type 
//          * string information as parameters. These must be
//          * removed to make correspond the call signature
//          * to the definition one
//          * parallelLoop->NUMBER_OF_ARGUMENTS_PER_OP_DAT_GENERIC - 1;
//          * is the distance between the type string of an op_arg,
//          * and the dimension of the following one
//          * ======================================================
//          */   
//           int dimensionPosition = offset + parallelLoop->POSITION_OF_DIMENSION;
//           int typeStringPosition = offset + parallelLoop->POSITION_OF_TYPE;
//           
//           Debug::getInstance ()->debugMessage ("generic op_arg: erasing dimension and type from argument list, offset is " +
//           lexical_cast<string> ( offset ) + " and limit is " + lexical_cast<string> (arguments.size ()),
//             Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);
//           
//           arguments.erase (arguments.begin () + dimensionPosition, arguments.begin () + typeStringPosition +1);
//           
//           offset += parallelLoop->NUMBER_OF_ARGUMENTS_PER_OP_DAT;
//           
//           Debug::getInstance ()->debugMessage ("generic op_arg: erasing dimension and type from argument list, offset is " +
//             lexical_cast<string> ( offset ) + " and limit is " + lexical_cast<string> (arguments.size ()),
//             Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);
//           
//         }
//       }
//         
      /*
       * ======================================================
       * Set where the function call is invoked as a transformation
       * in the unparser
       * ======================================================
       */
      Debug::getInstance ()->debugMessage ("Done eliminating generic op_arg arguments, set transformation",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);          

      
      SgLocatedNode * functionCallLocation = isSgLocatedNode (
          functionCallExpression);

      ROSE_ASSERT (functionCallLocation != NULL);

      functionCallLocation->get_file_info ()->setTransformation ();
      
      Debug::getInstance ()->debugMessage ("Set transformation done",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);          

      
    }
  }
}

void
FortranSubroutinesGeneration::addContains ()
{
  using namespace SageInterface;

  SgContainsStatement * containsStatement = new SgContainsStatement (
      RoseHelper::getFileInfo ());

  containsStatement->set_definingDeclaration (containsStatement);

  appendStatement (containsStatement, moduleScope);
}

SgModuleStatement *
FortranSubroutinesGeneration::createFortranModule (
    std::string const & moduleName)
{
  using namespace SageInterface;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Creating Fortran module",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgGlobal * globalScope = sourceFile->get_globalScope ();

  SgModuleStatement * moduleStatement =
      FortranTypesBuilder::buildModuleDeclaration (moduleName, globalScope);

  moduleStatement->get_declarationModifier ().get_accessModifier ().setPublic ();

  moduleStatement->get_definition ()->setCaseInsensitive (true);

  appendStatement (moduleStatement, globalScope);

  return moduleStatement;
}

void
FortranSubroutinesGeneration::generate ()
{
  using std::string;

  string const & moduleName = "GENERATED_MODULE";

  SgModuleStatement * moduleStatement = createFortranModule (moduleName);

  moduleScope = moduleStatement->get_definition ();

  addLibraries ();

  createModuleDeclarations ();

  addContains ();

  createReductionSubroutines ();

  createSubroutines ();

  patchCallsToParallelLoops (moduleName);

  determineWhichInputFilesToBeUnparsed ();

  processOP2ConstantDeclarations ();
}

FortranSubroutinesGeneration::FortranSubroutinesGeneration (
    SgProject * project,
    FortranProgramDeclarationsAndDefinitions * declarations,
    std::string const & newFileName) :
  SubroutinesGeneration <FortranProgramDeclarationsAndDefinitions,
      FortranHostSubroutine> (project, declarations, newFileName)
{
  reductionSubroutines = new FortranReductionSubroutines ();
}
