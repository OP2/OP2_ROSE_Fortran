#pragma once
#ifndef CPP_PREPROCESS_H
#define CPP_PREPROCESS_H

#include <rose.h>
#include "OP2Definitions.h"

class CPPProgramDeclarationsAndDefinitions;

class CPPPreProcess: public AstSimpleProcessing
{
  private:
	SgProject* project;
	
	bool precompute;
	
	std::string newFileName;
	
	SgSourceFile* sourceFile;
    
    CPPProgramDeclarationsAndDefinitions* declarations;
    
    virtual void
    visit (SgNode* node);

	void
	generateSubsetDeclarationPrecomputedSubSet (std::string name, OpSubSetDefinition* opSubSet);
	
	void
	generateSubsetDeclarationInlineFilter (std::string name, OpSubSetDefinition* opSubSet);
	
	void
	generateWrapperFunction (OpSubSetDefinition* opSubSet);
	
	void
	generateKernelInlinedFilter (OpSubSetDefinition* opSubSet, SgFunctionCallExp * parLoop);
	
	void
	handleSubsetDeclarations ();

	void
	handleOPParLoopPrecomputedSubSet (OpSubSetDefinition* opSubSet, SgFunctionCallExp * parLoop);
	
	void
	handleOPParLoopInlineFilter (OpSubSetDefinition* opSubSet, SgFunctionCallExp * parLoop);
	
	void
	handleOPParLoops ();
    	
  public:
    
    CPPPreProcess (SgProject* project,
                   CPPProgramDeclarationsAndDefinitions* declarations);
};

#endif