/****************************************************************************
**
** Copyright (C) 2006-2009 fullmetalcoder <fullmetalcoder@hotmail.fr>
**
** This file is part of the Edyuk project <http://edyuk.org>
**
** This file may be used under the terms of the GNU General Public License
** version 3 as published by the Free Software Foundation and appearing in the
** file GPL.txt included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef _QLINE_MARK_PANEL_H_
#define _QLINE_MARK_PANEL_H_

/*!
        \file qlinemarkpanel.h
        \brief Definition of the QLineMarkPanel class.

        \see QLineMarkPanel
*/

#include "qpanel.h"

#include <QHash>
#include <QIcon>

class QDocumentLine;


class QCE_EXPORT QLineMarkPanel : public QPanel
{
        Q_OBJECT

        public:
                Q_PANEL(QLineMarkPanel, "Line Mark Panel")

                QLineMarkPanel(QWidget *p = 0);
                virtual ~QLineMarkPanel();

                virtual QString type() const;

                void setDisableClicks(bool val) { disableClicks=val; }
                bool clicksDisabled() const     { return disableClicks; }

        protected:
                virtual void paint(QPainter *p, QEditor *e);
                virtual void mousePressEvent(QMouseEvent *e);
                virtual void mouseReleaseEvent(QMouseEvent *e);
                virtual void contextMenuEvent(QContextMenuEvent *e);

        private:
                QList<QRect> m_rects;
                QList<int> m_lines;

                bool disableClicks;
};

#endif // _QLINE_MARK_PANEL_H_

