


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


#include <Reduction.h>
#include <Debug.h>
#include <Exceptions.h>
#include <rose.h>
#include <boost/lexical_cast.hpp>

SgType *
Reduction::getBaseType ()
{
  return baseType;
}

unsigned int
Reduction::getVariableSize () const
{
  return variableSize;
}

std::string
Reduction::getSubroutineName () const
{
  using boost::lexical_cast;
  using std::string;

  std::string name = "Reduction";

  if (isSgTypeInt (baseType) != NULL)
  {
    name += "Integer";
  }
  else if (isSgTypeFloat (baseType) != NULL)
  {
    name += "Float";
  }
  else if (isSgTypeDouble (baseType) != NULL)
  {
    name += "Double";
  }
  else
  {
    throw Exceptions::ParallelLoop::UnsupportedBaseTypeException (
        "Base type of reduction variable is not supported");
  }

  name += lexical_cast <string> (variableSize);

  return name;
}

unsigned int
Reduction::hashKey ()
{
  unsigned int key = 0;

  if (isSgTypeInt (baseType) != NULL)
  {
    key += 1;
  }
  else if (isSgTypeFloat (baseType) != NULL)
  {
    key += 2;
  }
  else if (isSgTypeDouble (baseType) != NULL)
  {
    key += 4;
  }
  else
  {
    throw Exceptions::ParallelLoop::UnsupportedBaseTypeException (
        "Base type of reduction variable is not supported");
  }

  key += variableSize;

  return key;
}

bool
Reduction::isEquivalent (Reduction * reduction)
{
  return reduction->hashKey () == hashKey ();
}

Reduction::Reduction (SgType * baseType, unsigned int variableSize) :
  baseType (baseType), variableSize (variableSize)
{
}
