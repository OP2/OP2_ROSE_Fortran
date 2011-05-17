/*

Copyright (c) 2010, Graham Markall
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Imperial College London nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

//
// op2rose.cpp
//
// A ROSE Implementation of the OPlus2 prototype 
// op2.m that was current at 27/01/2010.
//

#include <rose.h>

#include "rose_op2parloop.h"
#include "rose_op2build.h"
#include "rose_op2source.h"

using namespace std;
using namespace SageBuilder;
using namespace SageInterface;


int main(int argc, char **argv) 
{
  SgProject *project = frontend(argc, argv);
  ROSE_ASSERT(project != NULL);

  cout << "Parsed file, beginning AST traversal." << endl;

  OPParLoop p;
  p.setProject(project);
  p.traverseInputFiles(project,preorder);
  p.generateGlobalKernelsHeader();
  p.unparse();

	OPBuild c;
	c.setProject(project);
  c.setParLoop(&p);
	c.traverseInputFiles(project,preorder);
	c.generateBuildFile();
	c.unparse();
  
  OPSource source;
  source.cudaFunctionDeclarations = p.cudaFunctionDeclarations;
  source.setProject(project);
  source.traverseInputFiles(project,preorder);
  source.unparse();
  
  cout << "Process completed." << endl;
  
  return 0;
}

