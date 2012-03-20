


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


#ifndef FORTRAN_USER_SUBROUTINE_H
#define FORTRAN_USER_SUBROUTINE_H

#include <UserSubroutine.h>
#include <vector>


class FortranParallelLoop;
class FortranProgramDeclarationsAndDefinitions;
class FortranConstantDeclarations;

using namespace std;

class FortranUserSubroutine: public UserSubroutine <SgProcedureHeaderStatement,
    FortranProgramDeclarationsAndDefinitions>
{

  protected:
    
    /*
     * ======================================================
     * This boolean indicates if the user subroutine is one
     * used as an argument to an op_par_loop or if it is 
     * called inside a user kernel or another user subroutine
     * It permits handling the "value" attribute in a 
     * different way w.r.t. the parallelLoop object in which
     * it is called and the positional identities of its
     * formal parameters
     * ======================================================
     */
    bool isUserKernel;
    
   /*
    * ======================================================
    * List of other subroutines called by this subroutine
    * Used to avoid copying the same routine twice
    * ======================================================
    */
    vector < FortranUserSubroutine * > additionalSubroutines;
    
  public:

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

    void appendAdditionalSubroutines ( SgScopeStatement * moduleScope,
      FortranParallelLoop * parallelLoop, FortranProgramDeclarationsAndDefinitions * declarations,
      FortranConstantDeclarations * OP2constants, std::vector < SgProcedureHeaderStatement * > * allCalledRoutines);
        
//    vector < FortranUserSubroutine * > getAdditionalSubroutines()
//    {
/*      return additionalSubroutines;
    }*/

    FortranUserSubroutine (SgScopeStatement * moduleScope, FortranParallelLoop * parallelLoop,
      FortranProgramDeclarationsAndDefinitions * declarations);
/*
 * ======================================================
 * This contructor is for non user kernels only (see 
 * above)
 * ======================================================
 */

  FortranUserSubroutine (SgScopeStatement * moduleScope, FortranParallelLoop * parallelLoop,
    FortranProgramDeclarationsAndDefinitions * declarations, string subroutineName);

/*    FortranUserSubroutine (SgScopeStatement * moduleScope,
        FortranParallelLoop * parallelLoop,
        FortranProgramDeclarationsAndDefinitions * declarations);

    FortranUserSubroutine (SgScopeStatement * moduleScope,
        FortranParallelLoop * parallelLoop,
        FortranProgramDeclarationsAndDefinitions * declarations,
        string subroutineName);*/
};

#endif
