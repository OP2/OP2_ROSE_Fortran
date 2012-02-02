


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


#include "CPPPreProcess.h"
#include "CPPProgramDeclarationsAndDefinitions.h"
#include "CPPOP2Definitions.h"
#include "Debug.h"
#include "Exceptions.h"
#include "OP2.h"

void
CPPPreProcess::visit (SgNode* node)
{
    using boost::iequals;
    using std::string;
    using SageInterface::getScope;
    using SageInterface::insertStatement;
    using SageInterface::insertStatementListAfter;
    using SageInterface::insertStatementListBefore;
    using SageInterface::removeStatement;
	using SageInterface::setOneSourcePositionForTransformation;
    
    using SageInterface::replaceStatement;
    using SageInterface::prependStatement;
	using SageInterface::appendStatement;

    using SageInterface::addTextForUnparser;
    using SageInterface::attachComment;
    using namespace SageBuilder;
    
    switch (node->variantT())
    {
        case V_SgVariableDeclaration:
        {
            SgScopeStatement* scope = getScope (node);    

            SgVariableDeclaration * variableDeclaration =
              isSgVariableDeclaration (node);
            
            string const variableName = variableDeclaration->get_variables ().front ()->get_name ().getString ();
            
            SgType* variableType = variableDeclaration->get_decl_item (variableName)->get_type ();
            
            if (isSgTypedefType (variableType) != NULL
                && iequals (isSgTypedefType (variableType)->get_name ().getString (), OP2::OP_SET))
            {
                OpSubSetDefinition* opSubSet;
                try {
                    opSubSet = declarations->getOpSubSetDefinition (variableName);
                } catch (string name) {
                    return ;
                }
                    
                    Debug::getInstance ()->debugMessage ("Patching op_decl_subset: '" + variableName + "'" , Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
                    
                    string const originSetName = opSubSet->getOriginSetName ();
                    OpSetDefinition* opOriginSet = declarations->getOpSetDefinition (originSetName);
                                        
                    SgVariableDeclaration* subCount = buildVariableDeclaration(variableName + "_count", buildIntType (), buildAssignInitializer (buildIntVal (0), buildIntType ()), scope);
                 
                    SgCastExp* ce =
                      buildCastExp (
                         buildFunctionCallExp (
                           SgName("malloc"),
                           buildPointerType (buildIntType ()),
                           buildExprListExp (
                             buildMultiplyOp (
                               buildVarRefExp (SgName (opOriginSet->getDimensionName ()), scope),
                               buildSizeOfOp ( buildIntType ()))),
                           scope),
                           buildPointerType (buildIntType ()),
                           SgCastExp::e_C_style_cast);
                    
                    SgVariableDeclaration* subFlag =
                        buildVariableDeclaration(
                          variableName + "_flag",
                          buildPointerType (buildIntType ()),
                          buildAssignInitializer (
                            ce,
                            buildPointerType (buildIntType ())),
                          scope);
                    attachComment (subFlag, "Host data for subset flag.");
                
                
                    SgVariableDeclaration* subFlagDat =
                      buildVariableDeclaration (
                        variableName + "_flag_op_dat",
                        buildOpaqueType (OP2::OP_DAT, scope),
                        buildAssignInitializer (
                          buildFunctionCallExp (
                            SgName (OP2::OP_DECL_DAT),
                            declarations->getOpDatType (),
                            buildExprListExp (
                              buildVarRefExp (SgName (originSetName), scope),
                              buildIntVal (1),
							  buildStringVal ("int"),
                              buildVarRefExp (variableName + "_flag", scope),
							  buildStringVal (variableName + "_flag_op_dat")),
                            scope)),
                        scope);
                    attachComment (subFlagDat, "op dat for subset flag.");    
                
                    
                    SgExprListExp* filterLoopCallParams = buildExprListExp ();
                    
                    filterLoopCallParams->append_expression (buildFunctionRefExp (static_cast<CPPOxfordOpSubSetDefinition*> (opSubSet)->getFilterWrapperFunction ()->get_declaration ()));
				
					filterLoopCallParams->append_expression (buildStringVal (static_cast<CPPOxfordOpSubSetDefinition*> (opSubSet)->getFilterWrapperFunction ()->get_declaration ()->get_name ().getString ()));
                    
                    filterLoopCallParams->append_expression (buildOpaqueVarRefExp (originSetName, scope));
                
                for (int i = 0; i < opSubSet->getNbFilterArg (); i++) 
                {
                    filterLoopCallParams->append_expression (isSgExpression (
                      static_cast<CPPOxfordOpSubSetDefinition*> (opSubSet)->getOpArgDat (i)));
                }
                    
				
				SgEnumVal * eval_OP_WRITE = buildEnumVal_nfi (1, declarations->getOpAccessEnumDeclaration (), SgName("OP_WRITE"));
				setOneSourcePositionForTransformation (eval_OP_WRITE);

				SgEnumVal * eval_OP_INC = buildEnumVal_nfi (3, declarations->getOpAccessEnumDeclaration (), SgName("OP_INC"));
				setOneSourcePositionForTransformation (eval_OP_INC);				
				
                filterLoopCallParams->append_expression (
                  buildFunctionCallExp (
                    SgName ("op_arg_dat"),
                    buildVoidType (), // Dummy not real type
                    buildExprListExp (
                      buildVarRefExp (variableName + "_flag_op_dat", scope),
                      buildIntVal (0),
                      buildOpaqueVarRefExp ("OP_ID", scope),
					  buildIntVal (1),
					  buildStringVal ("int"),
					  eval_OP_WRITE
                    ),
                    scope
                  )
                );

                filterLoopCallParams->append_expression (
                  buildFunctionCallExp (
                    SgName ("op_arg_gbl"),
                    buildVoidType (), // Dummy not real type
                    buildExprListExp (
                      buildAddressOfOp(buildVarRefExp (variableName + "_count", scope)),
					  buildIntVal (1),
					  buildStringVal ("int"),
                      eval_OP_INC
                    ),
                    scope
                  )                                     
                );
                
                SgFunctionCallExp* filterLoopCall =
                  buildFunctionCallExp (
                    SgName ("op_par_loop"),
                    buildVoidType (),
                    filterLoopCallParams,
                    scope
                  );
				attachComment (filterLoopCall, "flagging and counting elements of the subset.");
                
                SgVariableDeclaration * nSubSet =
                  buildVariableDeclaration (
                    variableName + "_new",
                    declarations->getOpSetType (),
                    buildAssignInitializer (
                      buildFunctionCallExp(
                        SgName ("op_decl_set"),
                        declarations->getOpSetType (),
                        buildExprListExp (
                          buildVarRefExp (
                            variableName + "_count",
                            scope
                          ),
						  buildStringVal (variableName + "_new")
                        ),
                        scope
                      ),
                      declarations->getOpSetType ()
                    ),
                    scope
                  );
				
				SgFunctionCallExp * fetchFlag =
				  buildFunctionCallExp (
					SgName ("op_fetch_data"),
					buildVoidType (),
					buildExprListExp (
					buildVarRefExp (
					  variableName + "_flag_op_dat",
					  scope
					)
					),
					scope
				  );
                
                SgVariableDeclaration* mapArray =
                buildVariableDeclaration(
                  "p_" + variableName,
                  buildPointerType (buildIntType ()),
                  buildAssignInitializer (
                    ce,
                    buildPointerType (buildIntType ())),
                  scope);

                SgVariableDeclaration * ivar =
                  buildVariableDeclaration (
                    variableName + "_i",
                    buildIntType (),
                    buildAssignInitializer (
                      buildIntVal (0),
                      buildIntType ()
                    ),
                    scope);
                
                SgVariableDeclaration * cvar =
                  buildVariableDeclaration (
                    variableName + "_c",
                    buildIntType (),
                    buildAssignInitializer (
                      buildIntVal (0),
                      buildIntType ()
                    ),
                    scope);

                
                SgStatement* loopBody =
                  buildIfStmt (
                    buildExprStatement (
                      buildPntrArrRefExp (
                        buildVarRefExp (variableName + "_flag", scope),
                        buildVarRefExp (variableName + "_i", scope)
                      )
                    ),
                    buildAssignStatement (
                      buildPntrArrRefExp (
                        buildVarRefExp ("p_" + variableName, scope),
                        buildPlusPlusOp(buildVarRefExp(variableName + "_c" ,scope))
                      ),
                      buildVarRefExp (variableName + "_i", scope)
                    ),
                    NULL
                  );
                
                SgForStatement* forLoop =
                  buildForStatement (
                    NULL,
                    buildExprStatement (
                      buildLessThanOp (
                        buildVarRefExp(variableName + "_i" ,scope),
                        buildVarRefExp(opOriginSet->getDimensionName (), scope)
                    )),
                    buildPlusPlusOp(buildVarRefExp(variableName + "_i" ,scope)),
                    loopBody
                  );
                
                
                SgVariableDeclaration * mapDec =
                  buildVariableDeclaration (
                    "op_p_" + variableName,
                    declarations->getOpMapType (),
                    buildAssignInitializer (
                      buildFunctionCallExp (
                        "op_decl_map",
                        declarations->getOpMapType (),
                        buildExprListExp (
                          buildVarRefExp (variableName + "_new", scope),
                          buildVarRefExp (originSetName, scope),
                          buildIntVal (1),
                          buildOpaqueVarRefExp ("p_" + variableName, scope),
						  buildStringVal ("op_p_" + variableName)
                        ),
                        scope),
                        declarations->getOpMapType ()),
                    scope
                  );
				attachComment (mapDec, "1to0 mapping from subelements set to original set.");
                
                std::vector< SgStatement * > block;
                block.push_back (subCount);
                block.push_back (subFlag);
                block.push_back (subFlagDat);
                block.push_back (buildExprStatement (filterLoopCall));
				block.push_back (buildExprStatement (fetchFlag));
                block.push_back (nSubSet);
                block.push_back (mapArray);
                block.push_back (ivar);
                block.push_back (cvar);
                block.push_back (forLoop);
                block.push_back (mapDec);
                
                insertStatementListAfter(isSgStatement (node), block);
                removeStatement (isSgStatement (node), false); 
                
            }
            break;
        }
            
        case V_SgFunctionCallExp:
        {
            SgScopeStatement* scope = getScope (node);    

            SgFunctionCallExp* functionCallExp =
              isSgFunctionCallExp (node);
            
            string functionName =
              functionCallExp->getAssociatedFunctionSymbol ()->get_name ().getString ();
            
            if (iequals (functionName, OP2::OP_PAR_LOOP))
            {   
                SgVarRefExp * setNameExp = 
                  isSgVarRefExp (
                    functionCallExp->get_args ()->get_expressions ()[2]);
                
                OpSubSetDefinition* opSubSet;
                try {
                    opSubSet = declarations->getOpSubSetDefinition (
                      setNameExp->get_symbol ()->get_name ().getString ());
                    
                    Debug::getInstance ()->debugMessage ("Patching op_par_loop, replacing subset: '" + setNameExp->get_symbol ()->get_name ().getString () + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
                    
                    functionCallExp->get_args ()->get_expressions ()[2] = 
                    buildVarRefExp (setNameExp->get_symbol ()->get_name ().getString () + "_new",
                                    scope);

							  for (int i = 3; i < functionCallExp->get_args ()->get_expressions ().size (); i++)
							  {
								  // parsing op_arg_dat || op_arg_gbl calls
								  SgFunctionCallExp *
								  opArg = isSgFunctionCallExp (functionCallExp->get_args ()->get_expressions ()[i]);
								  if (iequals (
											   opArg->getAssociatedFunctionSymbol ()->get_name ().getString (),
												"op_arg_dat"))
								  {
									  if ( isSgVarRefExp (opArg->get_args ()->get_expressions ()[2]) == NULL )
									  {
										  opArg->get_args ()->get_expressions ()[2] =
										    buildVarRefExp (
												 "op_p_" + setNameExp->get_symbol ()->get_name ().getString (),
										      scope  
										    );
									  } else
									  {
										  Debug::getInstance ()->debugMessage ("Indirect mapping flatening not supported yet: " + opArg->get_args ()->get_expressions ()[2]->unparseToString() + "'.", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
									  }
								  } else {
									  // ignore op_arg_gbl
								  }
							  }
                    // TODO: replace ID MAP by the fake map.
                    
                } catch (string name) {
                    break ;
                }
            }
            break;
        }
			
		case V_SgFunctionDeclaration:
		{
			SgFunctionDeclaration* decl = isSgFunctionDeclaration (node);
			std::string filterUserFuncName = decl->get_name ().getString ();
			
			std::map <std::string, OpSubSetDefinition*>::const_iterator it;
			
			OpSubSetDefinition* subSetDefinition = NULL;
			for (it = declarations->firstOpSubSetDefinition ();
				 it != declarations->lastOpSubSetDefinition ();
				 it++)
			{
				if (iequals (it->second->getFilterKernelName (), filterUserFuncName))
				{
					subSetDefinition = it->second;
					break;
				}
			}
			
			if (subSetDefinition)
			{
				Debug::getInstance ()->debugMessage ("Patching subset filter function: '" + filterUserFuncName + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
				
				SgScopeStatement* scope = getScope (node);
				
				SgFunctionParameterList * params = buildFunctionParameterList ();
				
				for (SgInitializedNamePtrList::iterator it = decl->get_args ().begin (); it != decl->get_args ().end (); it++)
				{
					params->append_arg (buildInitializedName ((*it)->get_name (), (*it)->get_type ()));

				}
				
				params->append_arg (buildInitializedName ("is", buildPointerType (buildIntType ())));
				params->append_arg (buildInitializedName ("isc", buildPointerType (buildIntType ())));
				
				SgFunctionDeclaration * filterFunc = 
				  buildDefiningFunctionDeclaration (
				    SgName ("_op_subset_filter_" + filterUserFuncName),
					buildVoidType (),
				    params,
					scope
					);
				
				
				SgBasicBlock * subroutineScope = filterFunc->get_definition ()->get_body ();
				
				SgExprListExp * paramsExp = buildExprListExp ();

				for (SgInitializedNamePtrList::iterator it = decl->get_args ().begin (); it != decl->get_args ().end (); it++)
				{
					paramsExp->append_expression (buildOpaqueVarRefExp ((*it)->get_name (), subroutineScope));
				}
				
				appendStatement (
				  buildVariableDeclaration (
					"_t",
					buildIntType (),
					buildAssignInitializer (
					  buildConditionalExp (
						buildFunctionCallExp (
						  SgName (filterUserFuncName),
						  buildBoolType (),
						  paramsExp,
						  subroutineScope
						),
						buildIntVal (1),
						buildIntVal (0)
					  ),
					  buildIntType ()),
					subroutineScope
				   ),
				  subroutineScope);
				
				appendStatement (
				  buildExprStatement (
				    buildAssignOp (
					  buildPointerDerefExp (buildOpaqueVarRefExp ("is", subroutineScope)),
					  buildOpaqueVarRefExp ("_t", subroutineScope)
				    )
				  ),
				  subroutineScope);

				appendStatement (
				  buildExprStatement (
					buildPlusAssignOp (
					  buildPointerDerefExp (buildOpaqueVarRefExp ("isc", subroutineScope)),
					  buildOpaqueVarRefExp ("_t", subroutineScope)
				    )
				  ),
				  subroutineScope);

				attachComment (filterFunc, "Wrapper function for the filter function");
				appendStatement (filterFunc, scope);
				
				static_cast<CPPOxfordOpSubSetDefinition*> (subSetDefinition)->setFilterWrapperFunction (filterFunc->get_definition ());
			}
			
			break;
		}
        
        default:
        {
            break;
        }
    }
}

CPPPreProcess::CPPPreProcess (SgProject* project, CPPProgramDeclarationsAndDefinitions * declarations) : declarations (declarations)
{
    Debug::getInstance ()->debugMessage ("Preprocessing OP2 code'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
        traverse (project, preorder);

//    Debug::getInstance ()->debugMessage ("Preprocessing OP2 code -- 2nd pass'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
//            traverse (project, preorder);
}