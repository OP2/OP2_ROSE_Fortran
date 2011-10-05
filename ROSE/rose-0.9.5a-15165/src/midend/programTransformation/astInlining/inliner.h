#ifndef INLINER_H
#define INLINER_H

// DQ (3/12/2006): This is now not required, the file name is 
// changed to rose_config.h and is included directly by rose.h.
// #include "config.h"


#include "rewrite.h"
#include "replaceExpressionWithStatement.h"
#include "inlinerSupport.h"

//! Main inliner code.  Accepts a function call as a parameter, and inlines
//! only that single function call.  Returns true if it succeeded, and false
//! otherwise.  The function call must be to a named function, static member
//! function, or non-virtual non-static member function, and the function
//! must be known (not through a function pointer or member function
//! pointer).  Also, the body of the function must already be visible.
//! Recursive procedures are handled properly (when allowRecursion is set), by
//! inlining one copy of the procedure into itself.  Any other restrictions on
//! what can be inlined are bugs in the inliner code.
bool doInline(SgFunctionCallExp* funcall, bool allowRecursion = false);

#endif // INLINER_H
