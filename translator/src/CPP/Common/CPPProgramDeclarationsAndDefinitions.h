/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef CPP_PROGRAM_DECLARATIONS_AND_DEFINITIONS_H
#define CPP_PROGRAM_DECLARATIONS_AND_DEFINITIONS_H

#include <ProgramDeclarationsAndDefinitions.h>

class CPPProgramDeclarationsAndDefinitions: public ProgramDeclarationsAndDefinitions <
    SgFunctionDeclaration> ,
    public AstSimpleProcessing
{
  private:

    std::vector <std::string> shortDeclarations;

    std::vector <std::string> integerDeclarations;

    std::vector <std::string> longDeclarations;

    std::vector <std::string> floatDeclarations;

    std::vector <std::string> doubleDeclarations;

  private:

    void
    detectAndHandleOP2Definition (SgVariableDeclaration * variableDeclaration,
        std::string const variableName, SgTypedefType * typeDefinition);

    void
    handleBaseTypeDeclaration (SgType * type, std::string const & variableName);

    virtual void
    visit (SgNode * node);

  public:

    bool
    isTypeShort (std::string const & variableName) const
    {
      using std::find;

      return find (shortDeclarations.begin (), shortDeclarations.end (),
          variableName) != shortDeclarations.end ();
    }

    bool
    isTypeInteger (std::string const & variableName) const
    {
      using std::find;

      return find (integerDeclarations.begin (), integerDeclarations.end (),
          variableName) != integerDeclarations.end ();
    }

    bool
    isTypeLong (std::string const & variableName) const
    {
      using std::find;

      return find (longDeclarations.begin (), longDeclarations.end (),
          variableName) != longDeclarations.end ();
    }

    bool
    isTypeFloat (std::string const & variableName) const
    {
      using std::find;

      return find (floatDeclarations.begin (), floatDeclarations.end (),
          variableName) != floatDeclarations.end ();
    }

    bool
    isTypeDouble (std::string const & variableName) const
    {
      using std::find;

      return find (doubleDeclarations.begin (), doubleDeclarations.end (),
          variableName) != doubleDeclarations.end ();
    }

    CPPProgramDeclarationsAndDefinitions (SgProject * project);
};

#endif
