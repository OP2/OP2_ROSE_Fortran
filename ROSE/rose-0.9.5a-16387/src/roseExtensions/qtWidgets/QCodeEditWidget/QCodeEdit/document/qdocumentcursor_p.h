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

#ifndef _QDOCUMENT_CURSOR_P_H_
#define _QDOCUMENT_CURSOR_P_H_

#include "../qce-config.h"

/*!
        \file qdocumentcursor_p.h
        \brief Definition of QDocumentCursorHandle
*/

#include "qdocumentcursor.h"

#include <QStack>

#if QT_VERSION < 0x040400
#include <QAtomic>
#else
#include <QAtomicInt>
#endif

class QPoint;
class QPolygon;

class QDocument;
class QDocumentLine;
class QDocumentPrivate;
class QDocumentCommand;
class QDocumentCommandBlock;

class QCE_EXPORT QDocumentCursorHandle
{
        friend class QDocumentCursor;
        friend class QDocumentPrivate;
        friend class QDocumentCommand;
        
        public:
                enum Flags
                {
                        Silent                          = 1,
                        ColumnMemory            = 2,
                        MoveWithinWrapped       = 4
                };

                QDocument* document() const;
                
                bool atEnd() const;
                bool atStart() const;
                
                bool atBlockEnd() const;
                bool atBlockStart() const;
                
                bool atLineEnd() const;
                bool atLineStart() const;
                
                bool hasSelection() const;
                
                bool isSilent() const;
                void setSilent(bool y);
                
                bool isAutoUpdated() const;
                void setAutoUpdated(bool y);
                
                QDocumentLine line() const;
                QDocumentLine anchorLine() const;
                
                int lineNumber() const;
                int columnNumber() const;
                
                int anchorLineNumber() const;
                int anchorColumnNumber() const;
                
                int visualColumnNumber() const;
                
                void setColumnNumber(int c, int m = QDocumentCursor::MoveAnchor);
                
                QPoint documentPosition() const;
                QPoint anchorDocumentPosition() const;
                
                QPolygon documentRegion() const;
                
                int position() const;
                
                void shift(int offset);
                void setPosition(int pos, int m);
                bool movePosition(int offset, int op, int m);
                
                void insertText(const QString& s);
                
                QChar nextChar() const;
                QChar previousChar() const;
                
                void eraseLine();
                void deleteChar();
                void deletePreviousChar();
                
                QDocumentCursor selectionStart() const;
                QDocumentCursor selectionEnd() const;
                
                bool eq(const QDocumentCursorHandle *h);
                bool lt(const QDocumentCursorHandle *h);
                bool gt(const QDocumentCursorHandle *h);
                
                QString selectedText() const;
                
                void clearSelection();
                void removeSelectedText();
                
                void select(QDocumentCursor::SelectionType t);
                void setSelectionBoundary(const QDocumentCursor& c);
                
                bool isWithinSelection(const QDocumentCursor& c) const;
                QDocumentCursor intersect(const QDocumentCursor& c) const;
                
                void substractBoundaries(int lbeg, int cbeg, int lend, int cend);
                void boundaries(int& begline, int& begcol, int& endline, int& endcol) const;
                void intersectBoundaries(int& lbeg, int& cbeg, int& lend, int& cend) const;
                void intersectBoundaries(QDocumentCursorHandle *h, int& lbeg, int& cbeg, int& lend, int& cend) const;
                
                void beginEditBlock();
                void endEditBlock();
                
                void moveTo(int line, int column);
                void moveTo(const QDocumentCursor &c);
                
                void copy(const QDocumentCursorHandle *c);
                
                void refreshColumnMemory();
                bool hasColumnMemory() const;
                void setColumnMemory(bool y);
                
                virtual void execute(QDocumentCommand *c);
                
                inline void ref() { m_ref.ref(); }
                inline void deref() { if ( m_ref ) m_ref.deref(); if ( !m_ref ) delete this; }
                
                inline bool hasFlag(int f) const { return m_flags & f; }
                inline void setFlag(int f) { m_flags |= f; }
                inline void clearFlag(int f) { m_flags &= ~f; }
                
        protected:
                QDocumentCursorHandle(QDocument *d, int line = 0);
                virtual ~QDocumentCursorHandle();
                
                QDocumentCursorHandle* clone() const;
                
        private:
                int m_flags;
                QDocument *m_doc;
#if QT_VERSION < 0x040400
                QBasicAtomic m_ref;
#else
                QAtomicInt m_ref;
#endif
                int m_begOffset, m_endOffset, m_max, m_begLine, m_endLine;
                QStack<QDocumentCommandBlock*> m_blocks;
};

Q_DECLARE_TYPEINFO(QDocumentCursorHandle*, Q_PRIMITIVE_TYPE);

#endif // !_QDOCUMENT_CURSOR_P_H_
