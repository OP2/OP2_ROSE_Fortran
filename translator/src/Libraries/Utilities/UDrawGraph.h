


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



#pragma once
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

    UDrawGraph (SgProject * project, std::string const & fileNameStem);
};

#endif
