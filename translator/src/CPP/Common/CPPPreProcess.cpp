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
    
    using SageInterface::replaceStatement;
    using SageInterface::prependStatement;

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
                    attachComment (subFlag, "Host data for subset flag");
                
                
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
                              buildVarRefExp (variableName + "_flag", scope)),
                            scope)),
                        scope);
                    attachComment (subFlagDat, "op dat for subset flag");    
                
                    
                    SgExprListExp* filterLoopCallParams = buildExprListExp ();
                    
                    filterLoopCallParams->append_expression (buildFunctionRefExp (declarations->getSubroutine (opSubSet->getFilterKernelName ())));
                    
                    filterLoopCallParams->append_expression (buildOpaqueVarRefExp (originSetName, scope));
                
                for (int i = 0; i < opSubSet->getNbFilterArg (); i++) 
                {
                    filterLoopCallParams->append_expression (isSgExpression (
                      static_cast<CPPImperialOpSubSetDefinition*> (opSubSet)->getOpArgDat (i)));
                }
                    
                filterLoopCallParams->append_expression (
                  buildFunctionCallExp (
                    SgName ("op_arg_dat"),
                    buildVoidType (), // Dummy not real type
                    buildExprListExp (
                      buildVarRefExp (variableName + "_flag_op_dat", scope),
                      buildIntVal (0),
                      buildOpaqueVarRefExp ("OP_ID", scope),
                      buildCastExp (buildIntVal (1), declarations->getOpAccessType (), SgCastExp::e_C_style_cast) // OP_WRITE
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
                      buildCastExp (buildIntVal (3), declarations->getOpAccessType (), SgCastExp::e_C_style_cast) // OP_INC
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
                          )
                        ),
                        scope
                      ),
                      declarations->getOpSetType ()
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
                          buildIntVal (0),
                          buildOpaqueVarRefExp ("p_" + variableName, scope)
                        ),
                        scope),
                        declarations->getOpMapType ()),
                    scope
                  );
                
                std::vector< SgStatement * > block;
                block.push_back (subCount);
                block.push_back (subFlag);
                block.push_back (subFlagDat);
                block.push_back (buildExprStatement(filterLoopCall));
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
                Debug::getInstance ()->debugMessage ("Traversing : " + functionCallExp->unparseToString () + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
                
                SgVarRefExp * setNameExp = 
                  isSgVarRefExp (
                    functionCallExp->get_args ()->get_expressions ()[1]);
                
                OpSubSetDefinition* opSubSet;
                try {
                    opSubSet = declarations->getOpSubSetDefinition (
                      setNameExp->get_symbol ()->get_name ().getString ());
                    
                    Debug::getInstance ()->debugMessage ("Patching op_par_loop, replacing subset: '" + setNameExp->get_symbol ()->get_name ().getString () + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
                    
                    functionCallExp->get_args ()->get_expressions ()[1] = 
                    buildVarRefExp (setNameExp->get_symbol ()->get_name ().getString () + "_new",
                                    scope);

                    // TODO: replace ID MAP by the fake map.
                    
                } catch (string name) {
                    break ;
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

CPPPreProcess::CPPPreProcess (SgProject* project, CPPProgramDeclarationsAndDefinitions * declarations) : declarations (declarations)
{
    Debug::getInstance ()->debugMessage ("Preprocessing OP2 code'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
//    traverseInputFiles (project, preorder);
        traverse (project, preorder);

    Debug::getInstance ()->debugMessage ("Preprocessing OP2 code -- 2nd pass'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
            traverse (project, preorder);
}