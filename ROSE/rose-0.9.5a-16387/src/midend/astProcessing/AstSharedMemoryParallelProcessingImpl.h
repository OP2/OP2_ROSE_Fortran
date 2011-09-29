// Author: Gergo Barany
// $Id: AstSharedMemoryParallelProcessing.C,v 1.1 2008/01/08 02:56:38 dquinlan Exp $

#ifndef ASTSHAREDMEMORYPARALLELPROCESSING_C
#define ASTSHAREDMEMORYPARALLELPROCESSING_C

// tps (01/08/2010) Added sage3basic since this doesnt compile under gcc4.1.2
//#include "sage3basic.h"
//#include "sage3.h"

// DQ (3/20/2009): Trying to fix error on Cygwin platform.
#ifdef _MSC_VER
#pragma message ("Error: pthread.h is unavailable on MSVC, we might want to use boost.thread library.")
#else
#include <pthread.h>
#endif

#include "AstSharedMemoryParallelProcessing.h"

// Throughout this file, I is the InheritedAttributeType, S is the
// SynthesizedAttributeType -- the type names are still horrible

// parallel TOP DOWN BOTTOM UP implementation

template <class I, class S>
AstSharedMemoryParallelizableTopDownBottomUpProcessing<I, S>::
AstSharedMemoryParallelizableTopDownBottomUpProcessing(
        const AstSharedMemoryParallelProcessingSynchronizationInfo &syncInfo,
        const typename AstSharedMemoryParallelizableTopDownBottomUpProcessing<I, S>::TraversalPtrList &t)
    : AstCombinedTopDownBottomUpProcessing<I, S>(t),
      AstSharedMemoryParallelProcessingSynchronizationBase(syncInfo),
      visitedNodes(0), runningParallelTraversal(false),
      synchronizationWindowSize(syncInfo.synchronizationWindowSize)
{
}

template <class I, class S>
void 
AstSharedMemoryParallelizableTopDownBottomUpProcessing<I, S>::set_runningParallelTraversal(bool val)
{
    runningParallelTraversal = val;
}

template <class I, class S>
typename AstSharedMemoryParallelizableTopDownBottomUpProcessing<I, S>::InheritedAttributeTypeList *
AstSharedMemoryParallelizableTopDownBottomUpProcessing<I, S>::
evaluateInheritedAttribute(SgNode *astNode,
        typename AstSharedMemoryParallelizableTopDownBottomUpProcessing<I, S>::InheritedAttributeTypeList *inheritedValues)
{
    if (runningParallelTraversal && ++visitedNodes > synchronizationWindowSize)
    {
        synchronize();
        visitedNodes = 0;
    }

    // let the superclass handle actual attribute evaluation
    return Superclass::evaluateInheritedAttribute(astNode, inheritedValues);
}

template <class I, class S>
void AstSharedMemoryParallelizableTopDownBottomUpProcessing<I, S>::
atTraversalEnd()
{
    // delegate the call to the superclass
    Superclass::atTraversalEnd();

    // signal that we are done
    if (runningParallelTraversal)
    {
        signalFinish();
        // clear the flag so subsequent traversals can be non-parallel
        runningParallelTraversal = false;
    }
}

// This class holds the arguments to a traversal thread: The parallelizable
// traversal class that contains a number of traversals, the node to start the
// traversal from, and the list of initial inherited values.
template <class I, class S>
struct AstSharedMemoryParallelTopDownBottomUpThreadArgs
{
    AstSharedMemoryParallelizableTopDownBottomUpProcessing<I, S> *traversal;
    SgNode *basenode;
    typename AstSharedMemoryParallelizableTopDownBottomUpProcessing<I, S>::InheritedAttributeTypeList *inheritedValues;

    AstSharedMemoryParallelTopDownBottomUpThreadArgs(
            AstSharedMemoryParallelizableTopDownBottomUpProcessing<I, S> *traversal,
            SgNode *basenode,
            typename AstSharedMemoryParallelizableTopDownBottomUpProcessing<I, S>::InheritedAttributeTypeList *inheritedValues)
        : traversal(traversal), basenode(basenode), inheritedValues(inheritedValues)
    {
    }
};

// This is the function that is executed in each thread. It basically unpacks
// its arguments and starts the traversal on them; the traversal's final
// result is returned. Synchronization is built into the parallelizable
// processing classes.
template <class I, class S>
void *parallelTopDownBottomUpProcessingThread(void *p)
{
    AstSharedMemoryParallelTopDownBottomUpThreadArgs<I, S> *threadArgs = (AstSharedMemoryParallelTopDownBottomUpThreadArgs<I, S> *) p;

    AstSharedMemoryParallelizableTopDownBottomUpProcessing<I, S> *traversal = threadArgs->traversal;
    SgNode *basenode = threadArgs->basenode;
    typename AstSharedMemoryParallelizableTopDownBottomUpProcessing<I, S>::InheritedAttributeTypeList
        *inheritedValues = threadArgs->inheritedValues;
    delete threadArgs;

    // Set the flag that indicates that this is indeed a parallel traversal;
    // it is cleared by the traversal class itself when it is done.
    traversal->set_runningParallelTraversal(true);
    // Start the traversal.
    return traversal->traverse(basenode, inheritedValues);
}

template <class I, class S>
typename AstSharedMemoryParallelTopDownBottomUpProcessing<I, S>::SynthesizedAttributeTypeList *
AstSharedMemoryParallelTopDownBottomUpProcessing<I, S>::traverseInParallel(
        SgNode *basenode,
        typename AstSharedMemoryParallelTopDownBottomUpProcessing<I, S>::InheritedAttributeTypeList *inheritedValues)
{
 // GB (09/27/2007): Is this really required? Inheritance of templated classes is just weird.
    const typename Superclass::TraversalPtrList &traversals = Superclass::traversals;

    size_t numberOfTraversals = traversals.size();
    size_t i;

    AstSharedMemoryParallelProcessingSynchronizationInfo syncInfo(numberOfThreads, synchronizationWindowSize);

    // Chop the flat list of traversals apart and distribute them into a few
    // parallelizable traversals.
    ParallelizableTraversalPtrList parallelTraversals(numberOfThreads);
    std::vector<InheritedAttributeTypeList *> parallelInheritedValues(numberOfThreads);
    size_t begin = 0, end;
    for (i = 0; i < numberOfThreads; i++)
    {
        end = begin + numberOfTraversals / numberOfThreads;
        if (end > numberOfTraversals)
            end = numberOfTraversals;

        parallelTraversals[i]
            = new AstSharedMemoryParallelizableTopDownBottomUpProcessing<I, S>(syncInfo,
                    std::vector<TraversalPtr>(traversals.begin() + begin, traversals.begin() + end));
        parallelInheritedValues[i]
            = new InheritedAttributeTypeList(
                        inheritedValues->begin() + begin, inheritedValues->begin() + end);
        begin = end;
    }

#ifndef _MSC_VER
    // Start a thread for each of the parallelizable traversals with its share
    // of the initial inherited attributes.
    pthread_t *threads = new pthread_t[numberOfThreads];
    for (i = 0; i < numberOfThreads; i++)
    {
        pthread_create(&threads[i], NULL,
                parallelTopDownBottomUpProcessingThread<I, S>,
                new AstSharedMemoryParallelTopDownBottomUpThreadArgs<I, S>(
                    parallelTraversals[i], basenode, parallelInheritedValues[i]));
    }

    // Main "event loop" for the "master" thread: Simply wait for the
    // condition that is signalled when a thread is completely done with its
    // traversal. The counter tells us when we are finished.
    pthread_mutex_lock(syncInfo.mutex);
    while (*syncInfo.finishedThreads < numberOfThreads)
        pthread_cond_wait(syncInfo.threadFinishedEvent, syncInfo.mutex);
    pthread_mutex_unlock(syncInfo.mutex);

    // Grab the results from each traversal.
    std::vector<SynthesizedAttributeTypeList *> finalResults(numberOfThreads);
    for (i = 0; i < numberOfThreads; i++)
        pthread_join(threads[i], (void **) &finalResults[i]);
    delete threads;
#endif

    // Flatten the nested list of traversal results.
    SynthesizedAttributeTypeList *flatFinalResults = new SynthesizedAttributeTypeList;
    for (i = 0; i < numberOfThreads; i++)
        std::copy(finalResults[i]->begin(), finalResults[i]->end(), std::back_inserter(*flatFinalResults));

    // Done! Return the final results.
    return flatFinalResults;
}

template <class I, class S>
AstSharedMemoryParallelTopDownBottomUpProcessing<I, S>::AstSharedMemoryParallelTopDownBottomUpProcessing()
  : numberOfThreads(2), synchronizationWindowSize(100000)
{
}

template <class I, class S>
AstSharedMemoryParallelTopDownBottomUpProcessing<I, S>::
AstSharedMemoryParallelTopDownBottomUpProcessing(const AstSharedMemoryParallelTopDownBottomUpProcessing<I,S>::TraversalPtrList &traversals)
  : AstCombinedTopDownBottomUpProcessing<I, S>(traversals), numberOfThreads(2), synchronizationWindowSize(100000)
{
}

template <class I, class S>
void
AstSharedMemoryParallelTopDownBottomUpProcessing<I, S>::set_numberOfThreads(size_t threads) const
{
    numberOfThreads = threads;
}

template <class I, class S>
void
AstSharedMemoryParallelTopDownBottomUpProcessing<I, S>::set_synchronizationWindowSize(size_t windowSize) const
{
    synchronizationWindowSize = windowSize;
}

// parallel TOP DOWN implementation

template <class I>
AstSharedMemoryParallelizableTopDownProcessing<I>::
AstSharedMemoryParallelizableTopDownProcessing(
        const AstSharedMemoryParallelProcessingSynchronizationInfo &syncInfo,
        const typename AstSharedMemoryParallelizableTopDownProcessing<I>::TraversalPtrList &t)
    : AstCombinedTopDownProcessing<I>(t),
      AstSharedMemoryParallelProcessingSynchronizationBase(syncInfo),
      visitedNodes(0), runningParallelTraversal(false),
      synchronizationWindowSize(syncInfo.synchronizationWindowSize)
{
}

template <class I>
void 
AstSharedMemoryParallelizableTopDownProcessing<I>::set_runningParallelTraversal(bool val)
{
    runningParallelTraversal = val;
}

template <class I>
typename AstSharedMemoryParallelizableTopDownProcessing<I>::InheritedAttributeTypeList *
AstSharedMemoryParallelizableTopDownProcessing<I>::
evaluateInheritedAttribute(SgNode *astNode,
        typename AstSharedMemoryParallelizableTopDownProcessing<I>::InheritedAttributeTypeList *inheritedValues)
{
    if (runningParallelTraversal && ++visitedNodes > synchronizationWindowSize)
    {
        synchronize();
        visitedNodes = 0;
    }

    // let the superclass handle actual attribute evaluation
    return Superclass::evaluateInheritedAttribute(astNode, inheritedValues);
}

template <class I>
void AstSharedMemoryParallelizableTopDownProcessing<I>::
atTraversalEnd()
{
    // delegate the call to the superclass
    Superclass::atTraversalEnd();

    // signal that we are done
    if (runningParallelTraversal)
    {
        signalFinish();
        // clear the flag so subsequent traversals can be non-parallel
        runningParallelTraversal = false;
    }
}

// This class holds the arguments to a traversal thread: The parallelizable
// traversal class that contains a number of traversals, the node to start the
// traversal from, and the list of initial inherited values.
template <class I>
struct AstSharedMemoryParallelTopDownThreadArgs
{
    AstSharedMemoryParallelizableTopDownProcessing<I> *traversal;
    SgNode *basenode;
    typename AstSharedMemoryParallelizableTopDownProcessing<I>::InheritedAttributeTypeList *inheritedValues;

    AstSharedMemoryParallelTopDownThreadArgs(
            AstSharedMemoryParallelizableTopDownProcessing<I> *traversal,
            SgNode *basenode,
            typename AstSharedMemoryParallelizableTopDownProcessing<I>::InheritedAttributeTypeList *inheritedValues)
        : traversal(traversal), basenode(basenode), inheritedValues(inheritedValues)
    {
    }
};

// This is the function that is executed in each thread. It basically unpacks
// its arguments and starts the traversal on them; the traversal's final
// result is returned. Synchronization is built into the parallelizable
// processing classes.
template <class I>
void *parallelTopDownProcessingThread(void *p)
{
    AstSharedMemoryParallelTopDownThreadArgs<I> *threadArgs = (AstSharedMemoryParallelTopDownThreadArgs<I> *) p;

    AstSharedMemoryParallelizableTopDownProcessing<I> *traversal = threadArgs->traversal;
    SgNode *basenode = threadArgs->basenode;
    typename AstSharedMemoryParallelizableTopDownProcessing<I>::InheritedAttributeTypeList
        *inheritedValues = threadArgs->inheritedValues;
    delete threadArgs;

    // Set the flag that indicates that this is indeed a parallel traversal;
    // it is cleared by the traversal class itself when it is done.
    traversal->set_runningParallelTraversal(true);
    // Start the traversal.
    traversal->traverse(basenode, inheritedValues);

    // Need to return something.
    return NULL;
}

template <class I>
void
AstSharedMemoryParallelTopDownProcessing<I>::traverseInParallel(
        SgNode *basenode,
        typename AstSharedMemoryParallelTopDownProcessing<I>::InheritedAttributeTypeList *inheritedValues)
{
    const typename Superclass::TraversalPtrList &traversals = Superclass::traversals;

    size_t numberOfTraversals = traversals.size();
    size_t i;

    AstSharedMemoryParallelProcessingSynchronizationInfo syncInfo(numberOfThreads, synchronizationWindowSize);

    // Chop the flat list of traversals apart and distribute them into a few
    // parallelizable traversals.
    ParallelizableTraversalPtrList parallelTraversals(numberOfThreads);
    std::vector<InheritedAttributeTypeList *> parallelInheritedValues(numberOfThreads);
    size_t begin = 0, end;
    for (i = 0; i < numberOfThreads; i++)
    {
        end = begin + numberOfTraversals / numberOfThreads;
        if (end > numberOfTraversals)
            end = numberOfTraversals;

        parallelTraversals[i]
            = new AstSharedMemoryParallelizableTopDownProcessing<I>(syncInfo,
                    std::vector<TraversalPtr>(traversals.begin() + begin, traversals.begin() + end));
        parallelInheritedValues[i]
            = new InheritedAttributeTypeList(
                        inheritedValues->begin() + begin, inheritedValues->begin() + end);
        begin = end;
    }

    // Start a thread for each of the parallelizable traversals with its share
    // of the initial inherited attributes.
    pthread_t *threads = new pthread_t[numberOfThreads];
    for (i = 0; i < numberOfThreads; i++)
    {
        pthread_create(&threads[i], NULL,
                parallelTopDownProcessingThread<I>,
                new AstSharedMemoryParallelTopDownThreadArgs<I>(
                    parallelTraversals[i], basenode, parallelInheritedValues[i]));
    }

    // Main "event loop" for the "master" thread: Simply wait for the
    // condition that is signalled when a thread is completely done with its
    // traversal. The counter tells us when we are finished.
    pthread_mutex_lock(syncInfo.mutex);
    while (*syncInfo.finishedThreads < numberOfThreads)
        pthread_cond_wait(syncInfo.threadFinishedEvent, syncInfo.mutex);
    pthread_mutex_unlock(syncInfo.mutex);

    // Grab the results from each traversal.
    std::vector<void *> finalResults(numberOfThreads);
    for (i = 0; i < numberOfThreads; i++)
        pthread_join(threads[i], &finalResults[i]);
    delete threads;

    // Done!
}

template <class I>
AstSharedMemoryParallelTopDownProcessing<I>::AstSharedMemoryParallelTopDownProcessing()
  : numberOfThreads(2), synchronizationWindowSize(100000)
{
}

template <class I>
AstSharedMemoryParallelTopDownProcessing<I>::
AstSharedMemoryParallelTopDownProcessing(const AstSharedMemoryParallelTopDownProcessing<I>::TraversalPtrList &traversals)
  : AstCombinedTopDownProcessing<I>(traversals), numberOfThreads(2), synchronizationWindowSize(100000)
{
}

template <class I>
void
AstSharedMemoryParallelTopDownProcessing<I>::set_numberOfThreads(size_t threads) const
{
    numberOfThreads = threads;
}

template <class I>
void
AstSharedMemoryParallelTopDownProcessing<I>::set_synchronizationWindowSize(size_t windowSize) const
{
    synchronizationWindowSize = windowSize;
}

// parallel BOTTOM UP implementation

template <class S>
AstSharedMemoryParallelizableBottomUpProcessing<S>::
AstSharedMemoryParallelizableBottomUpProcessing(
        const AstSharedMemoryParallelProcessingSynchronizationInfo &syncInfo,
        const typename AstSharedMemoryParallelizableBottomUpProcessing<S>::TraversalPtrList &t)
    : AstCombinedBottomUpProcessing<S>(t),
      AstSharedMemoryParallelProcessingSynchronizationBase(syncInfo),
      visitedNodes(0), runningParallelTraversal(false),
      synchronizationWindowSize(syncInfo.synchronizationWindowSize)
{
}

template <class S>
void 
AstSharedMemoryParallelizableBottomUpProcessing<S>::set_runningParallelTraversal(bool val)
{
    runningParallelTraversal = val;
}

template <class S>
typename AstSharedMemoryParallelizableBottomUpProcessing<S>::SynthesizedAttributeTypeList *
AstSharedMemoryParallelizableBottomUpProcessing<S>::
evaluateSynthesizedAttribute(SgNode *astNode,
        typename AstSharedMemoryParallelizableBottomUpProcessing<S>::SynthesizedAttributesList synthesizedAttributes)
{
    if (runningParallelTraversal && ++visitedNodes > synchronizationWindowSize)
    {
        synchronize();
        visitedNodes = 0;
    }

    // let the superclass handle actual attribute evaluation
    return Superclass::evaluateSynthesizedAttribute(astNode, synthesizedAttributes);
}

template <class S>
void AstSharedMemoryParallelizableBottomUpProcessing<S>::
atTraversalEnd()
{
    // delegate the call to the superclass
    Superclass::atTraversalEnd();

    // signal that we are done
    if (runningParallelTraversal)
    {
        signalFinish();
        // clear the flag so subsequent traversals can be non-parallel
        runningParallelTraversal = false;
    }
}

// This class holds the arguments to a traversal thread: The parallelizable
// traversal class that contains a number of traversals, and the node to start the
// traversal from.
template <class S>
struct AstSharedMemoryParallelBottomUpThreadArgs
{
    AstSharedMemoryParallelizableBottomUpProcessing<S> *traversal;
    SgNode *basenode;

    AstSharedMemoryParallelBottomUpThreadArgs(
            AstSharedMemoryParallelizableBottomUpProcessing<S> *traversal,
            SgNode *basenode)
        : traversal(traversal), basenode(basenode)
    {
    }
};

// This is the function that is executed in each thread. It basically unpacks
// its arguments and starts the traversal on them; the traversal's final
// result is returned. Synchronization is built into the parallelizable
// processing classes.
template <class S>
void *parallelBottomUpProcessingThread(void *p)
{
    AstSharedMemoryParallelBottomUpThreadArgs<S> *threadArgs = (AstSharedMemoryParallelBottomUpThreadArgs<S> *) p;

    AstSharedMemoryParallelizableBottomUpProcessing<S> *traversal = threadArgs->traversal;
    SgNode *basenode = threadArgs->basenode;
    delete threadArgs;

    // Set the flag that indicates that this is indeed a parallel traversal;
    // it is cleared by the traversal class itself when it is done.
    traversal->set_runningParallelTraversal(true);
    // Start the traversal.
    return traversal->traverse(basenode);
}

template <class S>
typename AstSharedMemoryParallelBottomUpProcessing<S>::SynthesizedAttributeTypeList *
AstSharedMemoryParallelBottomUpProcessing<S>::traverseInParallel(SgNode *basenode)
{
    const typename Superclass::TraversalPtrList &traversals = Superclass::traversals;

    size_t numberOfTraversals = traversals.size();
    size_t i;

    AstSharedMemoryParallelProcessingSynchronizationInfo syncInfo(numberOfThreads, synchronizationWindowSize);

    // Chop the flat list of traversals apart and distribute them into a few
    // parallelizable traversals.
    ParallelizableTraversalPtrList parallelTraversals(numberOfThreads);
    size_t begin = 0, end;
    for (i = 0; i < numberOfThreads; i++)
    {
        end = begin + numberOfTraversals / numberOfThreads;
        if (end > numberOfTraversals)
            end = numberOfTraversals;

        parallelTraversals[i]
            = new AstSharedMemoryParallelizableBottomUpProcessing<S>(syncInfo,
                    std::vector<TraversalPtr>(traversals.begin() + begin, traversals.begin() + end));
        begin = end;
    }

    // Start a thread for each of the parallelizable traversals.
    pthread_t *threads = new pthread_t[numberOfThreads];
    for (i = 0; i < numberOfThreads; i++)
    {
        pthread_create(&threads[i], NULL,
                parallelBottomUpProcessingThread<S>,
                new AstSharedMemoryParallelBottomUpThreadArgs<S>(
                    parallelTraversals[i], basenode));
    }

    // Main "event loop" for the "master" thread: Simply wait for the
    // condition that is signalled when a thread is completely done with its
    // traversal. The counter tells us when we are finished.
    pthread_mutex_lock(syncInfo.mutex);
    while (*syncInfo.finishedThreads < numberOfThreads)
        pthread_cond_wait(syncInfo.threadFinishedEvent, syncInfo.mutex);
    pthread_mutex_unlock(syncInfo.mutex);

    // Grab the results from each traversal.
    std::vector<SynthesizedAttributeTypeList *> finalResults(numberOfThreads);
    for (i = 0; i < numberOfThreads; i++)
        pthread_join(threads[i], (void **) &finalResults[i]);
    delete threads;

    // Flatten the nested list of traversal results.
    SynthesizedAttributeTypeList *flatFinalResults = new SynthesizedAttributeTypeList;
    for (i = 0; i < numberOfThreads; i++)
        std::copy(finalResults[i]->begin(), finalResults[i]->end(), std::back_inserter(*flatFinalResults));

    // Done! Return the final results.
    return flatFinalResults;
}

template <class S>
AstSharedMemoryParallelBottomUpProcessing<S>::AstSharedMemoryParallelBottomUpProcessing()
  : numberOfThreads(2), synchronizationWindowSize(100000)
{
}

template <class S>
AstSharedMemoryParallelBottomUpProcessing<S>::
AstSharedMemoryParallelBottomUpProcessing(const AstSharedMemoryParallelBottomUpProcessing<S>::TraversalPtrList &traversals)
  : AstCombinedBottomUpProcessing<S>(traversals), numberOfThreads(2), synchronizationWindowSize(100000)
{
}

template <class S>
void
AstSharedMemoryParallelBottomUpProcessing<S>::set_numberOfThreads(size_t threads) const
{
    numberOfThreads = threads;
}

template <class S>
void
AstSharedMemoryParallelBottomUpProcessing<S>::set_synchronizationWindowSize(size_t windowSize) const
{
    synchronizationWindowSize = windowSize;
}

#endif
