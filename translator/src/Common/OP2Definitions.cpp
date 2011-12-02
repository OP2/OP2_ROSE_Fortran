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
