


/*  Open source copyright declaration based on BSD open source template:
 *  http://www.opensource.org/licenses/bsd-license.php
 * 
 * Copyright (c) 2011-2012, Adam Betts, Carlo Bertolli
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


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
