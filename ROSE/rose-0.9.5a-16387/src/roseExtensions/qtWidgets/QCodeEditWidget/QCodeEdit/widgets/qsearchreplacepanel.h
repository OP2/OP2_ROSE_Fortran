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

#ifndef _QSEARCH_REPLACE_PANEL_H_
#define _QSEARCH_REPLACE_PANEL_H_

#include "qpanel.h"

/*!
        \file qsearchreplacepanel.h
        \brief Definition of the QSearchReplacePanel class.
        
        \see QSearchReplacePanel
*/

#include "ui_searchreplace.h"

class QDocumentLine;
class QDocumentSearch;

class QCE_EXPORT QSearchReplacePanel : public QPanel, private Ui::SearchReplace
{
        Q_OBJECT
        
        public:
                Q_PANEL(QSearchReplacePanel, "Search Replace Panel")
                
                QSearchReplacePanel(QWidget *p = 0);
                virtual ~QSearchReplacePanel();
                
                virtual QString type() const;
                
        public slots:
                void display(int mode, bool replace);
                
                void find(int backward = -1);
                
        protected:
                virtual void editorChange(QEditor *e);
                
                virtual bool eventFilter(QObject *o, QEvent *e);
                
                virtual void hideEvent(QHideEvent *e);
                virtual void paintEvent(QPaintEvent *e);
                
        private slots:
                void on_leFind_textEdited(const QString& text);
                void on_leReplace_textEdited(const QString& text);
                
                void on_cbReplace_toggled(bool on);
                
                void on_cbCase_toggled(bool on);
                void on_cbWords_toggled(bool on);
                void on_cbRegExp_toggled(bool on);
                void on_cbCursor_toggled(bool on);
                void on_cbHighlight_toggled(bool on);
                void on_cbSelection_toggled(bool on);
                void on_cbPrompt_toggled(bool on);
                void on_cbEscapeSeq_toggled(bool on);
                
                void on_bRefresh_clicked();
                
                void on_bNext_clicked();
                void on_bPrevious_clicked();
                
                void cursorPositionChanged();
                
        private:
                void init();
                void on_leFind_returnPressed(bool backward);
                
                int lastDirection;
                QDocumentSearch *m_search;
};

#endif // _QSEARCH_REPLACE_PANEL_H_
