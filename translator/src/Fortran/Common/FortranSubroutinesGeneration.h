


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
#ifndef FORTRAN_SUBROUTINES_GENERATION_H
#define FORTRAN_SUBROUTINES_GENERATION_H

#include <SubroutinesGeneration.h>

class SgProject;
class SgSourceFile;
class SgModuleStatement;
class FortranReductionSubroutines;
class FortranOpDatDimensionsDeclaration;
class FortranModuleDeclarations;
class FortranHostSubroutine;
class FortranParallelLoop;
class FortranProgramDeclarationsAndDefinitions;

class FortranSubroutinesGeneration: public SubroutinesGeneration <
    FortranProgramDeclarationsAndDefinitions, FortranHostSubroutine>
{
  protected:

    FortranReductionSubroutines * reductionSubroutines;

    std::map <std::string, FortranOpDatDimensionsDeclaration *>
        dimensionsDeclarations;

    std::map <std::string, FortranModuleDeclarations *> moduleDeclarations;

    std::vector <std::string> headersWithAddedUseStatements;

  private:

    virtual void
    processOP2ConstantDeclarations ();

    void
    addModuleUseStatement (SgNode * parent, std::string const & moduleName);

    void
    patchCallsToParallelLoops (std::string const & moduleName);

    /*
     * ======================================================
     * Adds the Fortran 'contains' statement to the module
     * which must precede all subroutine declarations
     * ======================================================
     */
    void
    addContains ();

    /*
     * ======================================================
     * Creates the Fortran module
     * ======================================================
     */
    SgModuleStatement *
    createFortranModule (std::string const & moduleName);

  protected:

    /*
     * ======================================================
     * Creates the new subroutines. This function is pure virtual
     * so the target backend has to implement it
     * ======================================================
     */
    virtual void
    createSubroutines () = 0;

    /*
     * ======================================================
     * Creates subroutines needed for reductions. This function
     * is pure virtual so the target backend has to implement it
     * ======================================================
     */
    virtual void
    createReductionSubroutines () = 0;

    /*
     * ======================================================
     * Creates the module type and variable declarations. This
     * function is pure virtual so the target backend has to
     * implement it
     * ======================================================
     */
    virtual void
    createModuleDeclarations () = 0;

    /*
     * ======================================================
     * Adds the relevant library 'use' statements to the
     * generated module. This function is pure virtual as the
     * needed libraries are backend specific
     * ======================================================
     */
    virtual void
    addLibraries () = 0;

    /*
     * ======================================================
     * Do the generation
     * ======================================================
     */
    void
    generate ();

  public:

    FortranSubroutinesGeneration (SgProject * project,
        FortranProgramDeclarationsAndDefinitions * declarations,
        std::string const & newFileName);
};

#endif
