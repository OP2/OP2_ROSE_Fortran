#line 1 "buildCheckers.C.in"
// This is an automatically generated file

#if ROSE_MPI
#include <mpi.h>
#endif
#include <rose.h>
#include "rose_config.h"

#include <iostream>
#include <vector>
#include "compass.h"
#include "checkers.h"

//AS(1/18/08) Read in the selecting of rules
std::map<std::string, bool > 
readFile( std::string filename){
  std::map<std::string, bool > checkersToUse;
  std::ifstream* file_op = Compass::openFile( filename );

  std::string current_word;
  //bool is_first_word = true;
  //read file
  char dummyString[2000];

  while((*file_op) >> current_word){
	//First word denotes what the regular expression should operate
	//upon. Second word denotes the regular expression

	if(  current_word.substr(0,1) == std::string("#") ){
	  //Skip rest of the line if a "#" character is found. This denotes a 
	  //comment
	  file_op->getline(dummyString,2000);

	}else{
	  if(current_word.substr(0,2) == "+:" ){
		checkersToUse[current_word.substr(2)] = true;
	  }else if(current_word.substr(0,2) == "-:" ){
		checkersToUse[current_word.substr(2)] = false;
	  }
	}
  }

  return checkersToUse;
}



void
buildCheckers( std::vector<const Compass::Checker*> &retVal, 
	       Compass::Parameters &params, 
	       Compass::OutputObject &output, SgProject* proj)
{

//ROSE_ASSERT(pr);

#define BUILD_ONE_CHECKER(NameLowercase) \
    try { \
        retVal.push_back(NameLowercase ## Checker); \
    } catch (const std::exception &e) { \
        std::cerr << "error initializing checker " #NameLowercase ": " << e.what() << std::endl; \
    } \

#line 1 "buildCheckers-inserted-code"
  // @@@ -- checker names inserted here
BUILD_ONE_CHECKER(allocateAndFreeMemoryInTheSameModule)
BUILD_ONE_CHECKER(allowedFunctions)
BUILD_ONE_CHECKER(assignmentOperatorCheckSelf)
BUILD_ONE_CHECKER(assignmentReturnConstThis)
BUILD_ONE_CHECKER(asynchronousSignalHandler)
BUILD_ONE_CHECKER(avoidUsingTheSameHandlerForMultipleSignals)
BUILD_ONE_CHECKER(binPrintAsmFunctions)
BUILD_ONE_CHECKER(binPrintAsmInstruction)
BUILD_ONE_CHECKER(binaryBufferOverflow)
BUILD_ONE_CHECKER(binaryInterruptAnalysis)
BUILD_ONE_CHECKER(booleanIsHas)
BUILD_ONE_CHECKER(bufferOverflowFunctions)
BUILD_ONE_CHECKER(byteByByteStructureComparison)
BUILD_ONE_CHECKER(charStarForString)
BUILD_ONE_CHECKER(commaOperator)
BUILD_ONE_CHECKER(computationalFunctions)
BUILD_ONE_CHECKER(constCast)
BUILD_ONE_CHECKER(constStringLiterals)
BUILD_ONE_CHECKER(constructorDestructorCallsVirtualFunction)
BUILD_ONE_CHECKER(controlVariableTestAgainstFunction)
BUILD_ONE_CHECKER(copyConstructorConstArg)
BUILD_ONE_CHECKER(cppCallsSetjmpLongjmp)
BUILD_ONE_CHECKER(cycleDetection)
BUILD_ONE_CHECKER(cyclomaticComplexity)
BUILD_ONE_CHECKER(dataMemberAccess)
BUILD_ONE_CHECKER(deepNesting)
BUILD_ONE_CHECKER(defaultCase)
BUILD_ONE_CHECKER(defaultConstructor)
BUILD_ONE_CHECKER(discardAssignment)
BUILD_ONE_CHECKER(doNotAssignPointerToFixedAddress)
BUILD_ONE_CHECKER(doNotCallPutenvWithAutoVar)
BUILD_ONE_CHECKER(doNotDeleteThis)
BUILD_ONE_CHECKER(doNotUseCstyleCasts)
BUILD_ONE_CHECKER(duffsDevice)
BUILD_ONE_CHECKER(dynamicCast)
BUILD_ONE_CHECKER(emptyInsteadOfSize)
BUILD_ONE_CHECKER(enumDeclarationNamespaceClassScope)
BUILD_ONE_CHECKER(explicitCharSign)
BUILD_ONE_CHECKER(explicitCopy)
BUILD_ONE_CHECKER(explicitTestForNonBooleanValue)
BUILD_ONE_CHECKER(fileReadOnlyAccess)
BUILD_ONE_CHECKER(floatForLoopCounter)
BUILD_ONE_CHECKER(floatingPointExactComparison)
BUILD_ONE_CHECKER(fopenFormatParameter)
BUILD_ONE_CHECKER(forLoopConstructionControlStmt)
BUILD_ONE_CHECKER(forLoopCppIndexVariableDeclaration)
BUILD_ONE_CHECKER(forbiddenFunctions)
BUILD_ONE_CHECKER(friendDeclarationModifier)
BUILD_ONE_CHECKER(functionCallAllocatesMultipleResources)
BUILD_ONE_CHECKER(functionDefinitionPrototype)
BUILD_ONE_CHECKER(functionDocumentation)
BUILD_ONE_CHECKER(inductionVariableUpdate)
BUILD_ONE_CHECKER(internalDataSharing)
BUILD_ONE_CHECKER(locPerFunction)
BUILD_ONE_CHECKER(localizedVariables)
BUILD_ONE_CHECKER(lowerRangeLimit)
BUILD_ONE_CHECKER(magicNumber)
BUILD_ONE_CHECKER(mallocReturnValueUsedInIfStmt)
BUILD_ONE_CHECKER(multiplePublicInheritance)
BUILD_ONE_CHECKER(nameAllParameters)
BUILD_ONE_CHECKER(newDelete)
BUILD_ONE_CHECKER(noAsmStmtsOps)
BUILD_ONE_CHECKER(noExceptions)
BUILD_ONE_CHECKER(noExitInMpiCode)
BUILD_ONE_CHECKER(noGoto)
BUILD_ONE_CHECKER(noOverloadAmpersand)
BUILD_ONE_CHECKER(noRand)
BUILD_ONE_CHECKER(noSecondTermSideEffects)
BUILD_ONE_CHECKER(noSideEffectInSizeof)
BUILD_ONE_CHECKER(noTemplateUsage)
BUILD_ONE_CHECKER(noVariadicFunctions)
BUILD_ONE_CHECKER(noVfork)
BUILD_ONE_CHECKER(nonAssociativeRelationalOperators)
BUILD_ONE_CHECKER(nonStandardTypeRefArgs)
BUILD_ONE_CHECKER(nonStandardTypeRefReturns)
BUILD_ONE_CHECKER(nonVirtualRedefinition)
BUILD_ONE_CHECKER(nonmemberFunctionInterfaceNamespace)
BUILD_ONE_CHECKER(nullDeref)
BUILD_ONE_CHECKER(ompPrivateLock)
BUILD_ONE_CHECKER(oneLinePerDeclaration)
BUILD_ONE_CHECKER(operatorOverloading)
BUILD_ONE_CHECKER(otherArgument)
BUILD_ONE_CHECKER(placeConstantOnTheLhs)
BUILD_ONE_CHECKER(pointerComparison)
BUILD_ONE_CHECKER(preferAlgorithms)
BUILD_ONE_CHECKER(preferFseekToRewind)
BUILD_ONE_CHECKER(preferSetvbufToSetbuf)
BUILD_ONE_CHECKER(protectVirtualMethods)
BUILD_ONE_CHECKER(pushBack)
BUILD_ONE_CHECKER(rightShiftMask)
BUILD_ONE_CHECKER(setPointersToNull)
BUILD_ONE_CHECKER(singleParameterConstructorExplicitModifier)
BUILD_ONE_CHECKER(sizeOfPointer)
BUILD_ONE_CHECKER(stringTokenToIntegerConverter)
BUILD_ONE_CHECKER(subExpressionEvaluationOrder)
BUILD_ONE_CHECKER(ternaryOperator)
BUILD_ONE_CHECKER(time_tDirectManipulation)
BUILD_ONE_CHECKER(unaryMinus)
BUILD_ONE_CHECKER(uninitializedDefinition)
BUILD_ONE_CHECKER(upperRangeLimit)
BUILD_ONE_CHECKER(variableNameEqualsDatabaseName)
BUILD_ONE_CHECKER(voidStar)
  // @@@ -- checker names inserted here

#line 63 "buildCheckers.C.in"
  //AS(1/18/2008) Remove the rules that has been deselected from the retVal
  std::string ruleFile = Compass::parseString(params["Compass.RuleSelection"]);

  std::map<std::string, bool > ruleSelection = readFile(ruleFile);

  std::vector<const Compass::Checker*> ruleDeselected;
  for( std::vector<const Compass::Checker*>::reverse_iterator iItr = retVal.rbegin();
	  iItr != retVal.rend(); iItr++ )
  {
	std::map<std::string, bool >::iterator isRuleSelected = ruleSelection.find((*iItr)->checkerName );   
      
	if( isRuleSelected == ruleSelection.end() ){
         std::cerr << "Error: It has not been selected if " + (*iItr)->checkerName + " should be run." 
		           << std::endl;
		 exit(1);
	}
	if( isRuleSelected->second == false  )
	  ruleDeselected.push_back(*iItr);
  }

   for( std::vector<const Compass::Checker*>::iterator iItr = ruleDeselected.begin();
	  iItr != ruleDeselected.end(); iItr++ )
  {
	retVal.erase(std::find(retVal.begin(),retVal.end(),*iItr));

  }

#if 0
   bool useDefAnalysis = false;

   std::vector<const Compass::Checker*>::const_iterator it;
   for (it= retVal.begin(); it!=retVal.end(); ++it) {
     const Compass::Checker* base = *it;
     try {
       std::string result = 
	 Compass::parseString(params[base->checkerName+".defUseAnalysis"]);
       if (result=="true") {
	 useDefAnalysis = true;
	 std::cout << " DefUse Analysis is used in " << base->checkerName << std::endl;
       }
     } catch (const std::exception &e) { 
     }     
     
   }
   if (useDefAnalysis) {
     std::cout << "  >> Using DefUse Analysis ... " << std::endl;

#if ROSE_GCC_OMP
#pragma omp critical (runDefUseAnalysisCompass)
#endif
     Compass::runDefUseAnalysis(pr);
   } else {
     std::cout << "  >> DefUse Analysis disabled... " << std::endl;
   }
#endif
  return;
} //buildCheckers()

