/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Class storing global variables set by the user on the command line.
 * Implemented by singleton pattern as we should only ever
 * create a single instance
 */

#pragma once
#ifndef GLOBALS_H
#define GLOBALS_H

#include <TargetBackend.h>

class Globals
{
  private:

    static Globals * globalsInstance;

    TargetBackend::BACKEND_VALUE backend;

    bool oxfordOption;

    bool uDrawOption;

    std::vector <std::string> inputFiles;

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
    setTargetBackend (TargetBackend::BACKEND_VALUE backend);

    /*
     * ======================================================
     * What type of code should be generated?
     * ======================================================
     */
    TargetBackend::BACKEND_VALUE
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
};

#endif
