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
  else
  {
    Exceptions::ParallelLoop::UnsupportedBaseTypeException (
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

  /*
   * ======================================================
   * If the key is still zero then the base type of the
   * OP_DAT is not supported
   * ======================================================
   */
  ROSE_ASSERT (key > 0);

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
