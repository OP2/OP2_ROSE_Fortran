/*
 * Written by Carlo Bertolli and Adam Betts
 *
 * This class generates code implementing reduction of OP_GBL variables marked
 * with OP_INC, OP_MAX or OP_MIN access
 */

#ifndef CPP_OpenCL_REDUCTION_SUBROUTINE_H
#define CPP_OpenCL_REDUCTION_SUBROUTINE_H

#include <Subroutine.h>
#include <Reduction.h>

namespace ReductionSubroutine
{
  std::string const reductionType = "reduction";
  std::string const autosharedV = "vtemp";
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
  std::string const reductionArraySize = "reductionArraySize";
  std::string const reductionArrayBytes = "reductionArrayBytes";
  std::string const reductionH = "OP_reduct_h";
}

class CPPOpenCLReductionSubroutine: public Subroutine <
    SgProcedureHeaderStatement>
{
  private:

    /*
     * ======================================================
     * Information pertaining to the type of reduction
     * ======================================================
     */
    Reduction * reduction;

    std::string autosharedVariableName;

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

    CPPOpenCLReductionSubroutine (
        std::string const & subroutineAndVariableName,
        SgScopeStatement * moduleScope, Reduction * reduction);
};

#endif

