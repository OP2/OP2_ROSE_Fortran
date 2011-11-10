#include <OP2Definitions.h>
#include <rose.h>

SgFunctionCallExp *
OP2::Macros::createRoundUpCallStatement (SgScopeStatement * scope,
    SgExpression * parameterExpression)
{
  using namespace SageBuilder;
  using std::string;

  SgExprListExp * actualParameters = buildExprListExp (parameterExpression);

  string const functionName = "ROUND_UP";

  return buildFunctionCallExp (functionName, buildVoidType (),
      actualParameters, scope);
}

SgFunctionCallExp *
OP2::Macros::createMaxCallStatement (SgScopeStatement * scope,
    SgExpression * parameterExpression1, SgExpression * parameterExpression2)
{
  using namespace SageBuilder;
  using std::string;

  SgExprListExp * actualParameters = buildExprListExp (parameterExpression1,
      parameterExpression2);

  string const functionName = "MAX";

  return buildFunctionCallExp (functionName, buildVoidType (),
      actualParameters, scope);
}

SgFunctionCallExp *
OP2::Macros::createMinCallStatement (SgScopeStatement * scope,
    SgExpression * parameterExpression1, SgExpression * parameterExpression2)
{
  using namespace SageBuilder;
  using std::string;

  SgExprListExp * actualParameters = buildExprListExp (parameterExpression1,
      parameterExpression2);

  string const functionName = "MIN";

  return buildFunctionCallExp (functionName, buildVoidType (),
      actualParameters, scope);
}

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
OpDatDefinition::getPrimitiveType ()
{
  return primitiveType;
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
OpGblDefinition::getPrimitiveType ()
{
  return primitiveType;
}

unsigned int
OpConstDefinition::getDimension () const
{
  return dimension;
}
