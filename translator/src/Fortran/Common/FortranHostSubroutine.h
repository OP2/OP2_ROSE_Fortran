
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
#ifndef FORTRAN_HOST_SUBROUTINE_H
#define FORTRAN_HOST_SUBROUTINE_H

#include <HostSubroutine.h>

class SgProcedureHeaderStatement;
class SgScopeStatement;
class SgBasicBlock;
class FortranKernelSubroutine;
class FortranParallelLoop;

class FortranHostSubroutine: public HostSubroutine <SgProcedureHeaderStatement>
{
  protected:

    virtual SgBasicBlock *
    createTransferOpDatStatements () = 0;

    virtual void
    createFormalParameterDeclarations ();

    /*
     * ======================================================
     * Generates if (set%size == 0) return statement
     * Having empty iteration sets is a typical behaviour of 
     * configurable applications. It also comes as a version 
     * for the new OP2-Common library
     * ======================================================
     */
    void
    createEarlyExitStatement (SgScopeStatement * subroutineScope);

    void
    createEarlyExitStatementNewLibrary (SgScopeStatement * subroutineScope);
    
    void
    createDumpOfOutputDeclarations (SgScopeStatement * subroutineScope);

    /*
     * ======================================================
     * Calls the op_mpi_halo_exchanges
     * ======================================================
     */    
    void
    appendCallMPIHaloExchangeFunction (SgScopeStatement * scope);

    /*
     * ======================================================
     * Calls the op_mpi_wait_all (called in if-then)
     * ======================================================
     */    
    void
    appendCallMPIWaitAll (SgScopeStatement * scope);

    /*
     * ======================================================
     * Calls the op_mpi_set_dirtybit. Needs a scope
     * because it is called in different contexts
     * ======================================================
     */
    void
    appendCallMPISetDirtyBit (SgScopeStatement * scope);

    /*
     * ======================================================
     * Populates the opArgArray variable with formal params.
     * ======================================================
     */
    void
    appendPopulationOpArgArray (SgScopeStatement * scope);

    /*
     * ======================================================
     * Initialise the variable counting the number of input
     * op_args
     * ======================================================
     */
    void
    initialiseNumberOfOpArgs (SgScopeStatement * scope);

   /*
    * ======================================================
    * Declare array of op_args and variable with number
    * of parameters
    * ======================================================
    */
    void
    createCommonLocalVariableDeclarations (SgScopeStatement * scope);

    /*
     * ======================================================
     * Appends one call to op_mpi_reduce for each reduction
     * argument
     * ======================================================
     */
    void
    appendCallsToMPIReduce (SgScopeStatement * scope);
    
    /*
     * ======================================================
     * Dumps all output op_dats of a parallel loop
     * ======================================================
     */
    void
    createDumpOfOutputStatements (SgScopeStatement * subroutineScope, std::string const dumpOpDatFunctionName);

    FortranHostSubroutine (SgScopeStatement * moduleScope, Subroutine <
        SgProcedureHeaderStatement> * calleeSubroutine,
        FortranParallelLoop * parallelLoop);
};

#endif
