#ifndef BASTMODEL_H
#define BASTMODEL_H

#include "ItemTreeModel.h"

class SgNode;
class AstFilterInterface;

/**
 * \brief The Qt-Model representing a beautified AST
 */
class BAstModel : public ItemTreeModel
{
    Q_OBJECT

    public:
        BAstModel(QObject * parent);
        virtual ~BAstModel();

        void setNode(SgNode * proj);
        void setFilter(AstFilterInterface * filter);

        SgNode * getNodeFromIndex(const QModelIndex & ind);

    protected:
        virtual QVariant data (const QModelIndex & ind, int role) const;

        void updateModel();

        AstFilterInterface * curFilter;
        SgNode * curNode;

};

#endif
