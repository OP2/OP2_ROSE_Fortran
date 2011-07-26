/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models an OP_PAR_LOOP call discovered in the
 * user-supplied code
 */

#ifndef PARALLEL_LOOP_H
#define PARALLEL_LOOP_H

#include <CommonNamespaces.h>
#include <ProgramDeclarationsAndDefinitions.h>
#include <OP2Definitions.h>

enum MAPPING_VALUE
{
  DIRECT, INDIRECT, GLOBAL
};

enum ACCESS_CODE_VALUE
{
  READ_ACCESS, WRITE_ACCESS, RW_ACCESS, INC_ACCESS, MAX_ACCESS, MIN_ACCESS
};

template <typename TSubroutineHeader>
  class ParallelLoop
  {
    protected:

      /*
       * ======================================================
       * The function call expression where the call to the
       * OP_PAR_LOOP occurs
       * ======================================================
       */
      SgFunctionCallExp * functionCallExpression;

      /*
       * ======================================================
       * The same OP_DAT variable can be passed as an argument
       * to OP_PAR_LOOP. This vector records the names of the
       * unique OP_DATs
       * ======================================================
       */
      std::vector <std::string> uniqueOpDats;

      /*
       * ======================================================
       * The base type of each OP_DAT. The map is indexed by
       * the argument group number
       * ======================================================
       */
      std::map <unsigned int, SgType *> OpDatTypes;

      /*
       * ======================================================
       * The dimension of each OP_DAT. The map is indexed by
       * the argument group number
       * ======================================================
       */
      std::map <unsigned int, unsigned int> OpDatDimensions;

      /*
       * ======================================================
       * The same OP_DAT variable can be passed as an argument
       * to OP_PAR_LOOP. This map records whether the OP_DAT argument
       * at this position (in the actual arguments) is a duplicate
       * of a previous OP_DAT argument
       * ======================================================
       */
      std::map <unsigned int, bool> OpDatDuplicates;

      /*
       * ======================================================
       * How is the data for the OP_DAT variables in this position
       * (in the actual arguments) accessed: through
       * a mapping or directly or as a global variable?
       * ======================================================
       */
      std::map <unsigned int, MAPPING_VALUE> OpDatMappingDescriptors;

      /*
       * ======================================================
       * How is the data for the OP_DAT variables in this position
       * (in the actual arguments) accessed: read, written, etc.
       * ======================================================
       */
      std::map <unsigned int, ACCESS_CODE_VALUE> OpDatAccessDescriptors;

      /*
       * ======================================================
       * What is OP_DAT variable name in this position
       * (in the actual arguments)?
       * ======================================================
       */
      std::map <unsigned int, std::string> OpDatVariableNames;

      /*
       * ======================================================
       * What is the size of the OP_DAT data types? Either it
       * is real(4) or real(8), so respectively 4 or 8
       * ======================================================
       */
      unsigned int sizeOfOpDat;

      /*
       * ======================================================
       * Mapping from an OP_DAT which requires a reduction to
       * its respective reduction subroutine
       * ======================================================
       */
      std::map <unsigned int, TSubroutineHeader *> reductionSubroutines;

    protected:

      ParallelLoop (SgFunctionCallExp * functionCallExpression) :
        functionCallExpression (functionCallExpression)
      {
      }

    public:

      /*
       * ======================================================
       * Generates all reduction subroutines: one per OP_DAT
       * type and kind
       * ======================================================
       */
      virtual void
      generateReductionSubroutines (SgScopeStatement * moduleScope) = 0;

      /*
       * ======================================================
       * How many OP_DAT argument groups are there?
       * ======================================================
       */
      virtual unsigned int
      getNumberOfOpDatArgumentGroups () = 0;

      /*
       * ======================================================
       * Is this parallel loop direct or indirect?
       * ======================================================
       */
      bool
      isDirectLoop ()
      {
        for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
        {
          if (OpDatMappingDescriptors[i] == INDIRECT)
          {
            return false;
          }
        }
        return true;
      }

      /*
       * ======================================================
       * How many OP_DAT arguments have indirect access
       * descriptors?
       * ======================================================
       */
      unsigned int
      getNumberOfDistinctIndirectOpDatArguments ()
      {
        unsigned int count = 0;
        for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
        {
          if (OpDatDuplicates[i] == false)
          {
            if (OpDatMappingDescriptors[i] == INDIRECT)
            {
              count++;
            }
          }
        }
        return count;
      }

      void
      setOpDatType (unsigned int OP_DAT_ArgumentGroup, SgType * type)
      {
        OpDatTypes[OP_DAT_ArgumentGroup] = type;
      }

      /*
       * ======================================================
       * What is the type which is wrapped by the OP_DAT declaration
       * in the particular OP_DAT argument group?
       * ======================================================
       */
      SgType *
      getOpDatType (unsigned int OP_DAT_ArgumentGroup)
      {
        return OpDatTypes[OP_DAT_ArgumentGroup];
      }

      void
      setOpDatDimension (unsigned int OP_DAT_ArgumentGroup,
          unsigned int dimension)
      {
        OpDatDimensions[OP_DAT_ArgumentGroup] = dimension;
      }

      /*
       * ======================================================
       * What is the dimension of the OP_DAT variable in
       * this OP_DAT argument group?
       * ======================================================
       */
      unsigned int
      getOpDatDimension (unsigned int OP_DAT_ArgumentGroup)
      {
        return OpDatDimensions[OP_DAT_ArgumentGroup];
      }

      void
      setDuplicateOpDat (unsigned int OP_DAT_ArgumentGroup, bool value)
      {
        OpDatDuplicates[OP_DAT_ArgumentGroup] = value;
      }

      /*
       * ======================================================
       * Is the OP_DAT variable in this OP_DAT argument group
       * the same as another OP_DAT in the actual arguments
       * passed to the OP_PAR_LOOP?
       * ======================================================
       */
      bool
      isDuplicateOpDat (unsigned int OP_DAT_ArgumentGroup)
      {
        return OpDatDuplicates[OP_DAT_ArgumentGroup];
      }

      void
      setOpMapValue (unsigned int OP_DAT_ArgumentGroup, MAPPING_VALUE value)
      {
        OpDatMappingDescriptors[OP_DAT_ArgumentGroup] = value;
      }

      /*
       * ======================================================
       * What is the mapping value of the OP_DAT argument
       * in this OP_DAT argument group?
       * ======================================================
       */
      MAPPING_VALUE
      getOpMapValue (unsigned int OP_DAT_ArgumentGroup)
      {
        return OpDatMappingDescriptors[OP_DAT_ArgumentGroup];
      }

      void
      setOpAccessValue (unsigned int OP_DAT_ArgumentGroup,
          ACCESS_CODE_VALUE value)
      {
        OpDatAccessDescriptors[OP_DAT_ArgumentGroup] = value;
      }

      /*
       * ======================================================
       * What is the access descriptor of the OP_DAT argument
       * in this OP_DAT argument group?
       * ======================================================
       */
      ACCESS_CODE_VALUE
      getOpAccessValue (unsigned int OP_DAT_ArgumentGroup)
      {
        return OpDatAccessDescriptors[OP_DAT_ArgumentGroup];
      }

      /*
       * ======================================================
       * How many OP_DATs are indirectly accessed?
       * ======================================================
       */
      unsigned int
      getNumberOfIndirectOpDats ()
      {
        int count = 0;
        for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
        {
          if (OpDatMappingDescriptors[i] == INDIRECT)
          {
            count++;
          }
        }
        return count;
      }

      /*
       * ======================================================
       * How many OP_DATs are indirectly accessed and are
       * unique in the actual arguments passed to the OP_PAR_LOOP?
       * ======================================================
       */
      unsigned int
      getNumberOfDifferentIndirectOpDats ()
      {
        int count = 0;
        for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
        {
          if (OpDatMappingDescriptors[i] == INDIRECT && isDuplicateOpDat (i)
              == false)
          {
            count++;
          }
        }
        return count;
      }

      /*
       * ======================================================
       * Does this parallel loop require a reduction?
       * ======================================================
       */
      bool
      isReductionRequired ()
      {
        for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
        {
          if (OpDatMappingDescriptors[i] == GLOBAL && OpDatAccessDescriptors[i]
              != READ_ACCESS)
          {
            return true;
          }
        }
        return false;
      }

      /*
       * ======================================================
       * Does the OP_DAT in this argument group require a reduction?
       * ======================================================
       */
      bool
      isReductionRequired (int OP_DAT_ArgumentGroup)
      {
        return OpDatMappingDescriptors[OP_DAT_ArgumentGroup] == GLOBAL
            && OpDatAccessDescriptors[OP_DAT_ArgumentGroup] != READ_ACCESS;
      }

      /*
       * ======================================================
       * Get the reduction type for this OP_PAR_LOOP.
       * WARNING: This assumes that there is ONLY ONE OP_DAT
       * which needs a reduction
       * ======================================================
       */
      SgType *
      getReductionType ()
      {
        SgType * type;
        int count = 0;

        for (unsigned int i = 1; i <= getNumberOfOpDatArgumentGroups (); ++i)
        {
          if (OpDatMappingDescriptors[i] == GLOBAL && OpDatAccessDescriptors[i]
              != READ_ACCESS)
          {
            count++;
            type = OpDatTypes[i];
          }
        }

        if (count > 1)
        {
          Debug::getInstance ()->errorMessage (
              "Compiler cannot currently handle parallel loop with multiple reduction arguments");
        }
        else if (count == 0)
        {
          Debug::getInstance ()->errorMessage ("No reduction argument found");
        }

        return type;
      }

      void
      setOpDatVariableName (unsigned int OP_DAT_ArgumentGroup,
          std::string const variableName)
      {
        OpDatVariableNames[OP_DAT_ArgumentGroup] = variableName;
      }

      /*
       * ======================================================
       * What is the name of the OP_DAT variable in this OP_DAT
       * argument group
       * ======================================================
       */
      std::string
      getOpDatVariableName (unsigned int OP_DAT_ArgumentGroup)
      {
        return OpDatVariableNames[OP_DAT_ArgumentGroup];
      }

      void
      setSizeOfOpDat (unsigned int size)
      {
        sizeOfOpDat = size;
      }

      /*
       * ======================================================
       * What is the maximum size of an individual element of an
       * OP_DAT in terms of bytes?
       * ======================================================
       */
      unsigned int
      getSizeOfOpDat () const
      {
        return sizeOfOpDat;
      }

      bool
      isUniqueOpDat (std::string const & variableName)
      {
        return find (uniqueOpDats.begin (), uniqueOpDats.end (), variableName)
            == uniqueOpDats.end ();
      }

      void
      setUniqueOpDat (std::string const & variableName)
      {
        uniqueOpDats.push_back (variableName);
      }

      /*
       * ======================================================
       * Returns the procedure header associated with the reduction
       * subroutine needed for this OP_DAT argument group
       * ======================================================
       */
      SgProcedureHeaderStatement *
      getReductionSubroutineHeader (unsigned int OP_DAT_ArgumentGroup)
      {
        return reductionSubroutines[OP_DAT_ArgumentGroup];
      }

      /*
       * ======================================================
       * Returns the actual arguments passed to the OP_PAR_LOOP
       * ======================================================
       */
      SgExpressionPtrList &
      getActualArguments ()
      {
        return functionCallExpression->get_args ()->get_expressions ();
      }

      /*
       * ======================================================
       * Returns the function call node in the AST corresponding
       * to this OP_PAR_LOOP
       * ======================================================
       */
      SgFunctionCallExp *
      getFunctionCall ()
      {
        return functionCallExpression;
      }

      /*
       * ======================================================
       * Returns the name of the file in which the OP_PAR_LOOP
       * call is contained
       * ======================================================
       */
      std::string const &
      getFileName () const
      {
        return functionCallExpression->getFilenameString ();
      }
  };

#endif
