#include <iostream>
#include <boost/lexical_cast.hpp>
#include <UDrawGraph.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

namespace
{
  std::string const beginGraph = "[\n";
  std::string const endGraph = "]\n";
  std::string const endVertex = "])),";
  std::string const newEdge = "\ne(\"tEdge\",";
  std::string const endEdge = ")";
  std::string const beginAttributes = "[";
  std::string const endAttibutes = "],";
  std::string const setDirectionalLessEdge = "a(\"_DIR\", \"none\"),";
}

std::string const
UDrawGraph::toString (COLOR color)
{
  switch (color)
  {
    case RED:
    {
      return "red";
    }

    case YELLOW:
    {
      return "yellow";
    }

    case BLUE:
    {
      return "blue";
    }

    case GREEN:
    {
      return "green";
    }

    case BLACK:
    {
      return "black";
    }
  }
}

std::string const
UDrawGraph::toString (SHAPE shape)
{
  switch (shape)
  {
    case BOX:
    {
      return "box";
    }

    case CIRCLE:
    {
      return "circle";
    }

    case ELLIPSE:
    {
      return "ellipse";
    }

    case RHOMBUS:
    {
      return "rhombus";
    }

    case TRIANGLE:
    {
      return "triangle";
    }
  }
}

std::string const
UDrawGraph::toString (EDGESHAPE edgeShape)
{
  switch (edgeShape)
  {
    case SOLID:
    {
      return "solid";
    }

    case DOTTED:
    {
      return "dotted";
    }

    case DASHED:
    {
      return "dashed";
    }
  }
}

std::string const
UDrawGraph::newVertex (int vertexID)
{
  using boost::lexical_cast;
  using std::string;

  return "l(\"v" + lexical_cast <string> (vertexID) + "\",n(\"tVertex\",";
}

std::string const
UDrawGraph::edgeLink (int vertexID)
{
  using boost::lexical_cast;
  using std::string;

  return "r(\"v" + lexical_cast <string> (vertexID) + "\")";
}

std::string const
UDrawGraph::setName (std::string name)
{
  return "a(\"OBJECT\", \"" + name + "\"),";
}

std::string const
UDrawGraph::setColor (COLOR color)
{
  return "a(\"COLOR\", \"" + toString (color) + "\"),";
}

std::string const
UDrawGraph::setShape (SHAPE shape)
{
  return "a(\"_GO\", \"" + toString (shape) + "\"),";
}

std::string const
UDrawGraph::setToolTip (std::string tooltip)
{
  return "a(\"INFO\", \"" + tooltip + "\"),";
}

std::string const
UDrawGraph::setEdgePattern (EDGESHAPE shape, int width)
{
  using boost::lexical_cast;
  using std::string;

  return "a(\"EDGEPATTERN\", \"single;" + toString (shape) + ";"
      + lexical_cast <string> (width) + ";1\"),";
}

std::string const
UDrawGraph::setEdgeColor (COLOR color)
{
  return "a(\"EDGECOLOR\", \"" + toString (color) + "\"),";
}

void
UDrawGraph::visit (SgNode * node)
{
  using boost::lexical_cast;
  using std::string;

  nodes[node] = counter;

  if (counter != rootID)
  {
    file << newVertex (counter) << beginAttributes << setName (lexical_cast <
        string> (counter)) << setToolTip (node->class_name ()) << endAttibutes
        << beginAttributes;

    unsigned int const parentID = nodes[node->get_parent ()];

    file << newEdge << beginAttributes << endAttibutes << edgeLink (parentID)
        << endEdge << "\n";

    file << endVertex << "\n";
  }

  counter++;
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

UDrawGraph::UDrawGraph (SgProject * project)
{
  using boost::lexical_cast;
  using std::string;

  rootID = 1;

  counter = rootID;

  file.open ("ast.udraw");

  file << beginGraph;

  file << newVertex (rootID) << beginAttributes << setName (lexical_cast <
      string> (rootID)) << setToolTip (project->class_name ()) << endAttibutes
      << beginAttributes << endVertex << "\n";

  traverseInputFiles (project, preorder);

  file << endGraph;

  file.close ();
}
