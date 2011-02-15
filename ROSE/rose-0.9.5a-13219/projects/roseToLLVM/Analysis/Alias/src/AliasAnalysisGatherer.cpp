#include <AliasAnalysisGatherer.h>
#include <llvm/Analysis/AliasSetTracker.h>
#include <llvm/Function.h>
#include <llvm/Assembly/Writer.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ADT/DenseSet.h>

using namespace llvm;

char AAGatherer::ID = 0;

static RegisterPass<AAGatherer>
X("aa-gather", "Gather Alias Analysis Results", false, true);

FunctionPass* llvm::createAAGathererPass() { return new AAGatherer(); }

void AAGatherer::addblocks(AliasSetTracker* _asTracker, Function &F) 
{
    // Adding all basic blocks in a function    
    
    Function::iterator I;

    for (I = F.begin(); I != F.end(); ++I) {
         BasicBlock *_BB = static_cast<BasicBlock*> (I);
        _asTracker->add(*_BB);
    }
    
}

std::string AAGatherer::WriteAsString(AliasSetTracker* _asTracker, Function &F)
{
    std::string result;
    raw_string_ostream _aaResults(result);


    // iterate over alias sets
    AliasSetTracker::iterator _asIter;
    for(_asIter = _asTracker->begin(); _asIter != _asTracker->end(); ++_asIter) {
        AliasSet *_aliasset = static_cast<AliasSet*> (_asIter);
        AliasSet::iterator I, E;

        _aaResults << "{";

        for( I = _aliasset->begin(), E = _aliasset->end(); I != E ; ++I) {

            // iterate over each pointer to write it as string
            WriteAsOperand(_aaResults, I.getPointer(), false, F.getParent());
            _aaResults << ",";
        }

         /*
         * Write the Type of Alias Set
         */

        if (_aliasset->isMayAlias() )
            _aaResults << "May";
        else if(_aliasset->isMustAlias() )
            _aaResults << "Must";
        else
            _aaResults << "No"; 


        _aaResults << "}";      
        
    }
    return _aaResults.str();   
}


bool AAGatherer::runOnFunction(Function &F) 
{
    AliasAnalysis &AA = getAnalysis<AliasAnalysis> ();    
    AliasSetTracker *_ASTracker = new AliasSetTracker(AA);

    // Gather Alias Sets
    addblocks(_ASTracker, F);

    AliasSetContainerList *list = AliasSetHandler::getInstance()->getAliasSetContainerList(F.getParent()->getModuleIdentifier());

    list->addNewFunction(F.getNameStr());

    std::string _AAResults = WriteAsString(_ASTracker, F);

    AliasSetContainer *container = list->getContainer(F.getNameStr());

    container->addaliasinglocations(_AAResults);

//     _ASTracker->dump();

    delete _ASTracker;
}

bool AAGatherer::doFinalization(Module &M) 
{
//    std::cout << "End of AA Gatherer Pass " << std::endl;
    return false;
}
