


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


#include "FortranProgramDeclarationsAndDefinitions.h"
#include "FortranOP2Definitions.h"
#include "FortranParallelLoop.h"
#include "Globals.h"
#include "Exceptions.h"
#include "OP2.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

void
FortranProgramDeclarationsAndDefinitions::setOpGblProperties (
    FortranParallelLoop * parallelLoop, std::string const & variableName,
    int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage ("'" + variableName + "' is an OP_GBL",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  OpGblDefinition * opGblDeclaration = getOpGblDefinition (variableName);
  
  FortranOpGblDefinition * fortanOpGBL =
      dynamic_cast <FortranOpGblDefinition *> (opGblDeclaration);
      
  if (fortanOpGBL == NULL)
  {
    Debug::getInstance ()->debugMessage ("'" + variableName + "' is a scalar",
        Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    /*
     * ======================================================
     * Since this is a scalar, set the dimension to 1
     * ======================================================
     */

    parallelLoop->setOpDatDimension (OP_DAT_ArgumentGroup, 1);
  }
  else
  {
    Debug::getInstance ()->debugMessage ("'" + variableName
        + "' is NOT a scalar, but has dimension " + lexical_cast<string> (opGblDeclaration->getDimension ()),
        Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpDatDimension (OP_DAT_ArgumentGroup,
        opGblDeclaration->getDimension ());
  }

  parallelLoop->setUniqueOpDat (variableName);

  parallelLoop->setOpDatType (OP_DAT_ArgumentGroup,
      opGblDeclaration->getBaseType ());

  parallelLoop->setOpDatVariableName (OP_DAT_ArgumentGroup, variableName);

  parallelLoop->setDuplicateOpDat (OP_DAT_ArgumentGroup, false);
}

void
FortranProgramDeclarationsAndDefinitions::setOpDatProperties (
    FortranParallelLoop * parallelLoop, std::string const & variableName,
    int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  OpDatDefinition * opDatDeclaration = getOpDatDefinition (variableName);

  Debug::getInstance ()->debugMessage ("'" + variableName
      + "' has been declared through OP_DECL_DAT", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  parallelLoop->setOpDatType (OP_DAT_ArgumentGroup,
      opDatDeclaration->getBaseType ());

  parallelLoop->setOpDatDimension (OP_DAT_ArgumentGroup,
      opDatDeclaration->getDimension ());

  parallelLoop->setOpDatVariableName (OP_DAT_ArgumentGroup, variableName);

  if (parallelLoop->isUniqueOpDat (variableName))
  {
    parallelLoop->setUniqueOpDat (variableName);

    parallelLoop->setDuplicateOpDat (OP_DAT_ArgumentGroup, false);

    if (isSgArrayType (opDatDeclaration->getBaseType ()) == NULL)
    {
      throw Exceptions::ParallelLoop::UnsupportedBaseTypeException ("OP_DAT '"
          + variableName + "' is not an array");
    }
  }
  else
  {
    parallelLoop->setDuplicateOpDat (OP_DAT_ArgumentGroup, true);
  }
}

void
FortranProgramDeclarationsAndDefinitions::setOpDatPropertiesGeneric (
    FortranParallelLoop * parallelLoop, std::string const & variableName,
    int opDatDimension, SgType * opDatBaseType, int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage ("'" + variableName
      + "' has been declared through OP_DECL_DAT with GENERIC format", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  parallelLoop->setOpDatType (OP_DAT_ArgumentGroup,
      opDatBaseType);

  parallelLoop->setOpDatDimension (OP_DAT_ArgumentGroup,
      opDatDimension);

  parallelLoop->setOpDatVariableName (OP_DAT_ArgumentGroup, variableName);

  if (parallelLoop->isUniqueOpDat (variableName))
  {
    parallelLoop->setUniqueOpDat (variableName);

    parallelLoop->setDuplicateOpDat (OP_DAT_ArgumentGroup, false);
  }
  else
  {
    parallelLoop->setDuplicateOpDat (OP_DAT_ArgumentGroup, true);
  }
}


void
FortranProgramDeclarationsAndDefinitions::setParallelLoopAccessDescriptor (
    FortranParallelLoop * parallelLoop, SgExprListExp * actualArguments,
    unsigned int OP_DAT_ArgumentGroup, unsigned int argumentPosition)
{
  using boost::iequals;
  using boost::lexical_cast;
  using std::string;

  SgVarRefExp * accessExpression = isSgVarRefExp (
      actualArguments->get_expressions ()[argumentPosition]);

  string const accessValue =
      accessExpression->get_symbol ()->get_name ().getString ();

  if (iequals (accessValue, OP2::OP_READ))
  {
    Debug::getInstance ()->debugMessage ("...READ access descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpAccessValue (OP_DAT_ArgumentGroup, READ_ACCESS);
  }
  else if (iequals (accessValue, OP2::OP_WRITE))
  {
    Debug::getInstance ()->debugMessage ("...WRITE access descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpAccessValue (OP_DAT_ArgumentGroup, WRITE_ACCESS);
  }
  else if (iequals (accessValue, OP2::OP_INC))
  {
    Debug::getInstance ()->debugMessage ("...INCREMENT access descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpAccessValue (OP_DAT_ArgumentGroup, INC_ACCESS);
  }
  else if (iequals (accessValue, OP2::OP_RW))
  {
    Debug::getInstance ()->debugMessage ("...READ/WRITE access descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpAccessValue (OP_DAT_ArgumentGroup, RW_ACCESS);
  }
  else if (iequals (accessValue, OP2::OP_MAX))
  {
    Debug::getInstance ()->debugMessage ("...MAXIMUM access descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpAccessValue (OP_DAT_ArgumentGroup, MAX_ACCESS);
  }
  else if (iequals (accessValue, OP2::OP_MIN))
  {
    Debug::getInstance ()->debugMessage ("...MINIMUM access descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    parallelLoop->setOpAccessValue (OP_DAT_ArgumentGroup, MIN_ACCESS);
  }
  else
  {
    throw Exceptions::ParallelLoop::UnknownAccessException (
        "Unknown access descriptor: '" + accessValue + "' for OP_DAT argument "
            + lexical_cast <string> (OP_DAT_ArgumentGroup));
  }
}

void
FortranProgramDeclarationsAndDefinitions::analyseParallelLoopArguments (
    FortranParallelLoop * parallelLoop, SgExprListExp * actualArguments,
    int numberOfOpArgs)
{
  using boost::iequals;
  using boost::lexical_cast;
  using std::find;
  using std::string;
  using std::vector;
  using std::map;

  Debug::getInstance ()->debugMessage (
      "Analysing OP_PAR_LOOP actual arguments", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  unsigned int OP_DAT_ArgumentGroup = 1;
  
  /*
   * ======================================================
   * The actual offset between args is given by the kind
   * of op_arg encountered (i.e. standard or generic)
   * For this reason, the offset index is incremented
   * according to the kind of argument encountered
   * ======================================================
   */
   
  for (unsigned int offset = parallelLoop->NUMBER_OF_NON_OP_DAT_ARGUMENTS; offset
      < actualArguments->get_expressions ().size (); )
  {
    /*
     * ======================================================
     * Get the OP_DAT variable name
     * ======================================================
     */

    unsigned int opDatArgumentPosition = offset
        + parallelLoop->POSITION_OF_OP_DAT;

    SgVarRefExp * opDatReference;

    if (isSgDotExp (actualArguments->get_expressions ()[opDatArgumentPosition])
        != NULL)
    {
      opDatReference
          = isSgVarRefExp (
              isSgDotExp (
                  actualArguments->get_expressions ()[opDatArgumentPosition])->get_rhs_operand ());
    }
    else
    {
      opDatReference = isSgVarRefExp (
          actualArguments->get_expressions ()[opDatArgumentPosition]);
    }

    string const opDatName =
        opDatReference->get_symbol ()->get_name ().getString ();

    unsigned int opMapArgumentPosition = offset
        + parallelLoop->POSITION_OF_MAPPING;

    /*
     * ======================================================
     * Get the OP_MAP name
     * ======================================================
     */

    SgVarRefExp * opMapReference;

    if (isSgDotExp (actualArguments->get_expressions ()[opMapArgumentPosition])
        != NULL)
    {
      opMapReference
          = isSgVarRefExp (
              isSgDotExp (
                  actualArguments->get_expressions ()[opMapArgumentPosition])->get_rhs_operand ());
    }
    else
    {
      opMapReference = isSgVarRefExp (
          actualArguments->get_expressions ()[opMapArgumentPosition]);
    }

    string const mappingValue =
        opMapReference->get_symbol ()->get_name ().getString ();

    Debug::getInstance ()->debugMessage ("OP_DAT '" + opDatName + "'",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    if (iequals (mappingValue, OP2::OP_GBL))
    {
      /*
       * ======================================================
       * OP_GBL signals that the op_arg is a global variable
       * No generic interface is allowed in this case
       * ======================================================
       */
      Debug::getInstance ()->debugMessage ("...GLOBAL mapping descriptor",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      setOpGblProperties (parallelLoop, opDatName, OP_DAT_ArgumentGroup);

      parallelLoop->setOpMapValue (OP_DAT_ArgumentGroup, GLOBAL);

      setParallelLoopAccessDescriptor (parallelLoop, actualArguments,
        OP_DAT_ArgumentGroup, offset + parallelLoop->POSITION_OF_ACCESS);
      
      offset += parallelLoop->NUMBER_OF_ARGUMENTS_PER_OP_DAT;
    }
    else
    {

      if (iequals (mappingValue, OP2::OP_ID))
      {
        /*
         * ======================================================
         * OP_ID signals identity mapping and therefore direct
         * access to the data
         * ======================================================
         */
        Debug::getInstance ()->debugMessage ("...DIRECT mapping descriptor",
            Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

        parallelLoop->setOpMapValue (OP_DAT_ArgumentGroup, DIRECT);
      }
      else
      {
        Debug::getInstance ()->debugMessage ("...INDIRECT mapping descriptor",
            Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

        parallelLoop->setOpMapValue (OP_DAT_ArgumentGroup, INDIRECT);
      }
           
      /*
       * ======================================================
       * Check if the op_arg is a standard or generic one
       * and set the op_dat properties consequently
       * ======================================================
       */
      int accessDescriptorPositionStandardLoop = offset
        + parallelLoop->NUMBER_OF_ARGUMENTS_PER_OP_DAT - 1;

      /*
       * ======================================================
       * I am looking for either a string (OP_INC, OP_RW, ..)
       * or an integer, denoting the dimension
       * ======================================================
       */
      SgIntVal * opDatDimension = isSgIntVal (
        actualArguments->get_expressions ()[accessDescriptorPositionStandardLoop]);

      /*
       * ======================================================
       * Found a access string => standard op_args
       * Not found a dimension => generic op_args
       * ======================================================
       */ 
      if ( opDatDimension == NULL )
      {
        setOpDatProperties (parallelLoop, opDatName, OP_DAT_ArgumentGroup);

        setParallelLoopAccessDescriptor (parallelLoop, actualArguments,
            OP_DAT_ArgumentGroup, offset + parallelLoop->POSITION_OF_ACCESS);

        offset += parallelLoop->NUMBER_OF_ARGUMENTS_PER_OP_DAT;
      }
      else
      {
       /*
        * ======================================================
        * In case of generic loop, I have to read also the
        * dimension and type to be able to set the op_dat info
        * in the parallel loop class
        * ======================================================
        */

//        unsigned int dimArgumentPosition = offset
//          + parallelLoop->POSITION_OF_DIMENSION;
                
//        SgIntVal * opDataDimension = isSgIntVal (
//          actualArguments->get_expressions ()[dimArgumentPosition]);

//        ROSE_ASSERT (opDataDimension != NULL);

        unsigned int typeArgumentPosition = offset
          + parallelLoop->POSITION_OF_TYPE;
        
        SgStringVal * opDataBaseTypeString = isSgStringVal (
          actualArguments->get_expressions ()[typeArgumentPosition]);

        ROSE_ASSERT (opDataBaseTypeString != NULL);
        
        SgType * opDataBaseType = getTypeFromString (opDataBaseTypeString->get_value (),
          opDatName);
          
        setOpDatPropertiesGeneric (parallelLoop, opDatName, opDatDimension->get_value (),
          opDataBaseType, OP_DAT_ArgumentGroup);
          
        setParallelLoopAccessDescriptor (parallelLoop, actualArguments,
            OP_DAT_ArgumentGroup, offset + parallelLoop->POSITION_OF_ACCESS_GENERIC);

        offset += parallelLoop->NUMBER_OF_ARGUMENTS_PER_OP_DAT_GENERIC;
      }
    }

    parallelLoop->setIsOpMatArg (OP_DAT_ArgumentGroup, false);
    OP_DAT_ArgumentGroup++;
  }

  parallelLoop->setNumberOfOpDatArgumentGroups (OP_DAT_ArgumentGroup - 1);
  parallelLoop->setNumberOfOpMatArgumentGroups (0);
}

void
FortranProgramDeclarationsAndDefinitions::visit (SgNode * node)
{
  using boost::filesystem::path;
  using boost::filesystem::system_complete;
  using boost::iequals;
  using boost::starts_with;
  using std::string;

  if (isSgSourceFile (node))
  {
    path p = system_complete (path (isSgSourceFile (node)->getFileName ()));

#if BOOST_FILESYSTEM_VERSION == 3
    currentSourceFile = p.filename ().string ();
#else
    currentSourceFile = p.filename ();
#endif

    Debug::getInstance ()->debugMessage ("Source file '" + currentSourceFile
        + "' detected", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__ );
  }
  else if (Globals::getInstance ()->isInputFile (currentSourceFile))
  {
    /*
     * ======================================================
     * Only process this portion of the AST if we recognise
     * this source file as one passed on the command line. In
     * Fortran, .rmod files are sometimes generated whose
     * traversal should be avoided
     * ======================================================
     */

    switch (node->variantT ())
    {
      case V_SgModuleStatement:
      {
        SgModuleStatement * moduleStatement = isSgModuleStatement (node);

        currentModuleName = moduleStatement->get_name ().getString ();

        fileNameToModuleNames[currentSourceFile].push_back (currentModuleName);

        moduleNameToFileName[currentModuleName] = currentSourceFile;

        Debug::getInstance ()->debugMessage ("Module '" + currentModuleName
            + "' in file '" + currentSourceFile + "'", Debug::OUTER_LOOP_LEVEL,
            __FILE__, __LINE__ );

        break;
      }

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

        string const subroutineName =
            procedureHeaderStatement->get_name ().getString ();

        subroutinesInSourceCode[subroutineName] = procedureHeaderStatement;

        ROSE_ASSERT (currentModuleName.size() > 0);

        moduleNameToSubroutines[currentModuleName].push_back (subroutineName);

        subroutineToFileName[subroutineName] = currentSourceFile;

        Debug::getInstance ()->debugMessage (
            "Found procedure header statement '"
                + procedureHeaderStatement->get_name ().getString ()
                + "' in file '" + currentSourceFile + "', and module '"
                + currentModuleName + "'", Debug::FUNCTION_LEVEL, __FILE__,
            __LINE__);

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

        SgExprListExp * actualArguments = functionCallExp->get_args ();

        string const
            calleeName =
                functionCallExp->getAssociatedFunctionSymbol ()->get_name ().getString ();

        Debug::getInstance ()->debugMessage ("Found function call '"
            + calleeName + "'", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

        if (iequals (calleeName, OP2::OP_DECL_SET))
        {
          /*
           * ======================================================
           * An OP_SET variable declared through an OP_DECL_SET call
           * ======================================================
           */

          FortranOpSetDefinition * opSetDeclaration =
              new FortranOpSetDefinition (actualArguments);

          OpSetDefinitions[opSetDeclaration->getVariableName ()]
              = opSetDeclaration;
        }
        else if (iequals (calleeName, OP2::OP_DECL_MAP))
        {
          /*
           * ======================================================
           * An OP_MAP variable declared through an OP_DECL_MAP call
           * ======================================================
           */

          FortranOpMapDefinition * opMapDeclaration =
              new FortranOpMapDefinition (actualArguments);

          OpMapDefinitions[opMapDeclaration->getVariableName ()]
              = opMapDeclaration;
        }
        else if (iequals (calleeName, OP2::OP_DECL_DAT))
        {
          /*
           * ======================================================
           * An OP_DAT variable declared through an OP_DECL_DAT call
           * ======================================================
           */

          FortranOpDatDefinition * opDatDeclaration =
              new FortranOpDatDefinition (actualArguments);

          OpDatDefinitions[opDatDeclaration->getVariableName ()]
              = opDatDeclaration;
        }
        else if (iequals (calleeName, OP2::OP_DECL_GBL))
        {
          /*
           * ======================================================
           * An OP_DAT variable declared through an OP_DECL_GBL call
           * ======================================================
           */

          OpGblDefinition * opGblDeclaration;

          if (actualArguments->get_expressions ().size ()
              == FortranOpGblDefinition::getNumberOfExpectedArguments ())
          {
            opGblDeclaration = new FortranOpGblDefinition (actualArguments);
          }
          else
          {
            ROSE_ASSERT (actualArguments->get_expressions().size() == FortranOpGblScalarDefinition::getNumberOfExpectedArguments());

            opGblDeclaration = new FortranOpGblScalarDefinition (
                actualArguments);
          }

          OpGblDefinitions[opGblDeclaration->getVariableName ()]
              = opGblDeclaration;
        }
        else if (iequals (calleeName, OP2::OP_DECL_CONST))
        {
          /*
           * ======================================================
           * A constant declared through an OP_DECL_CONST call
           * ======================================================
           */

          FortranOpConstDefinition * opConstDeclaration =
              new FortranOpConstDefinition (actualArguments, functionCallExp);

          OpConstDefinitions[opConstDeclaration->getVariableName ()]
              = opConstDeclaration;
        }
        else if (starts_with (calleeName, OP2::OP_PAR_LOOP))
        {
          /*
           * ======================================================
           * The first argument to an 'OP_PAR_LOOP' call should be
           * a reference to the kernel function. Cast it and proceed,
           * otherwise throw an exception
           * ======================================================
           */

          SgExprListExp * actualArguments = functionCallExp->get_args ();

          SgFunctionRefExp * functionRefExpression = isSgFunctionRefExp (
              actualArguments->get_expressions ().front ());

          ROSE_ASSERT (functionRefExpression != NULL);

          string const
              userSubroutineName =
                  functionRefExpression->getAssociatedFunctionDeclaration ()->get_name ().getString ();

          Debug::getInstance ()->debugMessage ("Found '" + calleeName
              + "' with (host) user subroutine '" + userSubroutineName + "'",
              Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

          if (parallelLoops.find (userSubroutineName) == parallelLoops.end ())
          {
            int numberOfOpArgs = getLoopSuffixNumber ( calleeName );

            /*
             * ======================================================
             * If this kernel has not been previously encountered then
             * build a new parallel loop representation
             * ======================================================
             */

            FortranParallelLoop * parallelLoop = new FortranParallelLoop (
                functionCallExp);

            parallelLoop->addFileName (currentSourceFile);

            parallelLoops[userSubroutineName] = parallelLoop;

            analyseParallelLoopArguments (parallelLoop, actualArguments,
              numberOfOpArgs);

            parallelLoop->checkArguments ();
          }
          else
          {
            Debug::getInstance ()->debugMessage ("Parallel loop for '"
                + userSubroutineName + "' already created",
                Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

            ParallelLoop * parallelLoop = parallelLoops[userSubroutineName];

            parallelLoop->addFunctionCallExpression (functionCallExp);

            parallelLoop->addFileName (currentSourceFile);
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
}

bool
FortranProgramDeclarationsAndDefinitions::checkModuleExists (
    std::string const & moduleName)
{
  using boost::iequals;
  using std::string;
  using std::map;
  using std::vector;

  bool found = false;

  for (map <string, vector <string> >::const_iterator it =
      fileNameToModuleNames.begin (); it != fileNameToModuleNames.end (); ++it)
  {
    string fileName = it->first;

    vector <string> moduleNames = it->second;

    for (vector <string>::const_iterator vectorIt = moduleNames.begin (); vectorIt
        != moduleNames.end (); ++vectorIt)
    {
      if (iequals (*vectorIt, moduleName))
      {
        found = true;
      }
    }
  }

  return found;
}

int
FortranProgramDeclarationsAndDefinitions::getLoopSuffixNumber (std::string const & calleeName)
{
  using std::string;
  using boost::lexical_cast; 
  
  string argumentsNumberString = calleeName.substr (OP2::OP_PAR_LOOP.size () + 1, calleeName.size () - 1);
    
  return lexical_cast<int> (argumentsNumberString);
}

SgType *
FortranProgramDeclarationsAndDefinitions::getTypeFromString (std::string const & opDataBaseTypeString,
  std::string const & variableName)
{
  using namespace SageBuilder;
  using boost::iequals;

  if ( iequals(opDataBaseTypeString, OP2::FortranSpecific::PrimitiveTypes::real8) )
    return buildDoubleType ();
  else if ( iequals(opDataBaseTypeString, OP2::FortranSpecific::PrimitiveTypes::real4) )
    return buildFloatType ();
  else if ( iequals(opDataBaseTypeString, OP2::FortranSpecific::PrimitiveTypes::integer4) )
    return buildIntType ();
  else
      throw Exceptions::ParallelLoop::UnsupportedBaseTypeException ("Bad type specified for GENERIC OP_DAT '"
          + variableName + "' ");    
}

std::vector <std::string>::const_iterator
FortranProgramDeclarationsAndDefinitions::getFirstSubroutine (
    std::string const & moduleName)
{
  return moduleNameToSubroutines[moduleName].begin ();
}

std::vector <std::string>::const_iterator
FortranProgramDeclarationsAndDefinitions::getLastSubroutine (
    std::string const & moduleName)
{
  return moduleNameToSubroutines[moduleName].end ();
}

std::string const &
FortranProgramDeclarationsAndDefinitions::getFileNameForModule (
    std::string const & moduleName)
{
  return moduleNameToFileName[moduleName];
}

std::string const &
FortranProgramDeclarationsAndDefinitions::getFileNameForSubroutine (
    std::string const & subroutineName)
{
  return subroutineToFileName[subroutineName];
}

FortranProgramDeclarationsAndDefinitions::FortranProgramDeclarationsAndDefinitions (
    SgProject * project)
{
  Debug::getInstance ()->debugMessage (
      "Obtaining declarations and definitions", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__ );

  traverseInputFiles (project, preorder);

  if (checkModuleExists (Globals::getInstance ()->getFreeVariablesModuleName ())
      == false)
  {
    throw Exceptions::ASTParsing::NoSourceFileException (
        "Unable to find the module '"
            + Globals::getInstance ()->getFreeVariablesModuleName ()
            + "' containing OP2 constant information");
  }
}
