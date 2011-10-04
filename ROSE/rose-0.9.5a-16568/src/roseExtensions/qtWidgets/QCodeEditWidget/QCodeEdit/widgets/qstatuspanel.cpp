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

#include "qstatuspanel.h"

/*!
        \file qstatuspanel.cpp
        \brief Implementation of the QStatusPanel class.
        
        \see QStatusPanel
*/

#include "qeditor.h"

#include "qdocument.h"
#include "qdocumentline.h"
#include "qdocumentcursor.h"

#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>
#include <QApplication>

/*!
        \ingroup widgets
        @{
*/

/*!
        \class QStatusPanel
        \brief A panel that display some status informations 
*/

QCE_AUTO_REGISTER(QStatusPanel)

/*!
        \brief Constructor
*/
QStatusPanel::QStatusPanel(QWidget *p)
 : QPanel(p)
{
        setFixedHeight(fontMetrics().lineSpacing() + 4);
}

/*!
        \brief Empty destructor
*/
QStatusPanel::~QStatusPanel()
{
        
}

/*!

*/
QString QStatusPanel::type() const
{
        return "Status";
}

/*!

*/
void QStatusPanel::editorChange(QEditor *e)
{
        if ( editor() )
        {
                disconnect(     editor(), SIGNAL( cursorPositionChanged() ),
                                        this    , SLOT  ( update() ) );
                
        }
        
        if ( e )
        {
                connect(e       , SIGNAL( cursorPositionChanged() ),
                                this, SLOT  ( update() ) );
                
        }
}

/*!

*/
void QStatusPanel::paint(QPainter *p, QEditor *e)
{
        //qDebug("drawing status panel... [%i, %i, %i, %i]",
        //              geometry().x(),
        //              geometry().y(),
        //              geometry().width(),
        //              geometry().height());
        static QPixmap _warn(":/warning.png");
        
        QString s;
        int xpos = 10;
        QDocumentCursor c = e->cursor();
        const QFontMetrics fm(fontMetrics());
        
        const int ls = fm.lineSpacing();
        const int ascent = fm.ascent() + 3;
        
        s = tr("Line : %1 Visual column : %2 Text column : %3")
                        .arg(c.lineNumber() + 1)
                        .arg(c.visualColumnNumber())
                        .arg(c.columnNumber());
        
        p->drawText(xpos, ascent, s);
        xpos += fm.width(s) + 10;
        
        // TODO : draw icon to show mod status
        
        s = editor()->flag(QEditor::Overwrite) ? tr("OVERWRITE") : tr("INSERT");
        p->drawText(xpos, ascent, s);
        xpos += fm.width(s) + 10;
        
        
        m_conflictSpot = 0;
        
        if ( editor()->isInConflict() )
        {
                s =  tr("Conflict");
                int w = fm.width(s) + 30;
                
                if ( xpos + w + _warn.width() < width() )
                {
                        m_conflictSpot = width() - (w + _warn.width());
                        p->drawText(width() - w + 15, ascent, s);
                        p->drawPixmap(m_conflictSpot, (ls - _warn.height()) / 2 + 2, _warn);
                } else if ( xpos + _warn.width() < width() ) {
                        m_conflictSpot = width() - _warn.width();
                        p->drawPixmap(m_conflictSpot, (ls - _warn.height()) / 2 + 2, _warn);
                }
        }
        
        setFixedHeight(ls + 4);
}

/*!

*/
void QStatusPanel::mousePressEvent(QMouseEvent *e)
{
        if ( !editor() || (e->button() != Qt::LeftButton) || !m_conflictSpot || e->x() < m_conflictSpot )
        {
                editor()->setFocus();
                return;
        }
        
        editor()->save();
}

/*!

*/
void QStatusPanel::mouseReleaseEvent(QMouseEvent *e)
{
        editor()->setFocus();
}

/*! @} */
