/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models the kernel subroutine for an indirect loop.
 * Its declarations and statements do the following:
 *
 */

#ifndef KERNEL_SUBROUTINE_OF_INDIRECT_LOOP_H
#define KERNEL_SUBROUTINE_OF_INDIRECT_LOOP_H

#include <KernelSubroutine.h>
#include <DataSizesDeclarationOfIndirectLoop.h>

class KernelSubroutineOfIndirectLoop: public KernelSubroutine
{
  private:

    std::map <std::string, unsigned int> positionOf_nbytes;

    /*
     * ======================================================
     * Remaining local variables
     * ======================================================
     */

    std::map <unsigned int, SgVariableDeclaration *> localVariables_indArgSizes;

    std::map <unsigned int, SgVariableDeclaration *> localVariables_nbytes;

    std::map <unsigned int, SgVariableDeclaration *> localVariables_inRoundUps;

    std::map <unsigned int, SgVariableDeclaration *>
        localVariables_incrementAccessMaps;

    std::map <unsigned int, SgVariableDeclaration *>
        localVariables_incrementAccessArrays;

  private:

    static std::string
    getLocalToGlobalMappingVariableName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectLoop::Fortran::VariablePrefixes::pindMaps + lexical_cast <
          string> (OP_DAT_ArgumentGroup);
    }

    static std::string
    getGlobalToLocalMappingVariableName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectLoop::Fortran::VariablePrefixes::pMaps + lexical_cast <
          string> (OP_DAT_ArgumentGroup);
    }

    SgStatement *
    createUserSubroutineCall (UserDeviceSubroutine & userDeviceSubroutine,
        ParallelLoop & parallelLoop);

    void
    applyPointeredIncrementsOrWrites (ParallelLoop & parallelLoop);

    void
    createPlanWhileLoop (UserDeviceSubroutine & userDeviceSubroutine,
        ParallelLoop & parallelLoop);

    void
    createAutosharedWhileLoops (ParallelLoop & parallelLoop);

    void
    initialiseLocalVariables (ParallelLoop & parallelLoop);

    void
    createThreadZeroStatements (ParallelLoop & parallelLoop);

    void
    createRemainingLocalVariables ();

    void
    createLocalVariables (ParallelLoop & parallelLoop);

    void
    createSharedLocalVariables (ParallelLoop & parallelLoop);

    void
        createPlanFormalParameters (
            DataSizesDeclarationOfIndirectLoop & dataSizesDeclarationOfIndirectLoop,
            ParallelLoop & parallelLoop);

    void
        create_OP_DAT_FormalParameters (
            DataSizesDeclarationOfIndirectLoop & dataSizesDeclarationOfIndirectLoop,
            ParallelLoop & parallelLoop);

  public:

        KernelSubroutineOfIndirectLoop (
            std::string const & subroutineName,
            UserDeviceSubroutine & userDeviceSubroutine,
            DataSizesDeclarationOfIndirectLoop & dataSizesDeclarationOfIndirectLoop,
            ParallelLoop & parallelLoop, SgScopeStatement * moduleScope);

};

#endif
