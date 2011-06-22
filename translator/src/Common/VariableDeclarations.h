/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class maintains a mapping between a variable name
 * and its declaration in ROSE
 */

#ifndef VARIABLE_DECLARATIONS_H
#define VARIABLE_DECLARATIONS_H

#include <map>
#include <string>
#include <rose.h>

class VariableDeclarations
{
  private:

    std::map <std::string, SgVariableDeclaration *> theDeclarations;

  public:

    bool
    exists (std::string const & variableName);

    SgVariableDeclaration *
    get (std::string const & variableName);

    void
    add (std::string const & variableName, SgVariableDeclaration * declaration);

    VariableDeclarations ();

    VariableDeclarations (
        std::vector <VariableDeclarations *> & allDeclarations);
};

#endif
