/*
 * Written by Carlo Bertolli and Adam Betts
 *
 * This class generates code implementing reduction of OP_GBL variables marked
 * with OP_INC, OP_MAX or OP_MIN access
 */

#ifndef FORTRAN_CUDA_REDUCTION_SUBROUTINE_H
#define FORTRAN_CUDA_REDUCTION_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>

namespace ReductionSubroutine
{
  std::string const inputValue = "dat_l";
  std::string const reductionResultOnDevice = "dat_g";
  std::string const sharedMemoryStartOffset = "sharedMemoryStartOffset";
  std::string const threadID = "threadID";
  std::string const warpSize = "warpSize";
  std::string const maximumBytesInSharedMemory = "maxSharedMemoryBytes";
  std::string const maximumNumberOfThreadBlocks = "maxThreadBlocks";
  std::string const numberOfThreadItems = "threadItems";
  std::string const offsetForReduction = "offsetReduction";
  std::string const reductionArrayHost = "reductionArrayHost";
  std::string const reductionArrayDevice = "reductionArrayDevice";
}

class FortranCUDAReductionSubroutine: public Subroutine <
    SgProcedureHeaderStatement>
{
  private:

    /*
     * ======================================================
     * Type of the reduction variable (must be an array type)
     * ======================================================
     */
    SgArrayType * reductionVariableType;

  private:

    SgStatement *
    createSynchThreadsCallStatement ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    FortranCUDAReductionSubroutine (
        std::string const & subroutineAndVariableName,
        SgScopeStatement * moduleScope, SgArrayType * reductionVariableType);
};

#endif

