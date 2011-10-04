/*!
 *  \file ASTtools/Copy.hh
 *
 *  \brief Implements a wrapper routine for making a deep-copy of a
 *  node.
 *
 *  \author Richard Vuduc <richie@llnl.gov>
 */

// tps (01/14/2010) : Switching from rose.h to sage3.
#include "sage3basic.h"
#include "Copy.hh"

// ========================================================================

using namespace std;

// ========================================================================

static SgTreeCopy g_treeCopy;

SgNode *
ASTtools::deepCopy (const SgNode* n)
{
#if 1  
  return n ? n->copy (g_treeCopy) : 0;
#else  
  return SageInterface::deepCopyNode(n);  
  // has some problem in handling test2005_27.C
#endif  
}

// eof
