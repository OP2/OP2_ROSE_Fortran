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

    TargetLanguage::FRONTEND frontend;

    TargetLanguage::BACKEND backend;

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
     * Set the host language
     * ======================================================
     */
    void
    setHostLanguage (TargetLanguage::FRONTEND frontend);

    /*
     * ======================================================
     * What is the host language?
     * ======================================================
     */
    TargetLanguage::FRONTEND
    getHostLanguage () const;

    /*
     * ======================================================
     * Set which type of code should be generated
     * ======================================================
     */
    void
    setTargetBackend (TargetLanguage::BACKEND backend);

    /*
     * ======================================================
     * What type of code should be generated?
     * ======================================================
     */
    TargetLanguage::BACKEND
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
