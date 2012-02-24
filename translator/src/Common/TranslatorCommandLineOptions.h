


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


#ifndef TRANSLATOR_COMMAND_LINE_OPTIONS_H
#define TRANSLATOR_COMMAND_LINE_OPTIONS_H

#include "Globals.h"
#include "TargetLanguage.h"
#include "CommandLineOption.h"
#include "CommandLineOptionWithParameters.h"

class FreeVariablesModuleOption: public CommandLineOptionWithParameters
{
  public:

    virtual void
    run ()
    {
      Globals::getInstance ()->setFreeVariablesModuleName (getParameter ());
    }

    FreeVariablesModuleOption (std::string helpMessage) :
      CommandLineOptionWithParameters (helpMessage, "string", "m",
          "constants-module")
    {
    }
};

class OxfordOption: public CommandLineOption
{
  public:

    virtual void
    run ()
    {
      Globals::getInstance ()->setRenderOxfordAPICalls ();
    }

    OxfordOption (std::string helpMessage, std::string longOption) :
      CommandLineOption (helpMessage, "", longOption)
    {
    }
};

class PreprocessOption: public CommandLineOption
{
public:
	virtual void
	run ()
	{
		Globals::getInstance ()->setPreprocess ();
	}
	
	PreprocessOption (std::string helpMessage, std::string longOption) :
      CommandLineOption (helpMessage, "", longOption)
	{
	}
};

class SyntacticFusionOption: public CommandLineOptionWithParameters
{
public:
	virtual void
	run ()
	{
		Globals::getInstance ()->setSyntacticFusion ();
		Globals::getInstance ()->setSyntacticFusionKernels (getParameter ());
	}
	
	SyntacticFusionOption (std::string helpMessage, std::string longOption) :
	CommandLineOptionWithParameters (helpMessage, "kernels", "", longOption)
	{
	}
};

class CUDAOption: public CommandLineOption
{
  public:

    virtual void
    run ()
    {
      Globals::getInstance ()->setTargetBackend (TargetLanguage::CUDA);
    }

    CUDAOption (std::string helpMessage, std::string longOption) :
      CommandLineOption (helpMessage, "", longOption)
    {
    }
};

class OpenMPOption: public CommandLineOption
{
  public:

    virtual void
    run ()
    {
      Globals::getInstance ()->setTargetBackend (TargetLanguage::OPENMP);
    }

    OpenMPOption (std::string helpMessage, std::string longOption) :
      CommandLineOption (helpMessage, "", longOption)
    {
    }
};

class OpenCLOption: public CommandLineOption
{
  public:

    virtual void
    run ()
    {
      Globals::getInstance ()->setTargetBackend (TargetLanguage::OPENCL);
    }

    OpenCLOption (std::string helpMessage, std::string longOption) :
      CommandLineOption (helpMessage, "", longOption)
    {
    }
};

#endif
