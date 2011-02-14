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

#ifndef _QDOCUMENT_LINE_P_H_
#define _QDOCUMENT_LINE_P_H_

#include "qce-config.h"

/*!
        \file qdocumentline_p.h
        \brief Definition of QDocumentLineHandle
*/

#include "qnfa.h"

#include "qformat.h"

#include "qdocumentline.h"

#include <QPair>
#include <QList>
#include <QString>
#include <QVector>

#include <QTextLayout>

#if QT_VERSION < 0x040400
#include <QAtomic>
#else
#include <QAtomicInt>
#endif

typedef QVector<int> QSmallArray;
typedef QVector<int> QMediumArray;

class QPoint;

class QDocument;
class QDocumentLine;
class QDocumentBuffer;
class QDocumentPrivate;

class QCE_EXPORT QDocumentLineHandle
{
        friend class QDocument;
        friend class QDocumentLine;
        friend class QDocumentBuffer;
        friend class QDocumentPrivate;

        public:
                QDocumentLineHandle(QDocument *d);
                QDocumentLineHandle(const QString& s, QDocument *d);
                
                int count() const;
                int length() const;
                
                int position() const;
                
                QString text() const;
                
                int line() const;
                
                int xToCursor(int x) const;
                int cursorToX(int i) const;
                
                int wrappedLineForCursor(int cpos) const;
                
                int documentOffsetToCursor(int x, int y) const;
                void cursorToDocumentOffset(int cpos, int& x, int& y) const;
                
                QPoint cursorToDocumentOffset(int cpos) const;
                
                int nextNonSpaceChar(uint pos) const;
                int previousNonSpaceChar(int pos) const;
                
                bool hasFlag(int flag) const;
                void setFlag(int flag, bool y = true) const;
                
                QDocument* document() const;
                
                QDocumentLineHandle* next() const;
                QDocumentLineHandle* previous() const;
                
                void updateWrap() const;
                
                void setFormats(const QVector<int>& formats);
                
                void clearOverlays();
                void addOverlay(const QFormatRange& over);
                void removeOverlay(const QFormatRange& over);
                
                void shiftOverlays(int position, int offset);
                
                void draw(QPainter *p,
                          int xOffset,
                          int vWidth,
                          const QSmallArray& sel,
                          const QSmallArray& cursors,
                          const QPalette& pal,
                          bool fullSel) const;
                
                inline QString& textBuffer() { setFlag(QDocumentLine::LayoutDirty, true); return m_text; }
                
                inline void ref() { m_ref.ref(); }
                inline void deref() { if ( m_ref ) m_ref.deref(); if ( !m_ref ) delete this; }
                
        protected:
                ~QDocumentLineHandle();
                
        private:
                void layout() const;
                void applyOverlays() const;
                
                QMediumArray compose() const;
                QList<QTextLayout::FormatRange> decorations() const;
                
                QString m_text;
                QDocument *m_doc;
#if QT_VERSION < 0x040400
                QBasicAtomic m_ref;
#else
                QAtomicInt m_ref;
#endif
                mutable int m_indent;
                mutable quint16 m_state;
                mutable QTextLayout *m_layout;
                mutable QVector<int> m_cache;
                mutable QVector< QPair<int, int> > m_frontiers;
                
                QNFAMatchContext m_context;
                
                QVector<int> m_formats;
                QVector<QParenthesis> m_parens;
                QList<QFormatRange> m_overlays;
};

Q_DECLARE_TYPEINFO(QDocumentLineHandle*, Q_PRIMITIVE_TYPE);

#endif // !_QDOCUMENT_LINE_P_H_
