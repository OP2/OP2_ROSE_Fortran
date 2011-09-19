/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models a reduction
 */

#ifndef REDUCTION_H
#define REDUCTION_H

#include <rose.h>
#include <boost/lexical_cast.hpp>
#include <Debug.h>

enum OPERATION
{
  INCREMENT, MINIMUM, MAXIMUM
};

class Reduction
{
  private:

    /*
     * ======================================================
     * The base type of the variable on which the reduction
     * operates
     * ======================================================
     */

    SgType * baseType;

    /*
     * ======================================================
     * The size of the variable on which the reduction operates
     * ======================================================
     */

    unsigned int variableSize;

  public:

    SgType *
    getBaseType ()
    {
      return baseType;
    }

    unsigned int
    getVariableSize () const
    {
      return variableSize;
    }

    std::string
    getSubroutineName () const
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
        Debug::getInstance ()->errorMessage (
            "Error: base type of reduction variable is not supported",
            __FILE__, __LINE__);
      }

      name += lexical_cast <string> (variableSize);

      return name;
    }

    unsigned int
    hashKey ()
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
    isEquivalent (Reduction * reduction)
    {
      return reduction->hashKey () == hashKey ();
    }

    Reduction (SgType * baseType, unsigned int variableSize) :
      baseType (baseType), variableSize (variableSize)
    {
    }
};

#endif
