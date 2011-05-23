/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models the host subroutine for an indirect loop.
 * Its declarations and statements do the following:
 */

#ifndef HOST_SUBROUTINE_OF_INDIRECT_LOOP_H
#define HOST_SUBROUTINE_OF_INDIRECT_LOOP_H

#include <HostSubroutine.h>
#include <KernelSubroutine.h>
#include <InitialiseConstantsSubroutine.h>
#include <DataSizesDeclarationOfIndirectLoop.h>

class HostSubroutineOfIndirectLoop: public HostSubroutine
{
  private:

    /*
     * ======================================================
     * Returns the name of the local variable which models
     * the mappings from local indices to global indices in
     * shared memory
     * ======================================================
     */
    static std::string
    getLocalToGlobalMappingVariableName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectLoop::Fortran::VariablePrefixes::pindMaps + lexical_cast <
          string> (OP_DAT_ArgumentGroup);
    }

    /*
     * ======================================================
     * Returns the name of the local variable which models
     * the indirect mappings to local indices in shared memory
     * ======================================================
     */
    static std::string
    getGlobalToLocalMappingVariableName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectLoop::Fortran::VariablePrefixes::pMaps + lexical_cast <
          string> (OP_DAT_ArgumentGroup);
    }

    /*
     * ======================================================
     * Returns the name of the local variable which models
     * the number of indirect elements in each block
     * ======================================================
     */
    static std::string
    getGlobalToLocalMappingSizeVariableName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectLoop::Fortran::VariablePrefixes::pMaps + lexical_cast <
          string> (OP_DAT_ArgumentGroup)
          + IndirectAndDirectLoop::Fortran::VariableSuffixes::Size;
    }

    void
        initialiseDeviceVariablesSizesVariable (
            ParallelLoop & parallelLoop,
            DataSizesDeclarationOfIndirectLoop & dataSizesDeclarationOfIndirectLoop);

    void
    createExecutionPlanExecutionStatements (
        KernelSubroutine & kernelSubroutine, ParallelLoop & parallelLoop);

    void
    initialiseVariablesAndConstants (
        InitialiseConstantsSubroutine & initialiseConstantsSubroutine);

    void
    createPlanCToForttranPointerConversionStatements (
        ParallelLoop & parallelLoop);

    void
    createPlanFunctionCallStatement ();

    void
    createDoLoopToCorrectIndexing (ParallelLoop & parallelLoop);

    void
    createExecutionPlanStatements (ParallelLoop & parallelLoop);

    void
        createExecutionPlanLocalVariables (
            DataSizesDeclarationOfIndirectLoop & dataSizesDeclarationOfIndirectLoop,
            ParallelLoop & parallelLoop);

  public:

        HostSubroutineOfIndirectLoop (
            std::string const & subroutineName,
            UserDeviceSubroutine & userDeviceSubroutine,
            KernelSubroutine & kernelSubroutine,
            InitialiseConstantsSubroutine & initialiseConstantsSubroutine,
            DataSizesDeclarationOfIndirectLoop & dataSizesDeclarationOfIndirectLoop,
            ParallelLoop & parallelLoop, SgScopeStatement * moduleScope);
};

#endif
