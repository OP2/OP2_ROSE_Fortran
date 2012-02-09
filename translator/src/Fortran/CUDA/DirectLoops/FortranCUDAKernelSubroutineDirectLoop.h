


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
#ifndef FORTRAN_CUDA_KERNEL_SUBROUTINE_DIRECT_LOOP_H
#define FORTRAN_CUDA_KERNEL_SUBROUTINE_DIRECT_LOOP_H

#include <FortranCUDAKernelSubroutine.h>

class FortranCUDAKernelSubroutineDirectLoop: public FortranCUDAKernelSubroutine
{
  private:

    virtual SgStatement *
    createUserSubroutineCallStatement ();

    virtual void
    createExecutionLoopStatements ();

    /*
     * ======================================================
     * Builds the statements which stages in data from device
     * memory into shared memory for the OP_DAT in this
     * argument group
     * ======================================================
     */
    SgFortranDo *
    createStageInFromDeviceMemoryToSharedMemoryStatements (
        unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Builds the statements which stages in data from shared
     * memory into local memory for the OP_DAT in this
     * argument group
     * ======================================================
     */
    SgFortranDo *
    createStageInFromSharedMemoryToLocalMemoryStatements (
        unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Builds the statements which stages out data from shared
     * memory into device memory for the OP_DAT in this
     * argument group
     * ======================================================
     */
    SgFortranDo *
    createStageOutFromSharedMemoryToDeviceMemoryStatements (
        unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Builds the statements which stages out data from local
     * memory into shared memory for the OP_DAT in this
     * argument group
     * ======================================================
     */
    SgFortranDo *
    createStageOutFromLocalMemoryToSharedMemoryStatements (
        unsigned int OP_DAT_ArgumentGroup);

    void
    createInitialiseOffsetIntoCUDASharedVariableStatements ();

    void
    createThreadIDInitialisationStatement ();

    virtual void
    createOpDatFormalParameterDeclarations ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    FortranCUDAKernelSubroutineDirectLoop (SgScopeStatement * moduleScope,
        FortranCUDAUserSubroutine * userSubroutine,
        FortranParallelLoop * parallelLoop,
        FortranReductionSubroutines * reductionSubroutines,
        FortranCUDAOpDatCardinalitiesDeclaration * cardinalitiesDeclaration,
        FortranOpDatDimensionsDeclaration * dimensionsDeclaration,
        FortranCUDAModuleDeclarations * moduleDeclarations);
};

#endif
