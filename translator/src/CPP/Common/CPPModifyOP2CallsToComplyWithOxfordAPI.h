


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


#pragma once
#ifndef CPP_MODIFY_OP2_CALLS_TO_COMPLY_WITH_OXFORD_API_H
#define CPP_MODIFY_OP2_CALLS_TO_COMPLY_WITH_OXFORD_API_H

#include <rose.h>

class CPPProgramDeclarationsAndDefinitions;

class CPPModifyOP2CallsToComplyWithOxfordAPI: public AstSimpleProcessing
{
  private:

    CPPProgramDeclarationsAndDefinitions * declarations;

  private:

#ifdef INCLUDE
    void
    patchOpArgGblCall (SgExpressionPtrList & actualArguments);

    void
    patchOpArgDatCall (SgExpressionPtrList & actualArguments);

    void
    patchOpParLoopCall (SgExpressionPtrList & actualArguments);

    void
    patchOpDeclareConstCall (SgExpressionPtrList & actualArguments);

    void
    patchOpDeclareDatCall (SgVariableDeclaration * variableDeclaration,
        std::string const variableName);

    void
    patchOpDeclareMapCall (SgVariableDeclaration * variableDeclaration,
        std::string const variableName);

    void
    patchOpDeclareSetCall (SgVariableDeclaration * variableDeclaration,
        std::string const variableName);

    virtual void
    visit (SgNode * node);
#endif

  public:

    CPPModifyOP2CallsToComplyWithOxfordAPI (SgProject * project,
        CPPProgramDeclarationsAndDefinitions * declarations);
};

#endif
