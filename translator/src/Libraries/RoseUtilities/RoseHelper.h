/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Helper functions to build ROSE expressions and statements
 */

#pragma once
#ifndef ROSE_HELPER_H
#define ROSE_HELPER_H

class Sg_File_Info;
class SgNode;

class RoseHelper
{
  private:

    static Sg_File_Info * fileInfo;

  public:

    /*
     * ======================================================
     * An Sg_File_Info object is often needed when building
     * ROSE expressions. Rather than build a fresh one for
     * each such expression, this returns a general one to
     * be used globally
     * ======================================================
     */

    static Sg_File_Info *
    getFileInfo ();

    static void
    forceOutputOfCodeToFile (SgNode * root);
};

#endif
