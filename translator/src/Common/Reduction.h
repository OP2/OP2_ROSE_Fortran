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
  MINIMUM, MAXIMUM, INCREMENT
};

class Reduction
{
  private:

    /*
     * ======================================================
     * The array type of the OP_DAT variable
     * ======================================================
     */

    SgArrayType * arrayType;

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

    /*
     * ======================================================
     * The reduction operation
     * ======================================================
     */

    OPERATION operation;

  public:

    SgArrayType *
    getArrayType ()
    {
      return arrayType;
    }

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

    OPERATION
    getOperation () const
    {
      return operation;
    }

    std::string
    getSubroutineName () const
    {
      using boost::lexical_cast;
      using std::string;

      std::string name = "Reduction_";

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
            "Error: base type of reduction variable is not supported");
      }

      name += lexical_cast <string> (variableSize);

      if (operation == MINIMUM)
      {
        name += "_Minimum";
      }
      else if (operation == MAXIMUM)
      {
        name += "_Maximum";
      }
      else if (operation == INCREMENT)
      {
        name += "_Increment";
      }

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
      key += operation;

      return key;
    }

    bool
    isEquivalent (Reduction * reduction)
    {
      return reduction->hashKey () == hashKey ();
    }

    Reduction (SgArrayType * arrayType, SgType * baseType,
        unsigned int variableSize, OPERATION operation) :
      arrayType (arrayType), baseType (baseType), variableSize (variableSize),
          operation (operation)
    {
    }
};

#endif