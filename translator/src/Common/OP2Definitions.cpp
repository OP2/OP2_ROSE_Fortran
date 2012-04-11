


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


#include <OP2Definitions.h>
#include <rose.h>

OP2Definition::OP2Definition ()
{
}

OP2Definition::~OP2Definition ()
{
}

std::string const &
OP2Definition::getVariableName () const
{
  return variableName;
}

std::string const &
OpDatDefinition::getOpSetName () const
{
  return opSetName;
}

unsigned int
OpDatDefinition::getDimension () const
{
  return dimension;
}

SgType *
OpDatDefinition::getBaseType ()
{
  return baseType;
}

std::string const &
OpSetDefinition::getDimensionName () const
{
  return dimensionName;
}

std::string const &
OpSubSetDefinition::getOriginSetName () const
{
  return originSetName;
}

std::string const &
OpSubSetDefinition::getFilterKernelName () const
{
  return filterKernelName;
}

int
OpSubSetDefinition::getNbFilterArg ()
{
    return nbFilterArg;
}

std::string const &
OpMapDefinition::getSourceOpSetName () const
{
  return sourceOpSetName;
}

std::string const &
OpMapDefinition::getDestinationOpSetName () const
{
  return destinationOpSetName;
}

unsigned int
OpMapDefinition::getDimension () const
{
  return dimension;
}

std::string const &
OpMapDefinition::getMappingName () const
{
  return mappingName;
}

unsigned int
OpGblDefinition::getDimension () const
{
  return dimension;
}

SgType *
OpGblDefinition::getBaseType ()
{
  return baseType;
}

SgType *
OpConstDefinition::getType ()
{
  return baseType;
}

unsigned int
OpConstDefinition::getDimension () const
{
  return dimension;
}

std::string const &
OpArgMatDefinition::getMatName () const
{
  return matName;
}

const int *
OpArgMatDefinition::getMap1Extent () const
{
  return map1extent;
}

std::string const &
OpArgMatDefinition::getMap1Name () const
{
  return map1Name;
}

const int *
OpArgMatDefinition::getMap2Extent () const
{
  return map2extent;
}

std::string const &
OpArgMatDefinition::getMap2Name () const
{
  return map2Name;
}

unsigned int
OpArgMatDefinition::getDimension () const
{
  return dimension;
}

std::string const &
OpMatDefinition::getSparsityName () const
{
  return sparsityName;
}

unsigned int
OpMatDefinition::getDimension () const
{
  return dimension;
}

SgType *
OpMatDefinition::getBaseType ()
{
  return baseType;
}

std::string const &
OpSparsityDefinition::getMap1Name () const
{
  return map1Name;
}

std::string const &
OpSparsityDefinition::getMap2Name () const
{
  return map2Name;
}

OpIterationSpaceDefinition::OpIterationSpaceDefinition (SgExprListExp * args)
{
  SgVarRefExp * set = isSgVarRefExp (args->get_expressions ()[0]);

  ROSE_ASSERT (set != NULL);

  setName = set->get_symbol ()->get_name ().getString ();

  for (unsigned int i = 1; i < args->get_expressions ().size (); i++)
  {
    SgIntVal * dim = isSgIntVal (args->get_expressions ()[i]);

    ROSE_ASSERT (dim != NULL);

    int tmp[2];
    tmp[0] = 0;
    tmp[1] = dim->get_value ();
    addIterationDimension (tmp);
  }
}

OpIterationSpaceDefinition::OpIterationSpaceDefinition ()
{
  setName = "";
}

void
OpIterationSpaceDefinition::addIterationDimension (int dim[2])
{
  int * thing = new int[2];
  thing[0] = dim[0];
  thing[1] = dim[1];
  iterationDimensions.push_back (thing);
}

std::vector<int*>
OpIterationSpaceDefinition::getIterationDimensions () const
{
  return iterationDimensions;
}

std::string const &
OpIterationSpaceDefinition::getSetName () const
{
  return setName;
}

void
OpIterationSpaceDefinition::setSetName (std::string const & name)
{
  setName = name;
}
