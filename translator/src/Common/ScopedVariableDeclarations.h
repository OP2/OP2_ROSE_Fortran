/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class maintains a mapping between a variable name
 * and its declaration in ROSE
 */

#ifndef SCOPED_VARIABLE_DECLARATIONS_H
#define SCOPED_VARIABLE_DECLARATIONS_H

#include <map>
#include <string>

class SgVariableDeclaration;
class SgVarRefExp;

class ScopedVariableDeclarations
{
  private:

    std::map <std::string, SgVariableDeclaration *> theDeclarations;

  public:

    SgVarRefExp *
    getReference (std::string const & variableName);

    void
    add (std::string const & variableName, SgVariableDeclaration * declaration);

    ScopedVariableDeclarations ();
};

#endif
