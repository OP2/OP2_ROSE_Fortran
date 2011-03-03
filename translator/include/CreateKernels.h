/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * The parallel loop class used to recognise and manipulate 'op_par_loop' calls
 * in Fortran code.
 *
 * This class extends AstSimpleProcessing since that class provides
 * utilities to traverse the Abstract Syntax Tree (AST).
 */

// TODO: restructure in such a way that we have a single OpParLoop object per each op_par_loop call 
// found in the code (or an additional data structure per each call)

#ifndef CREATEKERNELS_H
#define CREATEKERNELS_H

#include <rose.h>

#include "OP2CommonDefinitions.h"
#include "OpDeclaredVariables.h"
#include "OpParLoop.h"

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
     * Used to retrieve OP_DAT types and all other declarations, if needed
     */
    OpDeclaredVariables * opDeclaredVariables;

    /*
     * The kernel output files
     */
    std::vector <SgSourceFile *> kernelOutputFiles;

    /*
     * References to all subroutines found during the visit of the input files
     */
    std::vector <SgProcedureHeaderStatement *> inputSubroutines;

    OpParLoop * opParLoop;

    /*
     * ====================================================================================================
     * Private functions
     * ====================================================================================================
     */

    void
    fix_OP_PAR_LOOP_Calls (SgFunctionCallExp * functionCallExp,
        SgProcedureHeaderStatement * hostSubroutine, SgScopeStatement * scope,
        std::string createdCUDAModuleName);

    void
    generateKernelSubroutine (SgSourceFile & sourceFile,
        std::string subroutineName, SgExpressionPtrList & args);

    /*
     * The following two functions are used to generate the main CUDA kernel routine: not implemented for now
     */
    void
    lookupArgumentsTypes (std::vector <SgType *> * argTypes,
        SgExpressionPtrList& args, SgVariableDeclaration * setSizeFormalPar);

    /*
     * Builds the parameter of the user kernel subroutine called by the main kernel subroutine
     */
    SgExprListExp *
    buildUserKernelParams (SgFunctionParameterList * mainKernelParameters,
        SgVarRefExp * iterSetVarRef, SgScopeStatement * subroutineScope);

    void
    createHostSubroutineStatements (SgScopeStatement * subroutineScope,
        SgExpressionPtrList & args);

    void
    createHostSubroutineCUDAVariables (SgScopeStatement * subroutineScope);

    void
    createHostSubroutineLocals (SgScopeStatement * subroutineScope,
        SgExpressionPtrList & args);

    void
    createHostSubroutineFormalParamaters (SgScopeStatement * subroutineScope,
        SgExpressionPtrList & args, SgFunctionParameterList * hostParameters);

    /*
     * Creates a host subroutine which sets up CUDA run-time variables, such as block
     * and grid sizes, allocates memory on the device, calls the kernel, and deallocates
     * memory
     */
    SgProcedureHeaderStatement *
    createHostSubroutine (SgExpressionPtrList & args, SgSourceFile & sourceFile,
        SgScopeStatement * moduleScope);

    /*
     * Creates the main kernel, which is launched from the host and calls the
     * user-supplied subroutine
     */
    void
    createMainKernelSubroutine (SgScopeStatement * moduleScope,
        SgExpressionPtrList & args, Sg_File_Info & fileInfo);

    /*
     * Copies the user kernel function definition with some modifications so
     * that it can run on the device
     */
    void
    createCUDAKernel (SgScopeStatement * moduleScope);

    SgModuleStatement *
    buildClassDeclarationAndDefinition (std::string moduleName,
        SgScopeStatement * fileScope);

    /*
     * Creates the Fortran module including all subroutines implementing an OP_PAR_LOOP
     */
    SgScopeStatement *
    createDirectLoopCUDAModule (SgSourceFile & sourceFile,
        std::string const moduleName);

    /*
     * Creates the source file to be unparsed
     */
    SgSourceFile *
    createSourceFile ();

    /*
     * Retrieves the OP_PAR_LOOP argument data types
     */
    void
    retrieveArgumentsTypes (SgExpressionPtrList & args);

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
        OpDeclaredVariables * opDeclaredVariables)
    {
      this->project = project;
      this->opDeclaredVariables = opDeclaredVariables;
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
