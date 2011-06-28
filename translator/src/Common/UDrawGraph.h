#ifndef UDRAWGRAPH_H
#define UDRAWGRAPH_H

#include <rose.h>
#include <map>

class UDrawGraph: public AstSimpleProcessing
{
  private:

    enum COLOR
    {
      RED, YELLOW, BLUE, GREEN, BLACK
    };

    enum SHAPE
    {
      BOX, CIRCLE, ELLIPSE, RHOMBUS, TRIANGLE
    };

    enum EDGESHAPE
    {
      SOLID, DOTTED, DASHED
    };

  private:

    std::map <SgNode *, unsigned int> nodes;

    unsigned int counter;

    unsigned int rootID;

    std::ofstream file;

  private:

    std::string const
    toString (COLOR color);

    std::string const
    toString (SHAPE shape);

    std::string const
    toString (EDGESHAPE edgeShape);

    std::string const
    newVertex (int vertexID);

    std::string const
    edgeLink (int vertexID);

    std::string const
    setName (std::string name);

    std::string const
    setColor (COLOR color);

    std::string const
    setShape (SHAPE shape);

    std::string const
    setToolTip (std::string tooltip);

    std::string const
    setEdgePattern (EDGESHAPE shape, int width);

    std::string const
    setEdgeColor (COLOR color);

    virtual void
    visit (SgNode * node);

  public:

    UDrawGraph (SgProject * project);
};

#endif
