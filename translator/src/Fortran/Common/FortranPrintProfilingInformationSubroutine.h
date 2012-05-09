
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
#ifndef FORTRAN_PRINT_PROFILING_INFORMATION_SUBROUTINE_H
#define FORTRAN_PRINT_PROFILING_INFORMATION_SUBROUTINE_H

#include <string>
#include <map>

#include <Subroutine.h>
#include <ParallelLoop.h>

class FortranProgramDeclarationsAndDefinitions;
class FortranModuleDeclarations;

class FortranPrintProfilingInformationSubroutine: public Subroutine <
  SgProcedureHeaderStatement>
{
  private:

    std::map <std::string, FortranModuleDeclarations *> moduleDeclarations;

    FortranProgramDeclarationsAndDefinitions * parallelLoopsDeclarations;

    SgStatement * implicitStatement;

  protected:

    /*
     * ======================================================
     * Every created subroutine has a number of statements
     * ======================================================
     */
    
    virtual void
    createStatements ();

    /*
     * ======================================================
     * Every created subroutine has local variable declarations
     * ======================================================
     */
    
    virtual void
    createLocalVariableDeclarations ();

    /*
     * ======================================================
     * Every created subroutine has formal parameters
     * ======================================================
     */

    virtual void
    createFormalParameterDeclarations ();

  public:

    FortranPrintProfilingInformationSubroutine (std::string subroutineName,
      SgScopeStatement * moduleScope,
      std::map <std::string, FortranModuleDeclarations *> moduleDeclarations,
      FortranProgramDeclarationsAndDefinitions * parallelLoopDeclarations);
};
  
#endif
