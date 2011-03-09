/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * The parallel loop class used to recognise and manipulate 'op_par_loop' calls
 * in Fortran code.
 *
 * This class extends AstSimpleProcessing since that class provides
 * utilities to traverse the Abstract Syntax Tree (AST).
 */

#ifndef CREATE_KERNELS_H
#define CREATE_KERNELS_H

#include <rose.h>

#include "OP2CommonDefinitions.h"
#include "OP2DeclaredVariables.h"
#include "OP2ParallelLoop.h"

class CreateKernels: public AstSimpleProcessing
{
  private:
    /*
     * ====================================================================================================
     * Private data
     * ====================================================================================================
     */

    /*
     * The project which the source-to-source translator operates upon
     */
    SgProject * project;

    /*
     * References to all subroutines found during the visit of the input files
     */
    std::vector <SgProcedureHeaderStatement *> inputSubroutines;

    /*
     * The generated CUDA-Fortran output files for each kernel
     */
    std::vector <SgSourceFile *> CUDAOutputFiles;

    /*
     * Used to retrieve OP_DAT types and all other declarations, if needed
     */
    OP2DeclaredVariables * op2DeclaredVariables;

    /*
     * A mapping from a kernel name to our internal representation of an
     * OP_PAR_LOOP
     */
    std::map <std::string, OP2ParallelLoop *> op2ParallelLoops;

    /*
     * ====================================================================================================
     * Private functions
     * ====================================================================================================
     */

    void
    fix_OP_PAR_LOOP_Calls (SgScopeStatement * scope,
        OP2ParallelLoop * op2ParallelLoop, SgFunctionCallExp * functionCallExp,
        SgProcedureHeaderStatement * hostSubroutine);

    void
    setUp_OP_DAT_ArgumentTypes (std::vector <SgType *> & opDatArgumentTypes,
        SgVariableDeclaration * setSizeFormalParameter,
        OP2ParallelLoop * op2ParallelLoop);

    SgExprListExp *
    createUserDeviceFunctionParameters (SgFunctionParameterList * mainKernelParameters,
        SgVarRefExp * iterSetVarRef, SgScopeStatement * subroutineScope,
        OP2ParallelLoop * op2ParallelLoop);

    void
    createHostSubroutineStatements (SgScopeStatement * subroutineScope,
        OP2ParallelLoop * op2ParallelLoop);

    void
    createHostSubroutineCUDAVariables (SgScopeStatement * subroutineScope,
        OP2ParallelLoop * op2ParallelLoop);

    void
    createHostSubroutineLocals (SgScopeStatement * subroutineScope,
        OP2ParallelLoop * op2ParallelLoop);

    void
    createHostSubroutineFormalParamaters (SgScopeStatement * subroutineScope,
        SgFunctionParameterList * hostParameters,
        OP2ParallelLoop * op2ParallelLoop);

    /*
     * Creates a host subroutine which sets up CUDA run-time variables, such as block
     * and grid sizes, allocates memory on the device, calls the kernel, and deallocates
     * memory
     */
    SgProcedureHeaderStatement *
    createHostSubroutine (SgScopeStatement * moduleScope,
        OP2ParallelLoop * op2ParallelLoop);

    /*
     * Creates the main kernel, which is launched from the host and calls the
     * user-supplied function
     */
    void
    createMainKernelDeviceSubroutine (SgScopeStatement * moduleScope,
        OP2ParallelLoop * op2ParallelLoop);

    /*
     * Copies the user function (supplied to the kernel) and applies some
     * modifications so that it can run on the device
     */
    void
    copyAndModifyUserFunction (SgScopeStatement * moduleScope,
        OP2ParallelLoop * op2ParallelLoop);

    SgModuleStatement *
    buildClassDeclarationAndDefinition (SgScopeStatement * fileScope,
        OP2ParallelLoop * op2ParallelLoop);

    /*
     * Creates the Fortran module including all subroutines implementing an OP_PAR_LOOP
     */
    SgScopeStatement *
    createDirectLoopCUDAModule (SgSourceFile & sourceFile,
        OP2ParallelLoop * op2ParallelLoop);

    /*
     * Creates the source file to be unparsed
     */
    SgSourceFile *
    createSourceFile (OP2ParallelLoop * op2ParallelLoop);

  public:
    /*
     * ====================================================================================================
     * Public functions
     * ====================================================================================================
     */

    /*
     * Constructor requires the source files contained in the
     * project to detect OP_PAR_LOOPs
     */
    CreateKernels (SgProject * project,
        OP2DeclaredVariables * op2DeclaredVariables)
    {
      this->project = project;
      this->op2DeclaredVariables = op2DeclaredVariables;
    }

    /*
     * Visits each vertex in the AST
     */
    virtual void
    visit (SgNode *n);

    /*
     * Generates output files for each kernel
     */
    void
    unparse ();
};

#endif
