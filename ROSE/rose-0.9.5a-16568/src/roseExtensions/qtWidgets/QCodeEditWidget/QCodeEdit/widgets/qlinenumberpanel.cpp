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

#include "qlinenumberpanel.h"

/*!
        \file qlinenumberpanel.cpp
        \brief Implementation of the QLineNumberPanel class

        \see QLineNumberPanel
*/

#include <QPainter>
#include <QScrollBar>

#include "qeditor.h"
#include "qdocument.h"
#include "qdocument_p.h"
#include "qdocumentline.h"


/*!
        \ingroup widgets
        @{
*/

/*!
        \class QLineNumberPanel
        \brief A specific panel in charge of drawing line numbers of an editor
*/

QCE_AUTO_REGISTER(QLineNumberPanel)

/*!
        \brief Constructor
*/
QLineNumberPanel::QLineNumberPanel(QWidget *p)
 : QPanel(p), m_verbose(true)
{
        setFixedWidth(20);
}

/*!
        \brief Empty destructor
*/
QLineNumberPanel::~QLineNumberPanel()
{

}

/*!

*/
QString QLineNumberPanel::type() const
{
        return "Line numbers";
}

/*!

*/
bool QLineNumberPanel::isVerboseMode() const
{
        return m_verbose;
}

/*!

*/
void QLineNumberPanel::setVerboseMode(bool y)
{
        m_verbose = y;
        update();
}

/*!

*/
void QLineNumberPanel::editorChange(QEditor *e)
{
        if ( editor() )
        {
                disconnect(     editor(), SIGNAL( cursorPositionChanged() ),
                                        this    , SLOT  ( update() ) );

        }

        if ( e )
        {
                setFixedWidth(fontMetrics().width(QString::number(e->document()->lines())) + 5);

                connect(e       , SIGNAL( cursorPositionChanged() ),
                                this, SLOT  ( update() ) );

        }
}

/*!

*/
void QLineNumberPanel::paint(QPainter *p, QEditor *e)
{
        /*
                possible Unicode caracter for wrapping arrow :
                        0x21B3
                        0x2937
        */

        QFont f(font());
        f.setWeight(QFont::Bold);
        const QFontMetrics sfm(f);

        #ifndef WIN32
        static const QChar wrappingArrow(0x2937);
        const QFontMetrics specialSfm(sfm);
        #else
        static const QChar wrappingArrow('�');
        QFont specialFont(font());
        specialFont.setRawName("Wingdings");
        const QFontMetrics specialSfm(specialFont);
        #endif

        const int max = e->document()->lines();
        const int panelWidth = sfm.width(QString::number(max)) + 5;
        setFixedWidth(panelWidth);

        const QFontMetrics fm( e->document()->font() );

        int n, posY,
                as = fm.ascent(),
                ls = fm.lineSpacing(),
                pageBottom = e->viewport()->height(),
                contentsY = e->verticalOffset();

        QString txt;
        QDocument *d = e->document();
        const int cursorLine = e->cursor().lineNumber();

        n = d->lineNumber(contentsY);
        posY = as + 2 + d->y(n) - contentsY;

        //qDebug("first = %i; last = %i", first, last);
        //qDebug("beg pos : %i", posY);

        for ( ; ; ++n )
        {
                //qDebug("n = %i; pos = %i", n, posY);
                QDocumentLine line = d->line(n);

                if ( line.isNull() || ((posY - as) > pageBottom) )
                        break;

                if ( line.isHidden() )
                        continue;

                bool draw = true;

                if ( !m_verbose )
                {
                        draw = !((n + 1) % 10) || !n || line.marks().count();
                }

                txt = QString::number(n + 1);

                if ( n == cursorLine )
                {
                        draw = true;

                        p->save();
                        QFont f = p->font();
                        f.setWeight(QFont::Bold);

                        p->setFont(f);
                }

                if ( draw )
                {
                        p->drawText(width() - 2 - sfm.width(txt),
                                                posY,
                                                txt);

                } else {
                        int yOff = posY - (as + 1) + ls / 2;

                        if ( (n + 1) % 5 )
                                p->drawPoint(width() - 5, yOff);
                        else
                                p->drawLine(width() - 7, yOff, width() - 2, yOff);
                }

                if ( line.lineSpan() > 1 )
                {
                        #ifdef Q_OS_WIN32
                        p->save();
                        specialFont.setBold(n == cursorLine); //todo: only get bold on the current wrapped line
                        p->setFont(specialFont);
                        #endif

                        for ( int i = 1; i < line.lineSpan(); ++i )
                        {
                                // draw line wrapping indicators
                                //p->drawText(width() - 2 - sfm.width(wrappingArrow), posY + i * ls, wrappingArrow);
                                p->drawText(width() - 1 - specialSfm.width(wrappingArrow), posY + i * ls, wrappingArrow);
                        }

                        #ifdef Q_OS_WIN32
                        p->restore();
                        #endif
                }

                if ( n == cursorLine )
                {
                        p->restore();
                }

                posY += ls * line.lineSpan();
        }

        //p->setPen(Qt::DotLine);
        //p->drawLine(width()-1, 0, width()-1, pageBottom);

        //setFixedWidth(sfm.width(txt) + 5);
}

/*! @} */

