/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Class storing global variables set by the user on the command line.
 * Implemented by singleton pattern as we should only ever
 * create a single instance
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <CommonNamespaces.h>

class Globals
{
  private:

    static Globals * globalsInstance;

    TargetBackends::BACKEND_VALUE backend;

    bool oxfordOption;

    bool uDrawOption;

    std::vector <std::string> inputFiles;

    std::string constantsFileName;

  private:

    /*
     * ======================================================
     * Private constructor ensures users can never create
     * multiple instances
     * ======================================================
     */
    Globals ();

  public:

    /*
     * ======================================================
     * This always returns a single instance of the Globals
     * class to make it compliant with the singleton pattern
     * ======================================================
     */
    static Globals *
    getInstance ();

    /*
     * ======================================================
     * Set which type of code should be generated
     * ======================================================
     */
    void
    setTargetBackend (TargetBackends::BACKEND_VALUE backend);

    /*
     * ======================================================
     * What type of code should be generated?
     * ======================================================
     */
    TargetBackends::BACKEND_VALUE
    getTargetBackend () const;

    void
    setRenderOxfordAPICalls ();

    bool
    renderOxfordAPICalls () const;

    void
    setOutputUDrawGraphs ();

    bool
    outputUDrawGraphs () const;

    void
    addInputFile (std::string const & fileName);

    bool
    isInputFile (std::string const & fileName) const;

    void
    setConstantsFileName (std::string const & fileName);

    std::string const &
    getConstantsFileName () const;
};

#endif
