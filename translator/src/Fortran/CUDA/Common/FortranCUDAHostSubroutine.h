


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
#ifndef FORTRAN_CUDA_HOST_SUBROUTINE_H
#define FORTRAN_CUDA_HOST_SUBROUTINE_H

#include <FortranHostSubroutine.h>

class FortranKernelSubroutine;
class FortranCUDAOpDatCardinalitiesDeclaration;
class FortranOpDatDimensionsDeclaration;
class FortranCUDAModuleDeclarations;

class FortranCUDAHostSubroutine: public FortranHostSubroutine
{
  protected:

    FortranCUDAOpDatCardinalitiesDeclaration * cardinalitiesDeclaration;

    FortranOpDatDimensionsDeclaration * dimensionsDeclaration;

    FortranCUDAModuleDeclarations * moduleDeclarations;

  protected:

    void
    createProfilingVariablesDeclaration ();

    void
    initialiseProfilingVariablesDeclaration ();

    void
    createStartTimerHost ();

    void
    createEndTimerHost ();

    void
    createStartTimerKernel ();

    void
    createEndTimerKernel ();

    void
    createEndTimerSynchroniseHost ();

    void
    createEndTimerSynchroniseKernel ();

    void
    createElapsedTimeHost ();

    void
    createElapsedTimeKernel ();

    void
    createAccumulateTimesHost ();

    void
    createAccumulateTimesKernel ();

    virtual void
    createReductionEpilogueStatements ();

    virtual void
    createReductionPrologueStatements ();

    virtual void
    createReductionDeclarations ();

    SgExpression *
    getOpDatCardinalityInitialisationExpression (SgScopeStatement * scope,
        unsigned int OP_DAT_ArgumentGroup);

    virtual SgBasicBlock *
    createTransferOpDatStatements ();

    SgBasicBlock *
    createDeallocateStatements ();

    void
    createCUDAConfigurationLaunchDeclarations ();

    void
    createDataMarshallingDeclarations ();

    void
    createOpDatCardinalitiesDeclaration ();

    void
    createOpDatDimensionsDeclaration ();

    void
    createIterationVariablesDeclarations ();

    void
    appendBlockSizeFunctionCall (SgScopeStatement * scope);
    
    FortranCUDAHostSubroutine (SgScopeStatement * moduleScope,
        FortranKernelSubroutine * kernelSubroutine,
        FortranParallelLoop * parallelLoop,
        FortranCUDAOpDatCardinalitiesDeclaration * cardinalitiesDeclaration,
        FortranOpDatDimensionsDeclaration * dimensionsDeclaration,
        FortranCUDAModuleDeclarations * moduleDeclarations);
};

#endif
