#include "CPPPreProcess.h"
#include "CPPParallelLoop.h"
#include "CPPProgramDeclarationsAndDefinitions.h"
#include "CPPOP2Definitions.h"
#include "Debug.h"
#include "Exceptions.h"
#include "OP2.h"

void
CPPPreProcess::visit (SgNode* node)
{
}

void
CPPPreProcess::generateSubsetDeclarationInlineFilter (std::string name, OpSubSetDefinition* opSubSet)
{
	using SageInterface::removeStatement;
	
	SgVariableDeclaration* subsetDeclaration = static_cast<CPPOxfordOpSubSetDefinition*> (opSubSet)->getSubsetDeclaration ();
	
	removeStatement (isSgStatement (subsetDeclaration), false);
}

void
CPPPreProcess::generateSubsetDeclarationPrecomputedSubSet (std::string name, OpSubSetDefinition* opSubSet)
{
	using std::string;
	using boost::iequals;
	using SageInterface::attachComment;
    using SageInterface::getScope;
    using SageInterface::insertStatement;
    using SageInterface::insertStatementListAfter;
    using SageInterface::insertStatementListBefore;
    using SageInterface::removeStatement;
	using SageInterface::setOneSourcePositionForTransformation;
	using namespace SageBuilder;
	
	SgScopeStatement* scope = getScope(static_cast<CPPOxfordOpSubSetDefinition*> (opSubSet)->getSubsetDeclaration ());
	
	string const originSetName = opSubSet->getOriginSetName ();
	OpSetDefinition* opOriginSet = declarations->getOpSetDefinition (originSetName);
	
	SgVariableDeclaration* subCount = buildVariableDeclaration(name + "_count", buildIntType (), buildAssignInitializer (buildIntVal (0), buildIntType ()), scope);
	attachComment (subCount, "Patching op_decl_subset: '" + name + "'");
	
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
							 name + "_flag",
							 buildPointerType (buildIntType ()),
							 buildAssignInitializer (
													 ce,
													 buildPointerType (buildIntType ())),
							 scope);
	attachComment (subFlag, "Host data for subset flag.");
	
	
	SgVariableDeclaration* subFlagDat =
	buildVariableDeclaration (
							  name + "_flag_op_dat",
							  buildOpaqueType (OP2::OP_DAT, scope),
							  buildAssignInitializer (
													  buildFunctionCallExp (
																			SgName (OP2::OP_DECL_DAT),
																			declarations->getOpDatType (),
																			buildExprListExp (
																							  buildVarRefExp (SgName (originSetName), scope),
																							  buildIntVal (1),
																							  buildStringVal ("int"),
																							  buildVarRefExp (name + "_flag", scope),
																							  buildStringVal (name + "_flag_op_dat")),
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
																					 buildVarRefExp (name + "_flag_op_dat", scope),
																					 buildIntVal (-1),
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
																					 buildAddressOfOp(buildVarRefExp (name + "_count", scope)),
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
							  name + "_new",
							  declarations->getOpSetType (),
							  buildAssignInitializer (
													  buildFunctionCallExp(
																		   SgName ("op_decl_set"),
																		   declarations->getOpSetType (),
																		   buildExprListExp (
																							 buildVarRefExp (
																											 name + "_count",
																											 scope
																											 ),
																							 buildStringVal (name + "_new")
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
															name + "_flag_op_dat",
															scope
															)
											),
						  scope
						  );
	
	SgVariableDeclaration* mapArray =
	buildVariableDeclaration(
							 "p_" + name,
							 buildPointerType (buildIntType ()),
							 buildAssignInitializer (
													 ce,
													 buildPointerType (buildIntType ())),
							 scope);
	
	SgVariableDeclaration * ivar =
	buildVariableDeclaration (
							  name + "_i",
							  buildIntType (),
							  buildAssignInitializer (
													  buildIntVal (0),
													  buildIntType ()
													  ),
							  scope);
	
	SgVariableDeclaration * cvar =
	buildVariableDeclaration (
							  name + "_c",
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
														 buildVarRefExp (name + "_flag", scope),
														 buildVarRefExp (name + "_i", scope)
														 )
									 ),
				 buildAssignStatement (
									   buildPntrArrRefExp (
														   buildVarRefExp ("p_" + name, scope),
														   buildPlusPlusOp(buildVarRefExp(name + "_c" ,scope), SgUnaryOp::postfix)
														   ),
									   buildVarRefExp (name + "_i", scope)
									   ),
				 NULL
				 );
	
	SgForStatement* forLoop =
	buildForStatement (
					   NULL,
					   buildExprStatement (
										   buildLessThanOp (
															buildVarRefExp(name + "_i" ,scope),
															buildVarRefExp(opOriginSet->getDimensionName (), scope)
															)),
					   buildPlusPlusOp(buildVarRefExp(name + "_i" ,scope)),
					   loopBody
					   );
	
	
	SgVariableDeclaration * mapDec =
	buildVariableDeclaration (
							  "op_p_" + name,
							  declarations->getOpMapType (),
							  buildAssignInitializer (
													  buildFunctionCallExp (
																			"op_decl_map",
																			declarations->getOpMapType (),
																			buildExprListExp (
																							  buildVarRefExp (name + "_new", scope),
																							  buildVarRefExp (originSetName, scope),
																							  buildIntVal (1),
																							  buildOpaqueVarRefExp ("p_" + name, scope),
																							  buildStringVal ("op_p_" + name)
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
	
	
	std::vector< SgStatement * > collapseLoopBodyStatements;
		
	for (std::map <string, OpMapDefinition *>::const_iterator it = declarations->firstOpMapDefinition ();
		 it != declarations->lastOpMapDefinition ();
		 ++it)
	{
		if (iequals (it->second->getSourceOpSetName (), originSetName))
		{
			Debug::getInstance ()->debugMessage ("Collapsing mapping '" + it->second->getMappingName () + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
			
			SgCastExp* ce =
			buildCastExp (
						  buildFunctionCallExp (
												SgName("malloc"),
												buildPointerType (buildIntType ()),
												buildExprListExp (buildMultiplyOp (
																				   buildVarRefExp (SgName (name + "_count"), scope),
																				   buildMultiplyOp (
																									buildIntVal (it->second->getDimension ()),
																									buildSizeOfOp ( buildIntType ())))),
																  scope),
												buildPointerType (buildIntType ()),
												SgCastExp::e_C_style_cast);
			
			SgVariableDeclaration* collaspedData =
			buildVariableDeclaration(
									 "p_" + name + "_2_" + originSetName + "_via_" + it->first,
									 buildPointerType (buildIntType ()),
									 buildAssignInitializer (
															 ce,
															 buildPointerType (buildIntType ())),
									 scope);
			attachComment (collaspedData, "Collapsing mapping '" +
						   name + "' -(" + "op_p_" + name + ")> '" +
						   originSetName + "' -(" + it->first +
						   ")> '" + it->second->getDestinationOpSetName () + "'.");
			block.push_back (collaspedData);
			
			for (int i = 0; i < it->second->getDimension (); i++) {
				SgExpression* indexp =
				buildAddOp (
							buildMultiplyOp (buildIntVal (it->second->getDimension ()), buildVarRefExp (name + "_i", scope)),
							buildIntVal (i));
				
				SgStatement* ass =
				buildAssignStatement (
									  buildPntrArrRefExp (buildVarRefExp ("p_" + name + "_2_" + originSetName + "_via_" + it->first, scope),
														  indexp),
									  buildPntrArrRefExp (
														  buildArrowExp (
																	   buildVarRefExp (it->first, scope),
																	   buildOpaqueVarRefExp ("map", scope)
																	   ),
														  buildAddOp (
																	  buildMultiplyOp (
																					   buildIntVal (it->second->getDimension ()),
																					   buildPntrArrRefExp (
																										   buildArrowExp (
																														  buildVarRefExp ("op_p_" + name, scope),
																														  buildOpaqueVarRefExp ("map", scope)
																														  ),
																										   buildVarRefExp (name + "_i", scope)
																										   )
																					   ),
																	  buildIntVal (i)
																	  )
														  )
									  );
				
				collapseLoopBodyStatements.push_back (ass);
			}
		}
	}
	
	SgBasicBlock* collapseLoopBody = buildBasicBlock_nfi (collapseLoopBodyStatements);
	setOneSourcePositionForTransformation (collapseLoopBody);
	
	SgForStatement* collapseLoop =
	buildForStatement (
					   buildAssignStatement (buildVarRefExp (name + "_i", scope), buildIntVal (0)),
					   buildExprStatement (
										   buildLessThanOp (
															buildVarRefExp(name + "_i" ,scope),
															buildVarRefExp(name + "_count", scope)
															)),
					   buildPlusPlusOp(buildVarRefExp(name + "_i" ,scope)),
					   collapseLoopBody);
	
	block.push_back (collapseLoop);
	
	for (std::map <string, OpMapDefinition *>::const_iterator it = declarations->firstOpMapDefinition ();
		 it != declarations->lastOpMapDefinition ();
		 ++it)
	{
		if (iequals (it->second->getSourceOpSetName (), originSetName))
		{
			SgVariableDeclaration * collapseMapDec =
			buildVariableDeclaration (
									  "op_p_" + name + "_2_" + originSetName + "_via_" + it->first,
									  declarations->getOpMapType (),
									  buildAssignInitializer (
															  buildFunctionCallExp (
																					"op_decl_map",
																					declarations->getOpMapType (),
																					buildExprListExp (
																									  buildVarRefExp (name + "_new", scope),
																									  buildVarRefExp (it->second->getDestinationOpSetName (), scope),
																									  buildIntVal (it->second->getDimension ()),
																									  buildVarRefExp ("p_" + name + "_2_" + originSetName + "_via_" + it->first, scope),
																									  buildStringVal ("op_p_" + name + "_2_" + originSetName + "_via_" + it->first)
																									  ),
																					scope),
															  declarations->getOpMapType ()),
									  scope
									  );
			attachComment (collapseMapDec, "Collapsed mapping declaration.");
			block.push_back (collapseMapDec);
		}
	}
	
	insertStatementListBefore(isSgStatement (static_cast<CPPOxfordOpSubSetDefinition*> (opSubSet)->getSubsetDeclaration ()), block);
	removeStatement (isSgStatement (static_cast<CPPOxfordOpSubSetDefinition*> (opSubSet)->getSubsetDeclaration ()), false);
}

void
CPPPreProcess::generateWrapperFunction (OpSubSetDefinition* opSubSet)
{
	using std::string;
	using SageInterface::appendStatement;
	using SageInterface::attachComment;
    using SageInterface::getScope;
    using SageInterface::getGlobalScope;
	using SageInterface::setOneSourcePositionForTransformation;
	using namespace SageBuilder;
	
	SgFunctionDeclaration* decl = static_cast<CPPOxfordOpSubSetDefinition*> (opSubSet)->getFilterFunction ()->get_declaration ();
	SgScopeStatement* scope = getGlobalScope (decl);
	
	std::string filterUserFuncName = opSubSet->getFilterKernelName ();
	
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
	filterFunc->get_functionModifier ().setInline ();
	
	
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
	
	static_cast<CPPOxfordOpSubSetDefinition*> (opSubSet)->setFilterWrapperFunction (filterFunc->get_definition ());
}

void
CPPPreProcess::generateKernelInlinedFilter (OpSubSetDefinition* opSubSet, SgFunctionCallExp * parLoop)
{
	using std::string;
	using SageInterface::appendStatement;
	using SageInterface::attachComment;
    using SageInterface::getScope;
    using SageInterface::getGlobalScope;
	using SageInterface::setOneSourcePositionForTransformation;
	using namespace SageBuilder;

	SgFunctionDeclaration* kernelDecl = isSgFunctionRefExp (parLoop->get_args ()->get_expressions ()[0])->get_symbol ()->get_declaration ();
	SgScopeStatement* scope = getGlobalScope (kernelDecl);
	
	SgFunctionParameterList * params = buildFunctionParameterList ();
	for (SgInitializedNamePtrList::iterator it = kernelDecl->get_args ().begin (); it != kernelDecl->get_args ().end (); it++)
	{
		params->append_arg (buildInitializedName ((*it)->get_name (), (*it)->get_type ()));
	}
	
	SgFunctionDeclaration* fitlerDecl = static_cast<CPPOxfordOpSubSetDefinition*> (opSubSet)->getFilterFunction ()->get_declaration ();
	for (SgInitializedNamePtrList::iterator it = fitlerDecl->get_args ().begin (); it != fitlerDecl->get_args ().end (); it++)
	{
		params->append_arg (buildInitializedName ((*it)->get_name (), (*it)->get_type ()));
	}
	
	SgFunctionDeclaration * kernelInlinedFilterDecl = 
	buildDefiningFunctionDeclaration (
									  "__" +
									  isSgFunctionRefExp (parLoop->get_args ()->get_expressions ()[0])->get_symbol ()->get_name ().getString () +
									  "_" + opSubSet->getFilterKernelName (),
									  buildVoidType (),
									  params,
									  scope
									  );
	kernelInlinedFilterDecl->get_functionModifier ().setInline ();
	
	SgBasicBlock * subroutineScope = kernelInlinedFilterDecl->get_definition ()->get_body ();
	
	SgExprListExp * filterFuncExpList = buildExprListExp ();
	for (SgInitializedNamePtrList::iterator it = fitlerDecl->get_args ().begin (); it != fitlerDecl->get_args ().end (); it++)
	{
		filterFuncExpList->append_expression (buildOpaqueVarRefExp ((*it)->get_name (), subroutineScope));
	}
	
	SgExprListExp * kernelFuncExpList = buildExprListExp ();
	for (SgInitializedNamePtrList::iterator it = kernelDecl->get_args ().begin (); it != kernelDecl->get_args ().end (); it++)
	{
		kernelFuncExpList->append_expression (buildOpaqueVarRefExp ((*it)->get_name (), subroutineScope));
	}
	
	appendStatement (
					 buildIfStmt (buildExprStatement (buildFunctionCallExp (
																					opSubSet->getFilterKernelName (),
																					buildBoolType (),
																					filterFuncExpList,
																					subroutineScope
																					)),
															  buildExprStatement (buildFunctionCallExp (
																					isSgFunctionRefExp (parLoop->get_args ()->get_expressions ()[0])->get_symbol ()->get_name ().getString (),
																					buildVoidType (),
																					kernelFuncExpList,
																					subroutineScope)),
															  NULL),
					 subroutineScope
					 );
	
	appendStatement (kernelInlinedFilterDecl, scope);
}

void
CPPPreProcess::handleOPParLoopInlineFilter (OpSubSetDefinition* opSubSet, SgFunctionCallExp * parLoop)
{
	using boost::iequals;
	using std::string;
	using SageInterface::getScope;
	using namespace SageBuilder;
	
	string name = isSgVarRefExp (parLoop->get_args ()->get_expressions ()[2])->get_symbol ()->get_name ().getString ();
	
	Debug::getInstance ()->debugMessage ("Patching op_par_loop on subset: '" + name + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
	
	SgScopeStatement* scope = getScope (parLoop);
	
	// replace kernel name
	string kfname = "__" + 
	isSgFunctionRefExp (parLoop->get_args ()->get_expressions ()[0])->get_symbol ()->get_name ().getString () +
	"_" + opSubSet->getFilterKernelName ();
	
	parLoop->get_args ()->get_expressions ()[0] = 
	buildFunctionRefExp (kfname,
						 scope);
	
	parLoop->get_args ()->get_expressions ()[1] = 
	buildStringVal (kfname);
	
	// replace subset with superset
	parLoop->get_args ()->get_expressions ()[2] = 
	buildVarRefExp (opSubSet->getOriginSetName (),
					scope);
		
	// appending the extra op_arg_dat for evaluation the filter function
	for (int i = 0; i < opSubSet->getNbFilterArg (); i++)
	{
		parLoop->get_args ()->get_expressions ().push_back (isSgFunctionCallExp (static_cast<CPPOxfordOpSubSetDefinition*> (opSubSet)->getOpArgDat (i)));
	}
}

void
CPPPreProcess::handleOPParLoopPrecomputedSubSet (OpSubSetDefinition* opSubSet, SgFunctionCallExp * parLoop)
{
	using boost::iequals;
	using std::string;
	using SageInterface::getScope;
	using namespace SageBuilder;
	
	string name = isSgVarRefExp (parLoop->get_args ()->get_expressions ()[2])->get_symbol ()->get_name ().getString ();

	Debug::getInstance ()->debugMessage ("Patching op_par_loop on subset: '" + name + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
	
	SgScopeStatement* scope = getScope (parLoop);
	
	parLoop->get_args ()->get_expressions ()[2] = 
	buildVarRefExp (name + "_new",
					scope);
	
	for (int i = 3; i < parLoop->get_args ()->get_expressions ().size (); i++)
	{
		// parsing op_arg_dat || op_arg_gbl calls
		SgFunctionCallExp *
		opArg = isSgFunctionCallExp (parLoop->get_args ()->get_expressions ()[i]);
		if (iequals (
					 opArg->getAssociatedFunctionSymbol ()->get_name ().getString (),
					 "op_arg_dat"))
		{
			if ( isSgVarRefExp (opArg->get_args ()->get_expressions ()[2]) == NULL )
			{
				opArg->get_args ()->get_expressions ()[2] =
				buildVarRefExp (
								"op_p_" + name,
								scope
								);
				// change (-1) for OP_ID to 0
				opArg->get_args ()->get_expressions ()[1] = buildIntVal (0);
			} else
			{
				string mappingName = isSgVarRefExp (opArg->get_args ()->get_expressions ()[2])->get_symbol ()->get_name ().getString ();
				
				string collapsedMappingName = "op_p_" + name + "_2_" + opSubSet->getOriginSetName() + "_via_" +  mappingName;
				
				opArg->get_args ()->get_expressions ()[2] = buildOpaqueVarRefExp (collapsedMappingName, scope);
			}
		} else {
			// ignore op_arg_gbl
		}
	}
}

void
CPPPreProcess::handleSubsetDeclarations ()
{
	using std::map;
	using std::vector;
	using std::string;
	
	for (std::map <string, OpSubSetDefinition *>::const_iterator it = declarations->firstOpSubSetDefinition ();
		 it != declarations->lastOpSubSetDefinition ();
		 ++it)
	{
		// must do filter function wrapper first to have the filter func definition at hand for the subset op_par_loop creation
		Debug::getInstance ()->debugMessage ("generating filter function wrapper for '" + it->second->getFilterKernelName () + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
		
		
		
		Debug::getInstance ()->debugMessage ("generating creation code for subset '" + it->first + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
		
		generateWrapperFunction (it->second);
		
		if (precompute) 
		{
			generateSubsetDeclarationPrecomputedSubSet (it->first, it->second);
		} else {
			generateSubsetDeclarationInlineFilter (it->first, it->second);
		}
		
	}
	
	
}

void
CPPPreProcess::handleOPParLoops ()
{
	using boost::iequals;
	using std::map;
	using std::vector;
	using std::string;
	using SageInterface::getScope;
	using namespace SageBuilder;

	
	for (map <string, ParallelLoop *>::const_iterator it =
		 declarations->firstParallelLoop (); it
		 != declarations->lastParallelLoop (); ++it)
	{
		for (vector <SgFunctionCallExp *>::const_iterator fit =
			 it->second->getFirstFunctionCall (); fit
			 != it->second->getLastFunctionCall (); ++fit)
		{
			string name = isSgVarRefExp ((*fit)->get_args ()->get_expressions ()[2])->get_symbol ()->get_name ().getString ();

			if (declarations->isOpSubSet (name))
			{	
				OpSubSetDefinition* opSubSet;
                try {
                    opSubSet = declarations->getOpSubSetDefinition (name);
				} catch (string ename)
				{
					Debug::getInstance ()->debugMessage ("Something went very wrong.", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
					break;
				}
				
				generateKernelInlinedFilter (opSubSet, *fit);
				
				if (precompute)
				{
					handleOPParLoopPrecomputedSubSet (opSubSet, *fit);
				} else {
					handleOPParLoopInlineFilter (opSubSet, *fit);
				}
			}
		}
	}
}

CPPPreProcess::CPPPreProcess (SgProject* project, CPPProgramDeclarationsAndDefinitions * declarations) : project (project), declarations (declarations), precompute (true),newFileName ("filteredKernels.h")
{
	using namespace SageBuilder;
	using SageInterface::appendStatement;

	/*
	SgSourceFile *sgfile = isSgSourceFile( buildFile("dtc.cpp","dtc.h",project) );
	SgGlobal* global = sgfile->get_globalScope();
	SgVariableDeclaration *varDecl = buildVariableDeclaration("i", buildIntType());
	appendStatement (varDecl,isSgScopeStatement(global));
	*/
	
    Debug::getInstance ()->debugMessage ("Preprocessing OP2 code'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
	handleSubsetDeclarations ();
	handleOPParLoops ();
//	traverse (project, postorder);
}