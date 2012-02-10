


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


/*
 * Written by Adam Betts and Carlo Bertolli
 */

#pragma once
#ifndef FORTRAN_PARALLEL_LOOP_H
#define FORTRAN_PARALLEL_LOOP_H

#include <ParallelLoop.h>

class FortranParallelLoop: public ParallelLoop
{
  public:

    /*
     * ======================================================
     * The following constants assume that each call to an
     * OP_PAR_LOOP in Fortran is constructed as follows:
     *
     * OP_PAR_LOOP_<N>
     *   (nameOfUserSubroutine,
     *    OP_SET,
     *    OP_DAT,   int,   OP_MAP,   OP_ACCESS,
     *    ...
     *    OP_DAT_N, int_N, OP_MAP_N, OP_ACCESS_N)
     * ======================================================
     */

    /*
     * ======================================================
     * In the standard version, for every OP_DAT, there is an
     * int, an OP_MAP, and an OP_ACCESS
     * In the generic version, , for every OP_DAT, there is an
     * int, an OP_MAP, another int (dimension), a string
     * (type), and an OP_ACCESS
     * ======================================================
     */
    static unsigned int const NUMBER_OF_ARGUMENTS_PER_OP_DAT = 4;
    
    static unsigned int const NUMBER_OF_ARGUMENTS_PER_OP_DAT_GENERIC = 6;

    /*
     * ======================================================
     * There are 2 non-OP_DAT arguments, namely
     * 'nameOfUserSubroutine' and 'OP_SET'
     * ======================================================
     */
    static unsigned int const NUMBER_OF_NON_OP_DAT_ARGUMENTS = 2;

    /*
     * ======================================================
     * Standard version:
     * In an OP_DAT argument group:
     * 1) The OP_DAT appears in position 0
     * 2) The index into the data appears in position 1
     * 3) The mapping appears in position 2
     * 4) The access descriptor appears in position 3
     * Generic version:
     * Standard version:
     * In an OP_DAT argument group:
     * 1) The OP_DAT appears in position 0
     * 2) The index into the data appears in position 1
     * 3) The mapping appears in position 2
     * 5) The dimension of the op_dat (literal) appears in position 3
     * 6) The type of the op_dat (string) appears in position 4
     * 7) The access descriptor appears in position 5
     * ======================================================
     */
    static unsigned int const POSITION_OF_OP_DAT = 0;
    static unsigned int const POSITION_OF_INDEX = 1;
    static unsigned int const POSITION_OF_MAPPING = 2;
    static unsigned int const POSITION_OF_ACCESS = 3;
    static unsigned int const POSITION_OF_DIMENSION = 3;
    static unsigned int const POSITION_OF_TYPE = 4;
    static unsigned int const POSITION_OF_ACCESS_GENERIC = 5;

    static unsigned int const POSITION_OF_FIRST_DAT_DIMENSION_GENERIC = 5;
    
  protected:
    
    bool isGenericParallelLoop;
    
  public:

    /*
     * ======================================================
     * Does this OP_DAT argument group need a declaration
     * for its cardinality (= setSize * dimension)?
     * ======================================================
     */
    bool
    isCardinalityDeclarationNeeded (unsigned int OP_DAT_ArgumentGroup);

    FortranParallelLoop (SgFunctionCallExp * functionCallExpression);
    
    bool
    isGenericLoop () { return isGenericParallelLoop; }
    
    void
    setStandardLoop () { isGenericParallelLoop = false; }

    void
    setGenericLoop () { isGenericParallelLoop = true; }
};

#endif
