


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


#include <cstdlib>
#include <boost/filesystem.hpp>
#include <Globals.h>
#include <Debug.h>

Globals * Globals::globalsInstance = NULL;

Globals::Globals ()
{
  /*
   * ======================================================
   * Assume that the frontend and backend are unknown
   * ======================================================
   */

  frontend = TargetLanguage::UNKNOWN_FRONTEND;

  backend = TargetLanguage::UNKNOWN_BACKEND;

  /*
   * ======================================================
   * Assume other options have not been set
   * ======================================================
   */
  oxfordOption = false;

  preprocessOption = false;

  uDrawOption = false;
  
  includesMPI = false;
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

Globals *
Globals::getInstance ()
{
  if (globalsInstance == NULL)
  {
    globalsInstance = new Globals ();
  }
  return globalsInstance;
}

void
Globals::setHostLanguage (TargetLanguage::FRONTEND frontend)
{
  this->frontend = frontend;
}

TargetLanguage::FRONTEND
Globals::getHostLanguage () const
{
  return frontend;
}

void
Globals::setTargetBackend (TargetLanguage::BACKEND backend)
{
  this->backend = backend;
}

TargetLanguage::BACKEND
Globals::getTargetBackend () const
{
  return backend;
}

void
Globals::setRenderOxfordAPICalls ()
{
  oxfordOption = true;
}

bool
Globals::renderOxfordAPICalls () const
{
  return oxfordOption;
}

void
Globals::setPreprocess ()
{
    preprocessOption = true;
}

bool
Globals::preprocess () const
{
    return preprocessOption;
}

void
Globals::setSyntacticFusion ()
{
    syntacticFusionOption = true;
}

bool
Globals::syntacticFusion () const
{
    return syntacticFusionOption;
}

void
Globals::setSyntacticFusionKernels (std::string kernels)
{
	syntacticFusionKernels = kernels;
}

std::string
Globals::getSyntacticFusionKernels () const
{
	return syntacticFusionKernels;
}

void
Globals::setOutputUDrawGraphs ()
{
  uDrawOption = true;
}

bool
Globals::outputUDrawGraphs () const
{
  return uDrawOption;
}

void
Globals::addInputFile (std::string const & fileName)
{
  Debug::getInstance ()->debugMessage ("Adding file '" + fileName + "'",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  inputFilenames.push_back (fileName);
}

bool
Globals::isInputFile (std::string const & fileName) const
{
  return std::find (inputFilenames.begin (), inputFilenames.end (), fileName)
      != inputFilenames.end ();
}

void
Globals::setFreeVariablesModuleName (std::string const & moduleName)
{
  freeVariablesModuleName = moduleName;
}

std::string const
Globals::getFreeVariablesModuleName () const
{
  return freeVariablesModuleName;
}
