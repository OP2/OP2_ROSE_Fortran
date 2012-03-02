


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

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>

#include "CPPSyntacticFusion.h"
#include "CPPProgramDeclarationsAndDefinitions.h"
#include "CPPOP2Definitions.h"
#include "Debug.h"
#include "Exceptions.h"
#include "OP2.h"

#include "Globals.h"

void
CPPSyntacticFusion::visit (SgNode* node)
{
}

void
CPPSyntacticFusion::generateFusedKernel (SgFunctionDeclaration* subroutine1, SgFunctionDeclaration* subroutine2) {
	using std::string;
	using SageInterface::getScope;
	using SageInterface::appendStatement;
	using SageInterface::appendStatementList;
	using SageInterface::insertStatementAfter;
	using namespace SageBuilder;
	
	int i;
	
	SgScopeStatement* scope = getScope (subroutine2); // TODO : FIX: this assume the first kernel is defined before the second...
	
	SgFunctionParameterList * params = buildFunctionParameterList ();
	
	i = 0;
	for (SgInitializedNamePtrList::iterator it = subroutine1->get_args ().begin (); it != subroutine1->get_args ().end (); it++)
	{
		params->append_arg (buildInitializedName ("k1a" + boost::lexical_cast<string> (i), (*it)->get_type ()));
		i++;
	}

	i = 0;
	for (SgInitializedNamePtrList::iterator it = subroutine2->get_args ().begin (); it != subroutine2->get_args ().end (); it++)
	{
		params->append_arg (buildInitializedName ("k2a" + boost::lexical_cast<string> (i), (*it)->get_type ()));
		i++;
	}
	
	SgFunctionDeclaration * fusedSubroutine =
	buildDefiningFunctionDeclaration (
									  subroutine1->get_name ().getString () + "__" + subroutine2->get_name ().getString () + "__fused",
									  buildVoidType (),
									  params,
									  scope);
	
	fusedSubroutine->get_functionModifier ().setInline ();
	  
	SgBasicBlock * body = fusedSubroutine->get_definition ()->get_body ();
	
	SgBasicBlock * inner1 = buildBasicBlock ();
	SgBasicBlock * inner2 = buildBasicBlock ();
	
	SgScopeStatement* innerScope1 = getScope (inner1);
	SgScopeStatement* innerScope2 = getScope (inner2);
	
	i = 0;
	for (SgInitializedNamePtrList::iterator it = subroutine1->get_args ().begin (); it != subroutine1->get_args ().end (); it++)
	{
		inner1->append_statement (buildVariableDeclaration ((*it)->get_name ().getString (), (*it)->get_type (), buildAssignInitializer (buildOpaqueVarRefExp ("k1a" + boost::lexical_cast<string> (i), innerScope1), (*it)->get_type ()), innerScope1));
		i++;
	}
	appendStatement (inner1, body);
	appendStatementList (subroutine1->get_definition ()->get_body ()->get_statements (), inner1);
	
	
	i = 0;
	for (SgInitializedNamePtrList::iterator it = subroutine2->get_args ().begin (); it != subroutine2->get_args ().end (); it++)
	{
		inner2->append_statement (buildVariableDeclaration ((*it)->get_name ().getString (),
															(*it)->get_type (),
															buildAssignInitializer (buildOpaqueVarRefExp ("k2a" + boost::lexical_cast<string> (i), innerScope2), (*it)->get_type ()), innerScope2));
		i++;
	}
	appendStatement (inner2, body);
	appendStatementList (subroutine2->get_definition ()->get_body ()->get_statements (), inner2);
		
	insertStatementAfter (subroutine2, fusedSubroutine, false);
}

void
CPPSyntacticFusion::removeOPParLoopCalls (ParallelLoop* parallelLoop)
{
	using SageInterface::removeStatement;
	
	for (std::vector <SgFunctionCallExp *>::const_iterator it = parallelLoop->getFirstFunctionCall ();
		 it != parallelLoop->getLastFunctionCall ();
		 it++)
	{
		removeStatement (isSgStatement ( (*it)->get_parent ()), false);
	}
}

void
CPPSyntacticFusion::fuseOPParLoopCalls (ParallelLoop* parallelLoop1, ParallelLoop* parallelLoop2)
{
	using SageInterface::getScope;
	using namespace SageBuilder;

	std::string kfname = parallelLoop1->getUserSubroutineName () + "__" + parallelLoop2->getUserSubroutineName () + "__fused";
	
	for (std::vector <SgFunctionCallExp *>::const_iterator it = parallelLoop1->getFirstFunctionCall ();
		  it != parallelLoop1->getLastFunctionCall ();
		  it++)
	{
		SgScopeStatement* scope = getScope ((*it));
		for (int i = 0; i < parallelLoop2->getNumberOfArgumentGroups (); i++)
		{
			for ( int j = 0; j < parallelLoop1->getNumberOfOpDatArgumentGroups (); i++)
			{
				
			}
			
			(*it)->append_arg ((*(parallelLoop2->getFirstFunctionCall ()))->get_args ()->get_expressions ()[i + 3]);
		}
		(*it)->get_args ()->get_expressions ()[0] = buildFunctionRefExp (kfname,
																		 scope);
		(*it)->get_args ()->get_expressions ()[1] = buildStringVal (kfname);

	}
}

void
CPPSyntacticFusion::fuseLoops (std::string const & subroutine1, std::string const & subroutine2)
{
	using boost::iequals;
	
	Debug::getInstance ()->debugMessage ("Syntactic Kernel Fusion :'" + subroutine1 + "' & '" + subroutine2 + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
	generateFusedKernel (declarations->getSubroutine (subroutine1), declarations->getSubroutine (subroutine2));
	
	removeOPParLoopCalls (declarations->getParallelLoop (subroutine2));
	fuseOPParLoopCalls (
						declarations->getParallelLoop (subroutine1),
						declarations->getParallelLoop (subroutine2));
}

CPPSyntacticFusion::CPPSyntacticFusion (SgProject* project, CPPProgramDeclarationsAndDefinitions * declarations) : project (project), declarations (declarations)
{
	using namespace SageBuilder;
	using SageInterface::appendStatement;
	using std::string;
	
	Debug::getInstance ()->debugMessage ("Syntactic Kernel Fusion :'" + Globals::getInstance ()->getSyntacticFusionKernels () + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
	
	std::vector<string> splits;
	string arg = Globals::getInstance ()->getSyntacticFusionKernels ();
	boost::split( splits, arg, boost::algorithm::is_any_of(":"));
	fuseLoops (splits[0], splits[1]);
}
