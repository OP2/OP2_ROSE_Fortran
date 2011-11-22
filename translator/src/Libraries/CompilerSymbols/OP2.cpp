#include <OP2.h>
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
