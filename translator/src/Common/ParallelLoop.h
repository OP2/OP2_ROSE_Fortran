/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models an OP_PAR_LOOP call discovered in the
 * user-supplied code
 */

#ifndef PARALLEL_LOOP_H
#define PARALLEL_LOOP_H

#include <CommonNamespaces.h>
#include <Declarations.h>
#include <OP2Variables.h>

enum MAPPING_VALUE
{
  DIRECT, INDIRECT, GLOBAL
};

enum ACCESS_CODE_VALUE
{
  READ_ACCESS, WRITE_ACCESS, RW_ACCESS, INC_ACCESS, MAX_ACCESS, MIN_ACCESS
};

template <typename TSubroutineHeader, typename TDeclarations>
  class ParallelLoop
  {
    protected:

      /*
       * ======================================================
       * The name of the generated Fortran/C++ module for this
       * OP_PAR_LOOP
       * ======================================================
       */
      std::string moduleName;

      /*
       * ======================================================
       * The actual arguments passed to the OP_PAR_LOOP
       * ======================================================
       */
      SgExpressionPtrList actualArguments;

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

      /*
       * ======================================================
       * Handles the actions associated with the discovery of
       * an OP_GBL
       * ======================================================
       */
      virtual void
      handleOpGblDeclaration (OpGblDeclaration * opGblDeclaration,
          std::string const & variableName, int opDatArgumentGroup) = 0;

      /*
       * ======================================================
       * Handles the actions associated with the discovery of
       * an OP_DAT
       * ======================================================
       */
      virtual void
      handleOpDatDeclaration (OpDatDeclaration * opDatDeclaration,
          std::string const & variableName, int opDatArgumentGroup) = 0;

      /*
       * ======================================================
       * Retrieves the declarations of the OP_DAT arguments
       * so that we can later retrieve their primitive types,
       * dimensions, access values, etc.
       * ======================================================
       */
      virtual void
      retrieveOpDatDeclarations (TDeclarations * declarations) = 0;

      ParallelLoop (SgExpressionPtrList & actualArguments) :
        actualArguments (actualArguments)
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
      getNumberOfOpDatArgumentGroups () const = 0;

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
       * What is the name of the OP_DAT variable in this OP_DAT
       * argument group
       * ======================================================
       */
      std::string
      getOpDatVariableName (unsigned int OP_DAT_ArgumentGroup)
      {
        return OpDatVariableNames[OP_DAT_ArgumentGroup];
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
       * What is the name of the module generated for this
       * OP_PAR_LOOP containing the generated subroutines?
       * ======================================================
       */
      std::string const &
      getModuleName () const
      {
        return moduleName;
      }

      /*
       * ======================================================
       * Returns the actual arguments passed to the OP_PAR_LOOP
       * ======================================================
       */
      SgExpressionPtrList &
      getActualArguments ()
      {
        return actualArguments;
      }
  };

#endif
