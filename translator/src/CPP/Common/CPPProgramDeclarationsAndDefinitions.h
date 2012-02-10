


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
#ifndef CPP_PROGRAM_DECLARATIONS_AND_DEFINITIONS_H
#define CPP_PROGRAM_DECLARATIONS_AND_DEFINITIONS_H

#include <ProgramDeclarationsAndDefinitions.h>

class CPPParallelLoop;

class CPPProgramDeclarationsAndDefinitions: public ProgramDeclarationsAndDefinitions <
    SgFunctionDeclaration> ,
    public AstSimpleProcessing
{
  private:

    std::map <unsigned int, std::string> opAccessDescriptors;
    
    SgType * op_dat_type;
    SgType * op_set_type;
    SgType * op_map_type;
    SgType * op_access_type;
    SgEnumDeclaration * opAccessEnumDeclaration;

  private:

    void
    setOpGblProperties (CPPParallelLoop * parallelLoop,
        std::string const & variableName, int OP_DAT_ArgumentGroup);

    void
    setOpDatProperties (CPPParallelLoop * parallelLoop,
        std::string const & variableName, int OP_DAT_ArgumentGroup);

    void
    setParallelLoopAccessDescriptor (CPPParallelLoop * parallelLoop,
        SgExprListExp * actualArguments, unsigned int OP_DAT_ArgumentGroup,
        unsigned int argumentPosition);

    void
    handleImperialOpGblArgument (CPPParallelLoop * parallelLoop,
        SgExprListExp * actualArguments, unsigned int OP_DAT_ArgumentGroup);

    void
    handleOxfordOpGblArgument (CPPParallelLoop * parallelLoop,
        SgExprListExp * actualArguments, unsigned int OP_DAT_ArgumentGroup);

    void
    handleImperialOpDatArgument (CPPParallelLoop * parallelLoop,
        SgExprListExp * actualArguments, unsigned int OP_DAT_ArgumentGroup);

    void
    handleOxfordOpDatArgument (CPPParallelLoop * parallelLoop,
        SgExprListExp * actualArguments, unsigned int OP_DAT_ArgumentGroup);

    void
    analyseParallelLoopArguments (CPPParallelLoop * parallelLoop,
        SgExprListExp * actualArguments);

    void
    detectAndHandleOP2Definition (SgVariableDeclaration * variableDeclaration,
        std::string const variableName, SgTypedefType * typeDefinition);

    virtual void
    visit (SgNode * node);

  public:

    CPPProgramDeclarationsAndDefinitions (SgProject * project);
    
    SgType *
    getOpDatType ();

    SgType *
    getOpSetType ();

    SgType *
    getOpMapType ();
    
    SgType *
    getOpAccessType ();
    
    SgEnumDeclaration *
    getOpAccessEnumDeclaration ();
};

#endif
