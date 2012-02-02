


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
#ifndef FORTRAN_CUDA_KERNEL_SUBROUTINE_H
#define FORTRAN_CUDA_KERNEL_SUBROUTINE_H

#include <FortranKernelSubroutine.h>

class FortranCUDAUserSubroutine;
class FortranCUDAOpDatCardinalitiesDeclaration;
class FortranCUDAModuleDeclarations;
class FortranOpDatDimensionsDeclaration;
class FortranReductionSubroutines;

class FortranCUDAKernelSubroutine: public FortranKernelSubroutine
{
  protected:

    FortranCUDAOpDatCardinalitiesDeclaration * cardinalitiesDeclaration;

    FortranOpDatDimensionsDeclaration * dimensionsDeclaration;

    FortranCUDAModuleDeclarations * moduleDeclarations;

  protected:

    /*
     * ======================================================
     * Creates the statements initialising the local thread
     * variable (0 for OP_INC, actual values for OP_MAX and
     * OP_MIN)
     * ======================================================
     */    
    void
    createReductionLocalVariableInitialisation ();
    
    /*
     * ======================================================
     * Creates the statements executed after the call to the
     * user subroutine to perform the thread-block reduction
     * ======================================================
     */
    void
    createReductionEpilogueStatements ();

    /*
     * ======================================================
     * Creates the variable declarations needed to stage in
     * data from device->shared->stack memory
     * ======================================================
     */
    void
    createCUDAStageInVariablesVariableDeclarations ();

    /*
     * ======================================================
     * Creates the variable declarations needed in CUDA
     * shared memory
     * ======================================================
     */
    void
    createCUDASharedVariableDeclarations ();

    FortranCUDAKernelSubroutine (SgScopeStatement * moduleScope,
        FortranCUDAUserSubroutine * userSubroutine,
        FortranParallelLoop * parallelLoop,
        FortranReductionSubroutines * reductionSubroutines,
        FortranCUDAOpDatCardinalitiesDeclaration * cardinalitiesDeclaration,
        FortranOpDatDimensionsDeclaration * dimensionsDeclaration,
        FortranCUDAModuleDeclarations * moduleDeclarations);
};

#endif
