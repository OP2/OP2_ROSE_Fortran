


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


#include <FortranProgramDeclarationsAndDefinitions.h>
#include <FortranOP2Definitions.h>
#include <FortranParallelLoop.h>
#include <Globals.h>
#include <Exceptions.h>
#include <OP2.h>
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
    Debug::getInstance ()->debugMessage ("'" + variableName
      + "' is unique for this call (or the first one encountered)", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);
      
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
    Debug::getInstance ()->debugMessage ("'" + variableName
      + "' is NOT unique for this call", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);    

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
    unsigned int OP_DAT_ArgumentGroup, unsigned int accessPosition)
{
  using boost::iequals;
  using boost::lexical_cast;
  using std::string;

  SgVarRefExp * accessExpression = isSgVarRefExp (
      actualArguments->get_expressions ()[accessPosition]);

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

SgVarRefExp *
FortranProgramDeclarationsAndDefinitions::getOpDatReferenceFromOpArg (SgExprListExp * opArgArguments)
{
  SgVarRefExp * opDatReference = NULL;

  if (isSgDotExp (opArgArguments ->get_expressions ()[DAT_POSITION]) != NULL)
    opDatReference = isSgVarRefExp (isSgDotExp (opArgArguments ->get_expressions ()[DAT_POSITION]) ->get_rhs_operand ());
  else
    opDatReference = isSgVarRefExp (opArgArguments->get_expressions ()[DAT_POSITION]);

  return opDatReference;
}

SgVarRefExp *
FortranProgramDeclarationsAndDefinitions::getOpMapReferenceFromOpArg (SgExprListExp * opArgArguments)
{
  SgVarRefExp * opMapReference = NULL;
      
  if (isSgDotExp (opArgArguments ->get_expressions ()[MAP_POSITION]) != NULL)        
    opMapReference = isSgVarRefExp (isSgDotExp (opArgArguments ->get_expressions ()[MAP_POSITION])->get_rhs_operand ());
  else        
    opMapReference = isSgVarRefExp (opArgArguments ->get_expressions ()[MAP_POSITION]);

  return opMapReference;
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
   * Loops over the arguments to populate the parallel loop object
   * Each object in the actualArguments array is a function call
   * We analyse the arguments of each call
   * \warning: the args start from position 2 (the first two args are
   * the user kernel reference and the iteration set
   * ======================================================
   */
  
  for ( int i = 2; i < numberOfOpArgs + 2; i++ )
  {
   /*
    * ======================================================
    * Distinguish between op_dat and global variable by
    * checking the function name
    * ======================================================
    */
    SgFunctionCallExp * functionExpression = isSgFunctionCallExp (
      actualArguments->get_expressions ()[i]);
    
    ROSE_ASSERT (functionExpression != NULL);
    
    string functionName = functionExpression ->getAssociatedFunctionSymbol ()->
      get_name ().getString ();
    
    SgExprListExp * opArgArguments = functionExpression ->get_args ();
    ROSE_ASSERT (opArgArguments );

    /*
     * ======================================================
     * Assume that this is not an op_mat, possibly amend later
     * ======================================================     
     */
    parallelLoop->setIsOpMatArg (OP_DAT_ArgumentGroup, false);
    
    if ( functionName == "op_arg_dat" )
    {     
     /*
      * ======================================================
      * Obtain the op_dat reference and its name
      * ======================================================
      */
     
      SgVarRefExp * opDatReference = NULL;

      opDatReference = getOpDatReferenceFromOpArg (opArgArguments);

      ROSE_ASSERT (opDatReference != NULL);

      string const opDatName = opDatReference->get_symbol ()->get_name ().getString ();

     /*
      * ======================================================
      * Obtain the map reference and name, and select access
      * type (direct or indirect)
      * ======================================================            
      */
      SgVarRefExp * opMapReference;

      opMapReference = getOpMapReferenceFromOpArg (opArgArguments);

      ROSE_ASSERT (opMapReference != NULL);

      string const mappingValue = opMapReference->get_symbol ()->get_name ().getString ();

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

      setOpDatProperties (parallelLoop, opDatName, OP_DAT_ArgumentGroup);

      setParallelLoopAccessDescriptor (parallelLoop, opArgArguments, OP_DAT_ArgumentGroup, DAT_ACC_POSITION);
    }
    else if ( functionName == "op_arg_gbl" )
    {

      Debug::getInstance ()->debugMessage ("...GLOBAL mapping descriptor",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

     /*
      * ======================================================
      * Get the OP_GBL variable reference and name
      * ======================================================
      */
      SgVarRefExp * opDatReference;

      if (isSgDotExp (opArgArguments->get_expressions ()[DAT_POSITION])
          != NULL)
      {
        opDatReference
            = isSgVarRefExp (
                isSgDotExp (
                    opArgArguments->get_expressions ()[DAT_POSITION])->get_rhs_operand ());
      }
      else
      {
        opDatReference = isSgVarRefExp (
            opArgArguments->get_expressions ()[DAT_POSITION]);
      }

      string const globalName =
          opDatReference->get_symbol ()->get_name ().getString ();
      

     /*
      * ======================================================
      * Get the OP_GBL dimension: check the number of args
      * to the op_arg_gbl function (3 = array, 2 = scalar)
      * ======================================================
      */
      if ( opArgArguments->get_expressions ().size () == GBL_SCALAR_ARG_NUM )
      {

        Debug::getInstance ()->debugMessage ("'" + globalName + "' is a scalar",
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
        SgIntVal * intValExp = isSgIntVal (opArgArguments->get_expressions ()[GBL_DIM_POSITION]);
        ROSE_ASSERT (intValExp != NULL);
        
        int globalDimension = intValExp->get_value ();
        
        parallelLoop->setOpDatDimension (OP_DAT_ArgumentGroup, globalDimension);
        
        Debug::getInstance ()->debugMessage ("'" + globalName
          + "' is NOT a scalar, but has dimension " + lexical_cast<string> (globalDimension),
          Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
      }

     /*
      * ======================================================
      * Set the other fields
      * ======================================================
      */
      parallelLoop->setOpDatType (OP_DAT_ArgumentGroup,
        (opArgArguments->get_expressions ()[DAT_POSITION])->get_type ());
     
      parallelLoop->setUniqueOpDat (globalName);

      parallelLoop->setOpDatVariableName (OP_DAT_ArgumentGroup, globalName);

      parallelLoop->setDuplicateOpDat (OP_DAT_ArgumentGroup, false);

      parallelLoop->setOpMapValue (OP_DAT_ArgumentGroup, GLOBAL);

      if ( opArgArguments->get_expressions ().size () == GBL_SCALAR_ARG_NUM )       
        setParallelLoopAccessDescriptor (parallelLoop, opArgArguments,
          OP_DAT_ArgumentGroup, GBL_SCALAR_ACC_POSITION);
      else        
        setParallelLoopAccessDescriptor (parallelLoop, opArgArguments,
          OP_DAT_ArgumentGroup, GBL_ARRAY_ACC_POSITION);
    }
    else if ( functionName == "op_arg_dat_generic" )
    {
      Debug::getInstance ()->debugMessage ("Found generic op_dat",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      /*
       * ======================================================
       * Obtain the op_dat reference and its name
       * ======================================================
       */
      SgVarRefExp * opDatReference = NULL;

      opDatReference = getOpDatReferenceFromOpArg (opArgArguments);

      ROSE_ASSERT (opDatReference != NULL);

      string const opDatName = opDatReference->get_symbol ()->get_name ().getString ();      

      /*
       * ======================================================
       * Obtain the map reference and name, and select access
       * type (direct or indirect)
       * ======================================================
       */
      SgVarRefExp * opMapReference;

      opMapReference = getOpMapReferenceFromOpArg (opArgArguments);

      ROSE_ASSERT (opMapReference != NULL);

      string const mappingValue = opMapReference->get_symbol ()->get_name ().getString ();

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
       * In case of generic loop, I have to read also the
       * dimension and type to be able to set the op_dat info
       * in the parallel loop class
       * ======================================================
       */
      SgIntVal * opDatDimension = isSgIntVal (
        opArgArguments->get_expressions ()[GENERIC_DIM_POSITION]);

        
      SgStringVal * opDataBaseTypeString = isSgStringVal (
        opArgArguments->get_expressions ()[GENERIC_TYPE_POSITION]);

      ROSE_ASSERT (opDataBaseTypeString != NULL);
        
      SgType * opDataBaseType = getTypeFromString (opDataBaseTypeString->get_value (),
        opDatName);

      ROSE_ASSERT (opDataBaseType != NULL);

      setOpDatPropertiesGeneric (parallelLoop, opDatName, opDatDimension->get_value (),
         opDataBaseType, OP_DAT_ArgumentGroup);

      Debug::getInstance ()->debugMessage ("Getting access",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      setParallelLoopAccessDescriptor (parallelLoop, opArgArguments,
        OP_DAT_ArgumentGroup, GENERIC_ACC_POSITION);
    } 
    else if ( functionName == "op_arg_mat" )
    {
      Debug::getInstance ()->debugMessage ("Unsupported argument type",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);
    }
    else
    {
      Debug::getInstance ()->debugMessage ("Argument type not recognised",
        Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);      
    }

   /*
    * ======================================================
    * This identifies the argument position in the data
    * structures, while the iteration variable 'i' identifies
    * the corresponding op_arg position in the op_par_loop
    * arguments (i == OP_DAT_ArgumentGroup + 2)
    * ======================================================
    */

    OP_DAT_ArgumentGroup++;
  }
  
  parallelLoop->setNumberOfOpDatArgumentGroups (numberOfOpArgs);
  parallelLoop->setNumberOfOpMatArgumentGroups (0);
  
  if ( parallelLoop->isDirectLoop () )
    Debug::getInstance ()->debugMessage ("This is a DIRECT parallel loop",
      Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);
  else
    Debug::getInstance ()->debugMessage ("This is an INDIRECT parallel loop",
      Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);  
}

void
FortranProgramDeclarationsAndDefinitions::visit (SgNode * node)
{
  using boost::filesystem::path;
  using boost::filesystem::system_complete;
  using boost::iequals;
  using boost::starts_with;
  using boost::lexical_cast;
  using std::string;

  if (isSgSourceFile (node))
  {
    path p = system_complete (path (isSgSourceFile (node)->getFileName ()));

    currentSourceFile = p.filename ();

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

        if ( iequals (calleeName, OP2::OP_DECL_SET) ||
             iequals (calleeName, OP2::OP_DECL_SET_HDF5) )
        {
          /*
           * ======================================================
           * An OP_SET variable declared through an OP_DECL_SET call
           * ======================================================
           */
          bool isHDF5Format = false;
          
          if ( iequals (calleeName, OP2::OP_DECL_SET_HDF5) ) isHDF5Format = true;
          
          FortranOpSetDefinition * opSetDeclaration =
              new FortranOpSetDefinition (actualArguments, isHDF5Format);

          OpSetDefinitions[opSetDeclaration->getVariableName ()]
              = opSetDeclaration;
        }        
        else if ( iequals (calleeName, OP2::OP_DECL_MAP) ||
                  iequals (calleeName, OP2::OP_DECL_MAP_HDF5) )
        {
          /*
           * ======================================================
           * An OP_MAP variable declared through an OP_DECL_MAP call
           * ======================================================
           */

          bool isHDF5Format = false;
          
          if ( iequals (calleeName, OP2::OP_DECL_MAP_HDF5) ) isHDF5Format = true;

          FortranOpMapDefinition * opMapDeclaration =
              new FortranOpMapDefinition (actualArguments, isHDF5Format);

          OpMapDefinitions[opMapDeclaration->getVariableName ()]
              = opMapDeclaration;
        }
        else if ( iequals (calleeName, OP2::OP_DECL_DAT) ||
                  iequals (calleeName, OP2::OP_DECL_DAT_HDF5) )
        {
          /*
           * ======================================================
           * An OP_DAT variable declared through an OP_DECL_DAT call
           * ======================================================
           */
          bool isHDF5Format = false;
          
          if ( iequals (calleeName, OP2::OP_DECL_DAT_HDF5) ) isHDF5Format = true;

          FortranOpDatDefinition * opDatDeclaration =
              new FortranOpDatDefinition (actualArguments, isHDF5Format);

          OpDatDefinitions[opDatDeclaration->getVariableName ()]
              = opDatDeclaration;
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

            Debug::getInstance ()->debugMessage ("Parallel loop with '"
                + lexical_cast <string> (numberOfOpArgs)  + "' arguments",
                Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);
                        
            analyseParallelLoopArguments (parallelLoop, actualArguments,
              numberOfOpArgs);

            parallelLoop->checkArguments ();
            
            parallelLoop->setIncrementalID (IDCounter);
            IDCounter++;
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
    SgProject * project) : IDCounter (0)
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
