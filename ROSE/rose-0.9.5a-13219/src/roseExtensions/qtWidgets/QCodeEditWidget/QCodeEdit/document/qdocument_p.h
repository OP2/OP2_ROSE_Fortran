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

#ifndef _QDOCUMENT_P_H_
#define _QDOCUMENT_P_H_

#include "qce-config.h"

/*!
	\file qdocument_p.h
	\brief Definition of the private document API
*/

#include "qdocument.h"
#include "qdocumentline.h"
#include "qdocumentcursor.h"

#include <QHash>
#include <QFont>
#include <QStack>
#include <QQueue>
#include <QUndoStack>
#include <QStringList>
#include <QFontMetrics>
#include <QUndoCommand>

class QDocument;
class QDocumentBuffer;
class QDocumentPrivate;
class QDocumentCommand;
class QDocumentCommandBlock;

class QLanguageDefinition;

Q_DECLARE_TYPEINFO(QDocumentSelection, Q_PRIMITIVE_TYPE);

#include "qdocumentline_p.h"

#include "qdocumentcursor_p.h"

class QCE_EXPORT QDocumentPrivate
{
	friend class QEditConfig;
	
	friend class QDocument;
	friend class QDocumentCommand;
	friend class QDocumentLineHandle;
	friend class QDocumentCursorHandle;
	
	public:
		QDocumentPrivate(QDocument *d);
		~QDocumentPrivate();
		
		void execute(QDocumentCommand *cmd);
		
		void draw(QPainter *p, QDocument::PaintContext& cxt);
		
		QDocumentLineHandle* lineForPosition(int& position) const;
		int position(const QDocumentLineHandle *l) const;
		
		QDocumentLineHandle* at(int line) const;
		int indexOf(const QDocumentLineHandle *l) const;
		
		QDocumentIterator index(const QDocumentLineHandle *l);
		QDocumentConstIterator index(const QDocumentLineHandle *l) const;
		
		QDocumentLineHandle* next(const QDocumentLineHandle *l) const;
		QDocumentLineHandle* previous(const QDocumentLineHandle *l) const;
		
		void adjustWidth(int l);
		//int checkWidth(QDocumentLineHandle *l, int w);
		//int checkWidth(QDocumentLineHandle *l, const QString& s);
		
		void setWidth();
		void setHeight();
		
		static void setFont(const QFont& f);
		
		void beginChangeBlock();
		void endChangeBlock();
		
		inline int maxMarksPerLine() const
		{ return m_maxMarksPerLine; }
		
		inline bool hasMarks() const
		{ return m_marks.count(); }
		
		QList<int> marks(QDocumentLineHandle *h) const;
		
		void addMark(QDocumentLineHandle *h, int mid);
		void toggleMark(QDocumentLineHandle *h, int mid);
		void removeMark(QDocumentLineHandle *h, int mid);
		
		int findNextMark(int id, int from = 0, int until = -1);
		int findPreviousMark(int id, int from = -1, int until = 0);
		
		int getNextGroupId();
		void releaseGroupId(int groupId);
		void clearMatches(int gid);
		void flushMatches(int gid);
		void addMatch(int gid, int line, int pos, int len, int format);
		
		void emitFormatsChange (int line, int lines);
		void emitContentsChange(int line, int lines);
		
		int visualLine(int textLine) const;
		int textLine(int visualLine, int *wrap = 0) const;
		void hideEvent(int line, int count);
		void showEvent(int line, int count);
		
		void setWidth(int width);
		
		void emitFormatsChanged();
		void emitContentsChanged();
		
		void emitLineDeleted(QDocumentLineHandle *h);
		void emitMarkChanged(QDocumentLineHandle *l, int m, bool on);
		
		inline QDocumentIterator begin() { return m_lines.begin(); }
		inline QDocumentIterator end() { return m_lines.end(); }
		
		inline QDocumentConstIterator constBegin() const { return m_lines.constBegin(); }
		inline QDocumentConstIterator constEnd() const { return m_lines.constEnd(); }
		
	protected:
		void updateHidden(int line, int count);
		void updateWrapped(int line, int count);
		
		void insertLines(int after, const QList<QDocumentLineHandle*>& l);
		void removeLines(int after, int n);
		
		void emitWidthChanged();
		void emitHeightChanged();
		
		void updateFormatCache();
		void setFormatScheme(QFormatScheme *f);
		void tunePainter(QPainter *p, int fid);
		
	private:
		QDocument *m_doc;
		QUndoStack m_commands;
		QDocumentCursor *m_editCursor;
		
		bool m_suspend, m_deleting;
		QQueue<QPair<int, int> > m_notifications;
		
		QMap<int, int> m_hidden;
		QMap<int, int> m_wrapped;
		QVector< QPair<QDocumentLineHandle*, int> > m_largest;
		
		struct Match
		{
			int line;
			QFormatRange range;
			QDocumentLineHandle *h;
		};
		
		struct MatchList : QList<Match>
		{
			MatchList() : index(0) {}
			
			int index;
		};
		
		int m_lastGroupId;
		QList<int> m_freeGroupIds;
		QHash<int, MatchList> m_matches;
		
		bool m_constrained;
		int m_width, m_height;
		
		int m_tabStop;
		static int m_defaultTabStop;
		
		static QFont *m_font;
		static bool m_fixedPitch;
		static QFontMetrics *m_fontMetrics;
		static int m_leftMargin;
		static QDocument::WhiteSpaceMode m_showSpaces;
		static QDocument::LineEnding m_defaultLineEnding;
		static int m_lineHeight;
		static int m_lineSpacing;
		static int m_spaceWidth;
		static int m_ascent;
		static int m_descent;
		static int m_leading;
		static int m_wrapMargin;
		
		QFormatScheme *m_formatScheme;
		QLanguageDefinition *m_language;
		static QFormatScheme *m_defaultFormatScheme;
		
		QVector<QFont> m_fonts;
		
		static QList<QDocumentPrivate*> m_documents;
		
		int m_maxMarksPerLine;
		QHash<QDocumentLineHandle*, QList<int> > m_marks;
		QHash<QDocumentLineHandle*, QPair<int, int> > m_status;
		
		int _nix, _dos;
		QString m_lineEndingString;
		QDocument::LineEnding m_lineEnding;
		
		QDocumentBuffer *m_buffer;
		QVector<QDocumentLineHandle*> m_lines;
};

#endif
