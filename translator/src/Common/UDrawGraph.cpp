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
  std::string const fileNameSuffix = ".udraw";
  std::string const beginGraph = "[\n";
  std::string const endGraph = "]\n";
  std::string const endVertex = "])),\n";
  std::string const newEdge = "\ne(\"tEdge\",";
  std::string const endEdge = ")\n";
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

  file << newVertex (counter) << beginAttributes;

  switch (node->variantT ())
  {
    case V_SgInitializedName:
    {
      SgInitializedName * castedNode = isSgInitializedName (node);
      file << setName (castedNode->get_name ().getString ());
      break;
    }

    case V_SgVarRefExp:
    {
      SgVarRefExp * castedNode = isSgVarRefExp (node);
      file << setName (castedNode->get_symbol ()->get_name ().getString ());
      file << setColor (RED);
      break;
    }

    case V_SgVariableDeclaration:
    {
      file << setName (node->class_name ());
      file << setShape (RHOMBUS);
      break;
    }

    case V_SgIntVal:
    {
      SgIntVal * castedNode = isSgIntVal (node);
      file << setName (lexical_cast <string> (castedNode->get_value ()));
      file << setColor (GREEN);
      break;
    }

    case V_SgFloatVal:
    {
      SgFloatVal * castedNode = isSgFloatVal (node);
      file << setName (lexical_cast <string> (castedNode->get_value ()));
      file << setColor (GREEN);
      break;
    }

    case V_SgAddOp:
    {
      file << setName ("+");
      file << setColor (YELLOW);
      break;
    }

    case V_SgMinusOp:
    case V_SgSubtractOp:
    {
      file << setName ("-");
      file << setColor (YELLOW);
      break;
    }

    case V_SgMultiplyOp:
    {
      file << setName ("*");
      file << setColor (YELLOW);
      break;
    }

    case V_SgDivideOp:
    {
      file << setName ("/");
      file << setColor (YELLOW);
      break;
    }

    case V_SgAssignOp:
    {
      file << setName ("=");
      file << setColor (YELLOW);
      break;
    }

    case V_SgLessThanOp:
    {
      file << setName ("<");
      file << setColor (YELLOW);
      break;
    }

    case V_SgLessOrEqualOp:
    {
      file << setName ("<=");
      file << setColor (YELLOW);
      break;
    }

    case V_SgGreaterThanOp:
    {
      file << setName (">");
      file << setColor (YELLOW);
      break;
    }

    case V_SgGreaterOrEqualOp:
    {
      file << setName (">=");
      file << setColor (YELLOW);
      break;
    }

    case V_SgAddressOfOp:
    {
      file << setName ("&");
      file << setColor (YELLOW);
      break;
    }

    default:
    {
      file << setName (node->class_name ());
      break;
    }
  }

  file << endAttibutes << beginAttributes;

  if (counter != rootID)
  {
    SgNode * parent = node->get_parent ();

    unsigned int const parentID = nodes[parent];

    file << newEdge << beginAttributes << setDirectionalLessEdge
        << endAttibutes << edgeLink (parentID) << endEdge;
  }

  file << endVertex;

  nodes[node] = counter;

  counter++;
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

UDrawGraph::UDrawGraph (SgProject * project, std::string const & fileNameStem)
{
  using std::string;

  rootID = 1;

  counter = rootID;

  string const fileName = fileNameStem + fileNameSuffix;

  file.open (fileName.c_str ());

  file << beginGraph;

  traverseInputFiles (project, preorder);

  file << endGraph;

  file.close ();
}
