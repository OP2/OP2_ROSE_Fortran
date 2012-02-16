


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
#ifndef GLOBALS_H
#define GLOBALS_H

#include <TargetLanguage.h>

class Globals
{
  private:

    static Globals * globalsInstance;

    TargetLanguage::FRONTEND frontend;

    TargetLanguage::BACKEND backend;

    bool oxfordOption;
    
    bool preprocessOption;
	
	bool syntacticFusionOption;

	std::string syntacticFusionKernels;

    bool uDrawOption;

    std::vector <std::string> inputFilenames;

    std::string freeVariablesModuleName;

  private:

    /*
     * ======================================================
     * Private constructor ensures users can never create
     * multiple instances
     * ======================================================
     */
    Globals ();

  public:

    /*
     * ======================================================
     * This always returns a single instance of the Globals
     * class to make it compliant with the singleton pattern
     * ======================================================
     */
    static Globals *
    getInstance ();

    /*
     * ======================================================
     * Set the host language
     * ======================================================
     */
    void
    setHostLanguage (TargetLanguage::FRONTEND frontend);

    /*
     * ======================================================
     * What is the host language?
     * ======================================================
     */
    TargetLanguage::FRONTEND
    getHostLanguage () const;

    /*
     * ======================================================
     * Set which type of code should be generated
     * ======================================================
     */
    void
    setTargetBackend (TargetLanguage::BACKEND backend);

    /*
     * ======================================================
     * What type of code should be generated?
     * ======================================================
     */
    TargetLanguage::BACKEND
    getTargetBackend () const;

    void
    setRenderOxfordAPICalls ();

    bool
    renderOxfordAPICalls () const;
    
    void
    setPreprocess ();
    
    bool
    preprocess () const;

    void
    setSyntacticFusion ();
    
    bool
    syntacticFusion () const;

	void
	setSyntacticFusionKernels (std::string kernels);
	
	std::string
	getSyntacticFusionKernels () const;
	
    void
    setOutputUDrawGraphs ();

    bool
    outputUDrawGraphs () const;

    void
    addInputFile (std::string const & fileName);

    bool
    isInputFile (std::string const & fileName) const;

    void
    setFreeVariablesModuleName (std::string const & fileName);

    std::string const
    getFreeVariablesModuleName () const;
};

#endif
