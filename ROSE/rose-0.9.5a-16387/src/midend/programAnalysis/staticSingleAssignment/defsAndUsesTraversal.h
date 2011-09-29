#pragma once
#include "rose.h"
#include "staticSingleAssignment.h"

namespace ssa_private
{

    /** Attribute that describes the variables used by a given expression. */
    class ChildUses
    {
    private:
        /** An assignment to the current expression in the AST would define this variable */
        SgVarRefExp* currentVar;

        /** Stores all the varRefs that are used in the current subTree. */
        std::vector<SgNode*> uses;

    public:

        /** Create the attribute with no refs.   */
        ChildUses() : currentVar(NULL)
        {
        }

        ChildUses(SgNode* useNode, SgVarRefExp* var)
        {
            uses.push_back(useNode);
            currentVar = var;
        }

        /** Create the attribute with the def and list of uses.
         *
         * @param useTree The vector of uses to add, or an empty vector.
         */
        ChildUses(const std::vector<SgNode*>& useTree, SgVarRefExp* var = NULL)
        {
            if (useTree.size() > 0)
                uses.assign(useTree.begin(), useTree.end());
            currentVar = var;
        }

        /** Get the uses for this node and below.
         *
         * @return A constant reference to the use list.
         */
        std::vector<SgNode*>& getUses()
        {
            return uses;
        }

        /** Set the uses for this node and below.
         *
         * @param newUses A constant reference to the uses to copy to this node.
         */
        void setUses(const std::vector<SgNode*>& newUses)
        {
            uses.assign(newUses.begin(), newUses.end());
        }

        SgVarRefExp* getCurrentVar() const
        {
            return currentVar;
        }
    };

    /** This class collects all the defs and uses associated with each node in the traversed CFG.
     * Note that this does not compute reachability information; it just records each instance of
     * a variable used or defined. */
    class DefsAndUsesTraversal : public AstBottomUpProcessing<ChildUses>
    {
        StaticSingleAssignment* ssa;

        //! If true, modifications to a value pointed to by a pointer will count as defs for the pointer itself.
        //! For example, (delete p) would be considered to modify p.
        const bool treatPointersAsStructs;
        
    public:

        //! @param treatPointersAsStructs If true, modifications to a value pointed to by a pointer will 
        //!     count as defs for the pointer itself. For example, (delete p) would be considered to modify p.
        DefsAndUsesTraversal(StaticSingleAssignment* ssa, bool treatPointersAsStructs = true) : ssa(ssa),
                treatPointersAsStructs(treatPointersAsStructs)
        {
        }

        /** Called to evaluate the synthesized attribute on every node.
         *
         * This function will handle passing all variables that are defined and used by a given operation.
         *
         * @param node The node being evaluated.
         * @param attr The attributes from the child nodes.
         * @return The attribute at this node.
         */
        virtual ChildUses evaluateSynthesizedAttribute(SgNode* node, SynthesizedAttributesList attrs);

    private:

        /** Mark all the uses as occurring at the specified node. */
        void addUsesToNode(SgNode* node, std::vector<SgNode*> uses);

        /** Mark the given variable as being defined at the node. */
        void addDefForVarAtNode(SgVarRefExp* currentVar, SgNode* defNode);
    };

} //namespace ssa_private