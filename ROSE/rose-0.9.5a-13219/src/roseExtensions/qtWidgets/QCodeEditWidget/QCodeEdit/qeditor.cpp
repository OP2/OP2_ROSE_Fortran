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

#include "qeditor.h"

/*!
	\file qeditor.cpp
	\brief Implementation of the QEditor class
*/

#include "qdocument.h"
#include "qdocument_p.h"
#include "qdocumentline.h"
#include "qdocumentcursor.h"

#include "qlanguagedefinition.h"
#include "qcodecompletionengine.h"

#include "qcodeedit.h"
#include "qpanellayout.h"
#include "qgotolinedialog.h"
#include "qlinemarksinfocenter.h"

#include "qreliablefilewatch.h"

#include <QFile>
#include <QMenu>
#include <QTimer>
#include <QStyle>
#include <QDebug>
#include <QAction>
#include <QPointer>
#include <QPainter>
#include <QPrinter>
#include <QKeyEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QClipboard>
#include <QScrollBar>
#include <QTextCodec>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QPrintDialog>
#include <QApplication>
#include <QActionGroup>

//#define Q_GL_EDITOR

#ifdef _QMDI_
#include "qmdiserver.h"
#endif

#ifdef _EDYUK_
#include "edyukapplication.h"
#include "qshortcutmanager.h"

#define Q_SHORTCUT(a, s, c) EDYUK_SHORTCUT(a, tr(c), tr(s))
#else
#define Q_SHORTCUT(a, s, c) a->setShortcut( QKeySequence( tr(s, c) ) )
#endif

#ifdef Q_GL_EDITOR
#include <QGLWidget>
#endif

#define QCE_ACTION(name, action) { QAction *_a_ = m_actions.value(name); if ( _a_ ) _a_ action; }
#define QCE_TR_ACTION(name, label) { QAction *_a_ = m_actions.value(name); if ( _a_ ) _a_->setText(label); }
#define QCE_ENABLE_ACTION(name, yes) { QAction *_a_ = m_actions.value(name); if ( _a_ ) _a_->setEnabled(yes); }

/*!
	\ingroup editor
	@{
	
	\class QEditor
	\brief A text editing widget
	
	QEditor is the central widget in QCE. It allows user to view and edit a
	document.
	
	QEditor has an API similar to that of QTextEdit and it behaves in a very
	similar way.
	
	Notable differences are :
	<ul>
	<li>QEditor can be given an InputBinding which can change the way it
	handle user inputs which enables such things as implementing emacs-like
	or Vi-like editing (almost) effortlessly.</li>
	<li>QEditor has actions instead of hard coded shortcuts and expose them
	so that, among other things, they can be easily added to menus/toolbars
	and their shortcuts can be changed</li>
	<li>QEditor brings the notion of cursor mirrors. Column selection and
	column editing are just special use case of cursor mirrors.</li>
	<li>QEditor brings the notion of placeholders, snippets-editing is just
	as special use case of placeholders.</li>
	<li>QEditor allows easy encodings management</li>
	</ul>
	
	QEditor can gain features when it is managed by a QCodeEdit class which
	is responsible for panels management.
*/


/*!
	\enum QEditor::CodecUpdatePolicy
	\brief Specify the actions to take when changing the default codec
	
*/


/*!
	\enum QEditor::EditFlag
	\brief Flag holding information about the state of an editor
	
	Some of these are public and can be modified freely and some
	others are only meant to be used internally though they can
	still be read.
	
*/


/*!
	\class QEditor::InputBinding
	\brief A class designed to allow extending user input in a transparent way
	
	An input binding, when set to an editor, can intercept all the events the
	editor receive and radically change the behavior.
	
	The main purpose of this class is twofold :
	<ul>
	<li>Allow vi-like (or emacs-like, ...) editing to be implemented with little extra work.
	And allow the user to easily switch between input modes</li>
	<li>Allow applications using QCE to easily add extra features (e.g extended code
	navigation within projects, jump to documentation, ...) with little extra work</li>
	</ul>
*/

/*!
	\struct QEditor::PlaceHolder
	\brief A small structure holding placeholder data
	
	Placeholders are basically lists of cursors. When several palceholders coexist, it is
	possible to navigate among them using the key assigned to that function by the current
	input binding (tab and SHIFT+tab by default).
	
	Each placeholder consist of a primary cursor and a list of mirrors (modeling the internals
	of QEditor and allowing extended snippet replacements easily).
*/

////////////////////////////////////////////////////////////////////////
//	Bindings handling
////////////////////////////////////////////////////////////////////////

QList<QEditor*> QEditor::m_editors;
QEditor::InputBinding* QEditor::m_defaultBinding = 0;
QHash<QString, QEditor::InputBinding*> QEditor::m_bindings;

/*!
	\return A list of available input bindings
*/
QStringList QEditor::inputBindings()
{
	return m_bindings.keys();
}

/*!
	\return the name of the default input binding
	
	\note The "Default" name (or its translation, obtained via QEditor::tr())
	is used to indicate that no default input binding has been set.
*/
QString QEditor::defaultInputBinding()
{
	return m_defaultBinding ? m_defaultBinding->name() : tr("Default");
}

/*!
	\brief Add an input binding to make it available for all editors
*/
void QEditor::addInputBinding(QEditor::InputBinding *b)
{
	m_bindings[b->id()] = b;
	
	foreach ( QEditor *e, m_editors )
		e->updateBindingsMenu();
	
}

/*!
	\brief Remove an input binding from the pool of publicly available ones
*/
void QEditor::removeInputBinding(QEditor::InputBinding *b)
{
	m_bindings.remove(b->id());
	
	foreach ( QEditor *e, m_editors )
		e->updateBindingsMenu();
	
}

/*!
	\brief Set the default input binding
	
	\note This does not change the current input binding of existing editors
*/
void QEditor::setDefaultInputBinding(QEditor::InputBinding *b)
{
	m_defaultBinding = b;
}

/*!
	\brief Set the default input binding
	
	\note If no binding of the given name is available the default (null)
	binding will be set back as default binding.
	
	\note This does not change the current input binding of existing editors
*/
void QEditor::setDefaultInputBinding(const QString& b)
{
	m_defaultBinding = m_bindings.value(b);
}

////////////////////////////////////////////////////////////////////////

/*!
	\return A pointer to the global "reliable" file monitor used by QEditor to avoid file conflicts
	
	The point of using a custom file watcher is to work around a bug (limitation) of QFileSystemWatcher
	which sometimes emit multiple signals for a single file save. It also enables to use a single
	object shared by all QEditor instances and reduce memory footprint.
*/
QReliableFileWatch* QEditor::watcher()
{
	static QPointer<QReliableFileWatch> _qce_shared;
	
	if ( !_qce_shared )
		_qce_shared = new QReliableFileWatch;
	
	return _qce_shared;
}

////////////////////////////////////////////////////////////////////////

int QEditor::m_defaultFlags = QEditor::AutoIndent;
QTextCodec* QEditor::m_defaultCodec = 0;

/*!
	\return The default flags set to every QEditor upon construction
	\note the default flags are a configuration-oriented feature which only expose "user" flags
*/
int QEditor::defaultFlags()
{
	return m_defaultFlags;
}

/*!
	\brief Set the default editor flags
	
	Setting editor flags result in them being applied to ALL existing editors
	and editors to be created later on.
	
	These can of course be modified on a per-editor basis later on.
*/
void QEditor::setDefaultFlags(int flags)
{
	m_defaultFlags = flags & Accessible;
	
	foreach ( QEditor *e, m_editors )
	{
		bool ontoWrap = (m_defaultFlags & LineWrap) && !(e->m_state & LineWrap);
		bool outOfWrap = !(m_defaultFlags & LineWrap) && (e->m_state & LineWrap);
		
		e->m_state &= Internal;
		e->m_state |= m_defaultFlags;
		
		if ( ontoWrap )
		{
			e->document()->setWidthConstraint(e->wrapWidth());
		} else if ( outOfWrap ) {
			e->document()->clearWidthConstraint();
		}
		
		QAction *a = e->m_actions.value("wrap");
		
		if ( a && (a->isChecked() != (bool)(e->m_state & LineWrap)) )
			a->setChecked(e->m_state & LineWrap);
		
	}
}

/*!
	\return The default text codec used to load and save document contents
	
	\note a null pointer indicates that local 8 bit encoding is used.
*/
QTextCodec* QEditor::defaultCodec()
{
	return m_defaultCodec;
}

/*!
	\overload
	\param mib codec identifier
	\param update Update policy
*/
void QEditor::setDefaultCodec(int mib, int update)
{
	setDefaultCodec(QTextCodec::codecForMib(mib), update);
}

/*!
	\overload
	\param name name of the codec to use
	\param update Update policy
*/
void QEditor::setDefaultCodec(const char *name, int update)
{
	setDefaultCodec(QTextCodec::codecForName(name), update);
}

/*!
	\overload
	\param name name of the codec to use
	\param update Update policy
*/
void QEditor::setDefaultCodec(const QByteArray& name, int update)
{
	setDefaultCodec(QTextCodec::codecForName(name), update);
}

/*!
	\brief Set the default text codec
	\param c codec to use
	\param update Update policy
	
	The update policy determines whether existing editors are
	affected by the change of the default codec.
*/
void QEditor::setDefaultCodec(QTextCodec *c, int update)
{
	foreach ( QEditor *e, m_editors )
	{
		if ( e->codec() == m_defaultCodec )
		{
			if ( update & UpdateOld )
				e->setCodec(c);
		} else if ( e->codec() ) {
			if ( update & UpdateCustom )
				e->setCodec(c);
		} else {
			if ( update & UpdateDefault )
				e->setCodec(c);
		}
	}
	
	//qDebug("new codec is : 0x%x (%s)", c, c ? c->name().constData() : "System");
	
	m_defaultCodec = c;
}

/*!
	\brief ctor
	
	\note Creates builtin menus/actions
*/
QEditor::QEditor(QWidget *p)
 : QAbstractScrollArea(p),
   pMenu(0), m_lineEndingsMenu(0), m_lineEndingsActions(0),
   m_bindingsMenu(0), aDefaultBinding(0), m_bindingsActions(0),
   m_doc(0), m_codec(m_defaultCodec), m_binding(m_defaultBinding), m_definition(0), m_curPlaceHolder(-1), m_state(defaultFlags())
{
	m_editors << this;
	
	m_saveState = Undefined;
	
	init();
}

/*!
	\brief ctor
	\param actions Whether builtin actions and menus should be created
*/
QEditor::QEditor(bool actions, QWidget *p)
 : QAbstractScrollArea(p),
   pMenu(0), m_lineEndingsMenu(0), m_lineEndingsActions(0),
   m_bindingsMenu(0), aDefaultBinding(0), m_bindingsActions(0),
   m_doc(0), m_codec(m_defaultCodec), m_binding(m_defaultBinding), m_definition(0), m_curPlaceHolder(-1), m_state(defaultFlags())
{
	m_editors << this;
	
	m_saveState = Undefined;
	
	init(actions);
}

/*!
	\brief ctor
	\param s file to load
	
	\note Creates builtin menus/actions
*/
QEditor::QEditor(const QString& s, QWidget *p)
 : QAbstractScrollArea(p),
   pMenu(0), m_lineEndingsMenu(0), m_lineEndingsActions(0),
   m_bindingsMenu(0), aDefaultBinding(0), m_bindingsActions(0),
   m_doc(0), m_codec(m_defaultCodec), m_binding(m_defaultBinding), m_definition(0), m_curPlaceHolder(-1), m_state(defaultFlags())
{
	m_editors << this;
	
	m_saveState = Undefined;
	
	init();
	
	setText(s);
}

/*!
	\brief ctor
	\param s file to load
	\param actions Whether builtin actions and menus should be created
	\note Creates builtin menus/action
*/
QEditor::QEditor(const QString& s, bool actions, QWidget *p)
 : QAbstractScrollArea(p),
   pMenu(0), m_lineEndingsMenu(0), m_lineEndingsActions(0),
   m_bindingsMenu(0), aDefaultBinding(0), m_bindingsActions(0),
   m_doc(0), m_codec(m_defaultCodec), m_binding(m_defaultBinding), m_definition(0), m_curPlaceHolder(-1), m_state(defaultFlags())
{
	m_editors << this;
	
	m_saveState = Undefined;
	
	init(actions);
	
	setText(s);
}

/*!
	\brief dtor
*/
QEditor::~QEditor()
{
	m_editors.removeAll(this);
	
	if ( m_completionEngine )
		delete m_completionEngine;
	
	if ( m_doc )
		delete m_doc;
	
	if ( m_editors.isEmpty() )
	{
		delete watcher();
	}
}

/*!
	\internal
*/
void QEditor::init(bool actions)
{
	#ifdef Q_GL_EDITOR
	setViewport(new QGLWidget);
	#endif
	
	viewport()->setCursor(Qt::IBeamCursor);
	viewport()->setBackgroundRole(QPalette::Base);
	//viewport()->setAttribute(Qt::WA_OpaquePaintEvent, true);
	viewport()->setAttribute(Qt::WA_KeyCompression, true);
	viewport()->setAttribute(Qt::WA_InputMethodEnabled, true);
	
	verticalScrollBar()->setSingleStep(1);
	horizontalScrollBar()->setSingleStep(20);
	
	setAcceptDrops(true);
	//setDragEnabled(true);
	setFrameShadow(QFrame::Plain);
	setFocusPolicy(Qt::WheelFocus);
	setAttribute(Qt::WA_KeyCompression, true);
	setAttribute(Qt::WA_InputMethodEnabled, true);
	
	connect(this							,
			SIGNAL( markChanged(QString, QDocumentLineHandle*, int, bool) ),
			QLineMarksInfoCenter::instance(),
			SLOT  ( markChanged(QString, QDocumentLineHandle*, int, bool) ) );
	
	m_doc = new QDocument(this);
	
	connect(m_doc	, SIGNAL( formatsChange (int, int) ),
			this	, SLOT  ( repaintContent(int, int) ) );
	
	connect(m_doc	, SIGNAL( contentsChange(int, int) ),
			this	, SLOT  ( updateContent (int, int) ) );
	
	connect(m_doc		, SIGNAL( formatsChanged() ),
			viewport()	, SLOT  ( update() ) );
	
	connect(m_doc	, SIGNAL( widthChanged(int) ),
			this	, SLOT  ( documentWidthChanged(int) ) );
	
	connect(m_doc	, SIGNAL( heightChanged(int) ),
			this	, SLOT  ( documentHeightChanged(int) ) );
	
	connect(m_doc	, SIGNAL( cleanChanged(bool) ),
			this	, SLOT  ( setContentClean(bool) ) );
	
	connect(m_doc	, SIGNAL( undoAvailable(bool) ),
			this	, SIGNAL( undoAvailable(bool) ) );
	
	connect(m_doc	, SIGNAL( redoAvailable(bool) ),
			this	, SIGNAL( redoAvailable(bool) ) );
	
	connect(m_doc	, SIGNAL( markChanged(QDocumentLineHandle*, int, bool) ),
			this	, SLOT  ( markChanged(QDocumentLineHandle*, int, bool) ) );
	
	connect(m_doc	, SIGNAL( lineEndingChanged(int) ),
			this	, SLOT  ( lineEndingChanged(int) ) );
	
	m_cursor = QDocumentCursor(m_doc);
	m_cursor.setAutoUpdated(true);
	
	//m_doc->setEditCursor(&m_cursor);
	
	if ( actions )
	{
		pMenu = new QMenu;
		
		QAction *a, *sep;
		
		a = new QAction(QIcon(":/undo.png"), tr("&Undo"), this);
		a->setObjectName("undo");
		Q_SHORTCUT(a, "Ctrl+Z", "Edit");
		a->setEnabled(false);
		connect(this , SIGNAL( undoAvailable(bool) ),
				a	, SLOT  ( setEnabled(bool) ) );
		connect(a	, SIGNAL( triggered() ),
				this , SLOT  ( undo() ) );
		
		addAction(a, "&Edit", "Edit");
		
		a = new QAction(QIcon(":/redo.png"), tr("&Redo"), this);
		a->setObjectName("redo");
		Q_SHORTCUT(a, "Ctrl+Y", "Edit");
		a->setEnabled(false);
		connect(this , SIGNAL( redoAvailable(bool) ),
				a	, SLOT  ( setEnabled(bool) ) );
		connect(a	, SIGNAL( triggered() ),
				this , SLOT  ( redo() ) );
		
		addAction(a, "&Edit", "Edit");
		
		sep = new QAction(this);
		sep->setSeparator(true);
		addAction(sep, "&Edit", "Edit");

		a = new QAction(QIcon(":/cut.png"), tr("Cu&t"), this);
		a->setObjectName("cut");
		Q_SHORTCUT(a, "Ctrl+X", "Edit");
		a->setEnabled(false);
		connect(this, SIGNAL( copyAvailable(bool) ),
				a	, SLOT  ( setEnabled(bool) ) );
		connect(a	, SIGNAL( triggered() ),
				this, SLOT  ( cut() ) );
		
		addAction(a, "&Edit", "Edit");

		a = new QAction(QIcon(":/copy.png"), tr("&Copy"), this);
		a->setObjectName("copy");
		Q_SHORTCUT(a, "Ctrl+C", "Edit");
		a->setEnabled(false);
		connect(this , SIGNAL( copyAvailable(bool) ),
				a	, SLOT  ( setEnabled(bool) ) );
		connect(a	, SIGNAL( triggered() ),
				this , SLOT  ( copy() ) );
		
		addAction(a, "&Edit", "Edit");

		a = new QAction(QIcon(":/paste.png"), tr("&Paste"), this);
		a->setObjectName("paste");
		//aPaste->setEnabled(QApplication::clipboard()->text().count());
		Q_SHORTCUT(a, "Ctrl+V", "Edit");
		connect(QApplication::clipboard()	, SIGNAL( dataChanged() ),
				this						, SLOT  ( checkClipboard() ) );
		
		connect(a	, SIGNAL( triggered() ),
				this, SLOT  ( paste() ) );
		
		addAction(a, "&Edit", "Edit");
		
		sep = new QAction(this);
		sep->setSeparator(true);
		addAction(sep, "&Edit", "Edit");
		
		a = new QAction(QIcon(":/indent.png"), tr("&Indent"), this);
		a->setObjectName("indent");
		Q_SHORTCUT(a, "Ctrl+I", "Edit");
		connect(a	, SIGNAL( triggered() ),
				this, SLOT  ( indentSelection() ) );
		
		addAction(a, "&Edit", "Edit");
		
		a = new QAction(QIcon(":/unindent.png"), tr("&Unindent"), this);
		a->setObjectName("unindent");
		Q_SHORTCUT(a, "Ctrl+Shift+I", "Edit");
		connect(a	, SIGNAL( triggered() ),
				this, SLOT  ( unindentSelection() ) );
		
		addAction(a, "&Edit", "Edit");
		
		sep = new QAction(this);
		sep->setSeparator(true);
		addAction(sep, "&Edit", "");
		
		a = new QAction(QIcon(":/comment.png"), tr("Co&mment"), this);
		a->setObjectName("comment");
		Q_SHORTCUT(a, "Ctrl+D", "Edit");
		connect(a	, SIGNAL( triggered() ),
				this, SLOT  ( commentSelection() ) );
		
		addAction(a, "&Edit", "Edit");
		
		a = new QAction(QIcon(":/uncomment.png"), tr("Unc&omment"), this);
		a->setObjectName("uncomment");
		Q_SHORTCUT(a, "Ctrl+Shift+D", "Edit");
		connect(a	, SIGNAL( triggered() ),
				this, SLOT  ( uncommentSelection() ) );
		
		addAction(a, "&Edit", "Edit");
		
		sep = new QAction(this);
		sep->setSeparator(true);
		addAction(sep, "&Edit", "");
		
		a = new QAction(tr("&Select all"), this);
		a->setObjectName("selectAll");
		Q_SHORTCUT(a, "Ctrl+A", "Edit");
		connect(a	, SIGNAL( triggered() ),
				this, SLOT  ( selectAll() ) );
		
		addAction(a, "&Edit", "Edit");
		
		sep = new QAction(this);
		sep->setSeparator(true);
		addAction(sep, QString());
		
		a = new QAction(QIcon(":/find.png"), tr("&Find"), this);
		a->setObjectName("find");
		Q_SHORTCUT(a, "Ctrl+F", "Search");
		connect(a	, SIGNAL( triggered() ),
				this, SLOT  ( find() ) );
		
		addAction(a, "&Search", "Search");
		
		a = new QAction(QIcon(":/next.png"), tr("Fin&d next"), pMenu);
		a->setObjectName("findNext");
		Q_SHORTCUT(a, "F3", "Search");
		connect(a	, SIGNAL( triggered() ),
				this, SLOT  ( findNext() ) );
		
		addAction(a, "&Search", "Search");
		
		a = new QAction(QIcon(":/replace.png"), tr("&Replace"), this);
		a->setObjectName("replace");
		Q_SHORTCUT(a, "Ctrl+R", "Search");
		connect(a	, SIGNAL( triggered() ),
				this, SLOT  ( replace() ) );
		
		addAction(a, "&Search", "Search");
		
		sep = new QAction(this);
		sep->setSeparator(true);
		addAction(sep, "&Search", "Search");
		
		a = new QAction(QIcon(":/goto.png"), tr("&Goto line..."), this);
		a->setObjectName("goto");
		Q_SHORTCUT(a, "Ctrl+G", "Search");
		connect(a	, SIGNAL( triggered() ),
				this, SLOT  ( gotoLine() ) );
		
		addAction(a, "&Search", "Search");
		
		sep = new QAction(this);
		sep->setSeparator(true);
		addAction(sep, "&Edit", "");
		
		a = new QAction(tr("Dynamic line wrapping"), this);
		a->setObjectName("wrap");
		a->setCheckable(true);
		a->setChecked(flag(LineWrap));
		
		addAction(a, "&Edit", "");
		
		Q_SHORTCUT(a, "F10", "Edit");
		connect(a	, SIGNAL( toggled(bool) ),
				this, SLOT  ( setLineWrapping(bool) ) );
		
		
		m_bindingsMenu = new QMenu(tr("Input binding"), this);
		m_bindingsActions = new QActionGroup(m_bindingsMenu);
		m_bindingsActions->setExclusive(true);
		
		connect(m_bindingsActions	, SIGNAL( triggered(QAction*) ),
				this				, SLOT  ( bindingSelected(QAction*) ) );
		
		aDefaultBinding = new QAction(tr("Default"), m_bindingsMenu);
		aDefaultBinding->setCheckable(true);
		aDefaultBinding->setData("default");
		
		m_bindingsMenu->addAction(aDefaultBinding);
		m_bindingsMenu->addSeparator();
		m_bindingsActions->addAction(aDefaultBinding);
		m_bindings["default"] = 0;
		
		updateBindingsMenu();
		
		m_bindingsMenu->menuAction()->setObjectName("bindings");
		addAction(m_bindingsMenu->menuAction(), "&Edit", "");
		
		sep = new QAction(this);
		sep->setSeparator(true);
		addAction(sep, QString());
		
		m_lineEndingsMenu = new QMenu(tr("Line endings"), this);
		m_lineEndingsActions = new QActionGroup(m_lineEndingsMenu);
		m_lineEndingsActions->setExclusive(true);
		
		connect(m_lineEndingsActions, SIGNAL( triggered(QAction*) ),
				this				, SLOT  ( lineEndingSelected(QAction*) ) );
		
		m_lineEndingsActions->addAction(tr("Conservative"))->setData("conservative");
		m_lineEndingsActions->addAction(tr("Local"))->setData("local");
		m_lineEndingsActions->addAction(tr("Unix/Linux"))->setData("unix");
		m_lineEndingsActions->addAction(tr("Dos/Windows"))->setData("dos");
		
		QList<QAction*> lle = m_lineEndingsActions->actions();
		
		foreach ( QAction *a, lle )
		{
			a->setCheckable(true);
			m_lineEndingsMenu->addAction(a);
		}
		
		lle.at(0)->setChecked(true);
		
		m_lineEndingsMenu->menuAction()->setObjectName("lineEndings");
		addAction(m_lineEndingsMenu->menuAction(), "&Edit", "");
		
		/*
		sep = new QAction(this);
		sep->setSeparator(true);
		addAction(sep, QString());
		*/
	}
}

/*!
	\return wether the flag \a f is set
*/
bool QEditor::flag(EditFlag f) const
{
	return m_state & f;
}

/*!
	\brief Sets the flag \a f
*/
void QEditor::setFlag(EditFlag f, bool b)
{
	if ( b )
	{
		m_state |= f;
		
		if ( f == LineWrap )
		{
			if ( isVisible() )
				m_doc->setWidthConstraint(wrapWidth());
			
			m_cursor.refreshColumnMemory();
			
			QAction *a = m_actions.value("wrap");
			
			if ( a && !a->isChecked() )
				a->setChecked(true);
		}
	} else {
		m_state &= ~f;
		
		if ( f == LineWrap )
		{
			if ( isVisible() )
				m_doc->clearWidthConstraint();
			
			m_cursor.refreshColumnMemory();
			
			QAction *a = m_actions.value("wrap");
			
			if ( a && a->isChecked() )
				a->setChecked(false);
		}
	}
	
	// TODO : only update cpos if cursor used to be visible?
	if ( f == LineWrap )
		ensureCursorVisible();
	
}

/*!
	\return whether it is possible to call undo()
*/
bool QEditor::canUndo() const
{
	return m_doc ? m_doc->canUndo() : false;
}

/*!
	\return whether it is possible to call redo()
*/
bool QEditor::canRedo() const
{
	return m_doc ? m_doc->canRedo() : false;
}

/*!
	\brief Set line wrapping
	\param on line wrap on/off
	
	\note the function also enables "cursor movement within wrapped lines"
	which can be disabled manually using setFlag(QEditor::CursorJumpPastWrap, false);
*/
void QEditor::setLineWrapping(bool on)
{
	setFlag(LineWrap, on);
	setFlag(CursorJumpPastWrap, on);
}

/*!
	\return The whole text being edited
*/
QString QEditor::text() const
{
	return m_doc ? m_doc->text() : QString();
}

/*!
	\return The text at a given line
	\param line text line to extract, using C++ array conventions (start at zero)
*/
QString QEditor::text(int line) const
{
	return m_doc ? m_doc->line(line).text() : QString();
}

/*!
	\brief Set the text of the underlying document and update display
*/
void QEditor::setText(const QString& s)
{
	clearPlaceHolders();
	
	if ( m_doc )
		m_doc->setText(s);
	
	setCursor(QDocumentCursor(m_doc));

	documentWidthChanged(m_doc->width());
	documentHeightChanged(m_doc->height());
	viewport()->update();
}

/*!
	\brief Save the underlying document to a file
	
	\see fileName()
*/
void QEditor::save()
{
	if ( !m_doc )
		return;
	
	QString oldFileName = fileName();
	
	if ( fileName().isEmpty() )
	{
		QString fn = QFileDialog::getSaveFileName();
		
		if ( fn.isEmpty() )
			return;
		
		setFileName(fn);
	} else if ( isInConflict() ) {
		int ret = QMessageBox::warning(this,
										tr("Conflict!"),
										tr(
											"%1\nhas been modified by another application.\n"
											"Press \"Save\" to overwrite the file on disk\n"
											"Press \"Reset\"to be reload the file from disk.\n"
											"Press \"Discard\" to ignore this warning.\n"
										).arg(fileName()),
											QMessageBox::Save
										|
											QMessageBox::Reset
										|
											QMessageBox::Discard
										|
											QMessageBox::Cancel
										);
		if ( ret == QMessageBox::Save )
		{
			m_saveState = Undefined;
		} else if ( ret == QMessageBox::Reset ) {
			load(fileName());
			m_saveState = Undefined;
			return;
		} else if ( ret == QMessageBox::Discard ) {
			m_saveState = Undefined;
			return;
		} else {
			return;
		}
	}
	
	m_saveState = Saving;
	
	if ( oldFileName.count() )
	{
		watcher()->removeWatch(oldFileName, this);
	}
	
	QFile f(fileName());
	
	if ( !f.open(QFile::WriteOnly) )
	{
		m_saveState = Undefined;
		reconnectWatcher();
		
		return;
	}
	
	//QTextStream s(&f);
	//s << text();
	QString txt = m_doc->text(flag(RemoveTrailing), flag(PreserveTrailingIndent));
	
	if ( m_codec )
		f.write(m_codec->fromUnicode(txt));
	else
		f.write(txt.toLocal8Bit());
	
	m_doc->setClean();
	
	emit saved(this, fileName());
	m_saveState = Saved;
	
	QTimer::singleShot(100, this, SLOT( reconnectWatcher() ));
	
	update();
}

/*!
	\brief Save the content of the editor to a file
	
	\note This method renames the editor, stop monitoring the old
	file and monitor the new one
*/
void QEditor::save(const QString& fn)
{
	if ( fileName().count() )
	{
		watcher()->removeWatch(fileName(), this);
	}
	
	QFile f(fn);
	
	if ( !f.open(QFile::WriteOnly) )
	{
		m_saveState = Undefined;
		reconnectWatcher();
		
		return;
	}
	
	QString txt = m_doc->text(flag(RemoveTrailing), flag(PreserveTrailingIndent));
	
	if ( m_codec )
		f.write(m_codec->fromUnicode(txt));
	else
		f.write(txt.toLocal8Bit());
	
	m_doc->setClean();
	
	setFileName(fn);
	emit saved(this, fn);
	m_saveState = Saved;
	
	QTimer::singleShot(100, this, SLOT( reconnectWatcher() ));
}

/*!
	\internal
*/
void QEditor::checkClipboard()
{
	// LOOKS LIKE THIS FUNCTION NEVER GETS CALLED DESPITE THE CONNECTION...
	
	const QMimeData *d = QApplication::clipboard()->mimeData();
	
	//qDebug("checking clipboard : %s", d);
	
	//QCE_ENABLE_ACTION("paste", d && d->hasText())
}

/*!
	\internal
*/
void QEditor::reconnectWatcher()
{
	watcher()->addWatch(fileName(), this);
}

/*!
	\internal
*/
void QEditor::fileChanged(const QString& file)
{
	if ( (file != fileName()) || (m_saveState == Saving) )
		return;
	
	if ( m_saveState == Saved )
	{
		qApp->processEvents();
		
		m_saveState = Undefined;
		return;
	}
	
	if ( !isContentModified() )
	{
		// silently reload file if the editor contains no modification?
		// -> result in undo/redo history loss, still ask confirmation ?
		bool autoReload = true;

		if ( canUndo() || canRedo() )
		{
			int ret = QMessageBox::warning(this,
										tr("File changed"),
										tr(
											"%1\nhas been modified by another application.\n\n"
											"Undo/Redo stack would be discarded by the auto-reload.\n"
											"Do you wish to keep up to date by reloading the file?"
										).arg(fileName()),
											QMessageBox::Yes
										|
											QMessageBox::No
										);
			
			if ( ret == QMessageBox::No )
				autoReload = false;
		}
		
		if ( autoReload )
		{
			load(fileName());
			m_saveState = Undefined;
			return;
		}
	}
	
	// TODO : check for actual modification (using a checksum?)
	// TODO : conflict reversible (checksum again?)
	
	/*
	QFile f(file);
	quint64 sz = f.size();
	
	if ( sz == m_lastFileState.size )
	{
		quint32 checksum = 0;
		f.open(QFile::ReadOnly);
		
		while ( !f.atEnd() )
		{
			QByteArray b = f.read(100000);
			
			checksum ^= qChecksum(b.constData(), b.size());
		}
		
		qDebug("%s : checksum = %i", qPrintable(file),  checksum);
		
		if ( checksum == m_lastFileState.checksum )
			return;
		
		m_lastFileState.checksum = checksum;
		
	} else {
		qDebug("%s : size = %i", qPrintable(file), sz);
		m_lastFileState.size = sz;
	}
	*/
	
	//qDebug("conflict!");
	m_saveState = Conflict;
}

/*!
	\return Whether a file conflict has been detected
	
	File conflicts happen when the loaded file is modified
	on disk by another application if the text has been
	modified in QCE
*/
bool QEditor::isInConflict() const
{
	return m_saveState == Conflict;
}

/*!
	\brief Print the content of the editor
*/
void QEditor::print()
{
	if ( !m_doc )
		return;
	
	QPrinter printer;
	
	// TODO : create a custom print dialog, page range sucks, lines range would be better
	QPrintDialog dialog(&printer, this);
	dialog.setEnabledOptions(QPrintDialog::PrintToFile | QPrintDialog::PrintPageRange);
	
	if ( dialog.exec() == QDialog::Accepted )
	{
		m_doc->print(&printer);
	}
}

/*!
	\brief Show the search/replace panel, if any
*/
void QEditor::find()
{
	QCodeEdit *m = QCodeEdit::manager(this);
	
	if ( m )
	{
		m->sendPanelCommand("Search",
							"display",
							Q_COMMAND
								<< Q_ARG(int, 1)
								<< Q_ARG(bool, false)
							);
		
	} else {
		qDebug("Unmanaged QEditor");
	}
}

/*!
	\brief Ask the search/replace panel, if any, to move to next match
*/
void QEditor::findNext()
{
	QCodeEdit *m = QCodeEdit::manager(this);
	
	if ( m )
	{
		m->sendPanelCommand("Search",
							"find",
							Q_COMMAND
								<< Q_ARG(int, -1)
							);
		
	} else {
		qDebug("Unmanaged QEditor");
	}
}

/*!
	\brief Show the search/replace panel, if any
*/
void QEditor::replace()
{
	QCodeEdit *m = QCodeEdit::manager(this);
	
	if ( m )
	{
		m->sendPanelCommand("Search",
							"display",
							Q_COMMAND
								<< Q_ARG(int, 1)
								<< Q_ARG(bool, true)
							);
		
	} else {
		qDebug("Unmanaged QEditor");
	}
}

/*!
	\brief Show a dialog to go to a specific line
*/
void QEditor::gotoLine()
{
	QGotoLineDialog dlg(this);
	
	dlg.exec(this);
}

/*!
	\brief Run time translation entry point for compat with Edyuk
*/
void QEditor::retranslate()
{
	QCE_TR_ACTION("undo", tr("&Undo"))
	QCE_TR_ACTION("redo", tr("&Redo"))

	QCE_TR_ACTION("cut", tr("Cu&t"))
	QCE_TR_ACTION("copy", tr("&Copy"))
	QCE_TR_ACTION("paste", tr("&Paste"))
	
	QCE_TR_ACTION("indent", tr("&Indent"))
	QCE_TR_ACTION("unindent", tr("&Unindent"))
	QCE_TR_ACTION("comment", tr("Co&mment"))
	QCE_TR_ACTION("uncomment", tr("Unc&omment"))
	
	QCE_TR_ACTION("selectAll", tr("&Select all"))
	
	QCE_TR_ACTION("find", tr("&Find"))
	QCE_TR_ACTION("findNext", tr("Fin&d next"))
	QCE_TR_ACTION("replace", tr("&Replace"))
	
	QCE_TR_ACTION("goto", tr("&Goto line..."))
	
	if ( m_completionEngine )
		m_completionEngine->retranslate();
	
	if ( m_bindingsMenu )
		m_bindingsMenu->setTitle(tr("Input binding"));
	
	if ( aDefaultBinding )
		aDefaultBinding->setText(tr("Default"));
	
	#ifdef _QMDI_
	menus.setTranslation("&Edit", tr("&Edit"));
	menus.setTranslation("&Search", tr("&Search"));
	
	toolbars.setTranslation("Edit", tr("Edit"));
	toolbars.setTranslation("Search", tr("Search"));
	#endif
}

/*!
	\return the action associated with a given name, if the QEditor has been created with actions on
*/
QAction* QEditor::action(const QString& s)
{
	QHash<QString, QAction*>::const_iterator it = m_actions.constFind(s);

	return it != m_actions.constEnd() ? *it : 0;
}

/*!
	\brief Add an action to the editor
	\param a action to add
	\param menu if not empty (and if QCE is built with qmdilib support) the action will be added to that menu
	\param toolbar similar to \a menu but acts on toolbars
	
	\see removeAction()
*/
void QEditor::addAction(QAction *a, const QString& menu, const QString& toolbar)
{
	if ( !a )
		return;

	QWidget::addAction(a);
	
	m_actions[a->objectName()] = a;
	
	if ( pMenu && menu.count() )
	{
		pMenu->addAction(a);
		
		#ifdef _QMDI_
		menus[menu]->addAction(a);
		#endif
	}
	
	if ( toolbar.count() )
	{
		#ifdef _QMDI_
		toolbars[toolbar]->addAction(a);
		#endif
	}
}

/*!
	\brief remove an action form the editor
	\param a action to add
	\param menu if not empty (and if QCE is built with qmdilib support) the action will be added to that menu
	\param toolbar similar to \a menu but acts on toolbars
	
	\see addAction()
*/
void QEditor::removeAction(QAction *a, const QString& menu, const QString& toolbar)
{
	if ( !a )
		return;
	
	QWidget::removeAction(a);
	
	//m_actions.remove(a->objectName());

	if ( pMenu )
		pMenu->removeAction(a);
	
	#ifdef _QMDI_
	if ( menu.count() )
	{
		menus[menu]->removeAction(a);
	}
	
	if ( toolbar.count() )
	{
		toolbars[toolbar]->removeAction(a);
	}
	#endif
}

/*!
	\brief load a text file
	\param file file to load
	
	If the file cannot be loaded, previous content is cleared.
*/
void QEditor::load(const QString& file)
{
	QFile f(file);
	
	// gotta handle line endings ourselves if we want to detect current line ending style...
	//if ( !f.open(QFile::Text | QFile::ReadOnly) )
	if ( !f.open(QFile::ReadOnly) )
	{
		setText(QString());
		return;
	}
	
	const int size = f.size();
	//const int size = m_lastFileState.size = f.size();
	
	if ( size < 500000 )
	{
		// instant load for files smaller than 500kb
		QByteArray d = f.readAll();
		//m_lastFileState.checksum = qChecksum(d.constData(), d.size());
		if ( m_codec )
			setText(m_codec->toUnicode(d));
		else
			setText(QString::fromLocal8Bit(d));
	} else {
		// load by chunks of 100kb otherwise to avoid huge peaks of memory usage
		// and driving mad the disk drivers
		
		int count = 0;
		QByteArray ba;
		
		m_doc->startChunkLoading();
		//m_lastFileState.checksum = 0;
		
		do
		{
			ba = f.read(100000);
			count += ba.count();
			
			//m_lastFileState.checksum ^= qChecksum(ba.constData(), ba.size());
			
			if ( m_codec )
				m_doc->addChunk(m_codec->toUnicode(ba));
			else
				m_doc->addChunk(QString::fromLocal8Bit(ba));
			
		} while ( (count < size) && ba.count() );
		
		m_doc->stopChunkLoading();
		
		setCursor(QDocumentCursor(m_doc));
		
		documentWidthChanged(m_doc->width());
		documentHeightChanged(m_doc->height());
	}
	
	//qDebug("checksum = %i", m_lastFileState.checksum);
	
	if ( m_lineEndingsActions )
	{
		// TODO : update Conservative to report original line endings
		static const QRegExp rx(" \\[\\w+\\]");
		QAction *a = m_lineEndingsActions->actions().at(0);
		
		if ( a )
		{
			QDocument::LineEnding le = m_doc->originalLineEnding();
			
			QString txt = a->text();
			txt.remove(rx);
			txt += " [";
			
			if ( le == QDocument::Windows )
				txt += tr("Windows");
			else
				txt += tr("Unix");
			
			txt += ']';
			
			a->setText(txt);
		}
	}

	setFileName(file);
}

/*!
	\return a pointer to the underlying QDocument object
*/
QDocument* QEditor::document() const
{
	return m_doc;
}

/*!
	\internal
*/
void QEditor::setDocument(QDocument *d)
{
	Q_UNUSED(d)
	
	qWarning("QEditor::setDocument() is not working yet...");
}

/*!
	\return The text codec to use for load/save operations
*/
QTextCodec* QEditor::codec() const
{
	return m_codec;
}

/*!
	\overload
*/
void QEditor::setCodec(int mib)
{
	setCodec(QTextCodec::codecForMib(mib));
}

/*!
	\overload
*/
void QEditor::setCodec(const char *name)
{
	setCodec(QTextCodec::codecForName(name));
}

/*!
	\overload
*/
void QEditor::setCodec(const QByteArray& name)
{
	setCodec(QTextCodec::codecForName(name));
}

/*!
	\brief Set the text codec to use for load/save operations
*/
void QEditor::setCodec(QTextCodec *c)
{
	if ( c == m_codec )
		return;
	
	m_codec = c;
	
	// TODO : reload file?
	if ( fileName().count() && QFile::exists(fileName()) )
	{
		if ( !isContentModified() )
		{
			load(fileName());
		}
	}
}

/*!
	\brief Force a full re-highlighting of the document
*/
void QEditor::highlight()
{
	m_doc->highlight();
	//updateContent(0, m_doc->lines());
}

/*!
	\return the current InputBinding
*/
QEditor::InputBinding* QEditor::inputBinding() const
{
	return m_binding;
}

/*!
	\brief Set the current input binding
*/
void QEditor::setInputBinding(QEditor::InputBinding *b)
{
	m_binding = b;
	
	if ( !aDefaultBinding || !m_bindingsActions )
		return;
	
	QString id = b ? b->id() : QString();
	aDefaultBinding->setChecked(!b);
	
	if ( !b )
		return;
	
	QList<QAction*> actions = m_bindingsActions->actions();
	
	foreach ( QAction *a, actions )
	{
		if ( a->data().toString() != id )
			continue;
		
		a->setChecked(true);
		break;
	}
}

/*!
	\internal
*/
void QEditor::updateBindingsMenu()
{
	if ( !aDefaultBinding || !m_bindingsMenu || !m_bindingsActions )
		return;

	QStringList bindings = inputBindings();
	QList<QAction*> actions = m_bindingsActions->actions();
	
	QString id = m_binding ? m_binding->id() : QString();
	aDefaultBinding->setChecked(!m_binding);
	
	foreach ( QAction *a, actions )
	{
		int idx = bindings.indexOf(a->data().toString());
		
		if ( idx == -1 )
		{
			m_bindingsMenu->removeAction(a);
			m_bindingsActions->removeAction(a);
			delete a;
		} else {
			bindings.removeAt(idx);
			
			if ( a->data().toString() == id )
				a->setChecked(true);
			
		}
	}
	
	bindings.removeAll("default");
	
	foreach ( QString s, bindings )
	{
		InputBinding *b = m_bindings.value(s);
		
		if ( !b )
			continue;
		
		QAction *a = new QAction(b->name(), m_bindingsMenu);
		a->setData(b->id());
		a->setCheckable(true);
		
		m_bindingsActions->addAction(a);
		m_bindingsMenu->addAction(a);
	}
}

/*!
	\internal
*/
void QEditor::bindingSelected(QAction *a)
{
	a = m_bindingsActions->checkedAction();
	m_binding = m_bindings.value(a->data().toString());
	
	//qDebug("setting binding to %s [0x%x]", qPrintable(a->data().toString()), m_binding);
	
	updateMicroFocus();
}

/*!
	\internal
*/
void QEditor::lineEndingSelected(QAction *a)
{
	a = m_lineEndingsActions->checkedAction();
	
	if ( !a )
		return;
	
	QString le = a->data().toString();
	
	if ( le == "conservative" )
		m_doc->setLineEnding(QDocument::Conservative);
	else if ( le == "local" )
		m_doc->setLineEnding(QDocument::Local);
	else if ( le == "unix" )
		m_doc->setLineEnding(QDocument::Unix);
	else if ( le == "dos" )
		m_doc->setLineEnding(QDocument::Windows);
	
	
	updateMicroFocus();
}

/*!
	\internal
*/
void QEditor::lineEndingChanged(int lineEnding)
{
	if ( !m_lineEndingsActions )
		return;
	
	QAction *a = m_lineEndingsActions->checkedAction(),
			*n = m_lineEndingsActions->actions().at(lineEnding);
	
	if ( a != n )
		n->setChecked(true);
	
}

/*!
	\return the current cursor
*/
QDocumentCursor QEditor::cursor() const
{
	QDocumentCursor copy = m_cursor;
	copy.setAutoUpdated(false);
	return copy;
}

/*!
	\brief Set the document cursor
*/
void QEditor::setCursor(const QDocumentCursor& c)
{
	repaintCursor();
	
	m_cursor = c.isValid() ? c : QDocumentCursor(m_doc);
	m_cursor.setAutoUpdated(true);
	clearCursorMirrors();
	
	emitCursorPositionChanged();
	
	setFlag(CursorOn, true);
	repaintCursor();
	ensureCursorVisible();
	selectionChange();
	
	updateMicroFocus();
}

/*!
	\brief Set the cursor
	\param line document line to move the cursor to (start at zero)
	\param index column index of the new cursor (start at zero)
*/
void QEditor::setCursorPosition(int line, int index)
{
	setCursor(QDocumentCursor(m_doc, line, index));
}

/*!
	\brief Write the current cursor position to to integers
*/
void QEditor::getCursorPosition(int &line, int &index)
{
	line = m_cursor.lineNumber();
	index = m_cursor.columnNumber();
}

/*!
	\return the number of cursor mirrors currently used
*/
int QEditor::cursorMirrorCount() const
{
	return m_mirrors.count();
}

/*!
	\return the cursor mirror at index \a i
	
	Index has no extra meaning : you cannot deduce anything about
	the cursor mirror it corresponds to from it. For instance, the
	cursor mirror at index 0 is neither the first mirror added nor
	the one at smallest document position (well : it *might* be but
	that would be a coincidence...)
*/
QDocumentCursor QEditor::cursorMirror(int i) const
{
	return i >= 0 && i < m_mirrors.count() ? m_mirrors.at(i) : QDocumentCursor();
}

/*!
	\brief Clear all placeholders
*/
void QEditor::clearPlaceHolders()
{
	m_curPlaceHolder = -1;
	
	for ( int i = 0; i < m_placeHolders.count(); ++i )
	{
		PlaceHolder& ph = m_placeHolders[i];
		
		ph.cursor.setAutoUpdated(false);
		
		for ( int j = 0; j < ph.mirrors.count(); ++j )
		{
			ph.mirrors[j].setAutoUpdated(false);
		}
		
		ph.mirrors.clear();
	}
	
	m_placeHolders.clear();
}

/*!
	\brief Add a placeholder
	\param p placeholder data
	\param autoUpdate whether to force auto updating of all cursors used by the placeholder
	
	Auto update is on by default and it is recommended not to disable it unless you know what you are doing.
*/
void QEditor::addPlaceHolder(const PlaceHolder& p, bool autoUpdate)
{
	m_placeHolders << p;

	PlaceHolder& ph = m_placeHolders.last();

	ph.cursor.setAutoUpdated(true);
	ph.cursor.movePosition(ph.length, QDocumentCursor::NextCharacter, QDocumentCursor::KeepAnchor);
	
	for ( int i = 0; i < ph.mirrors.count(); ++i )
	{
		ph.mirrors[i].setAutoUpdated(true);
		ph.mirrors[i].movePosition(ph.length, QDocumentCursor::NextCharacter, QDocumentCursor::KeepAnchor);
	}
}

/*!
	\return the number of placeholders currently set
*/
int QEditor::placeHolderCount() const
{
	return m_placeHolders.count();
}

/*!
	\brief Set the current placeholder to use
	
	This function change the cursor and the cursor mirrors.
*/
void QEditor::setPlaceHolder(int i)
{
	if ( i < 0 || i >= m_placeHolders.count() )
		return;
	
	clearCursorMirrors();
	
	const PlaceHolder& ph = m_placeHolders.at(i);
	QDocumentCursor cc = ph.cursor;
	
	//if ( ph.length > 0 )
	//	cc.movePosition(ph.length, QDocumentCursor::NextCharacter, QDocumentCursor::KeepAnchor);
	
	setCursor(cc);
	
	foreach ( cc, ph.mirrors )
	{
		//if ( ph.length > 0 )
		//	cc.movePosition(ph.length, QDocumentCursor::NextCharacter, QDocumentCursor::KeepAnchor);
		
		addCursorMirror(cc);
	}
	
	m_curPlaceHolder = i;
}

/*!
	\brief Move to next placeholder
	
	\see setPlaceHolder
*/
void QEditor::nextPlaceHolder()
{
	if ( m_placeHolders.isEmpty() )
		return;
	
	++m_curPlaceHolder;
	
	if ( m_curPlaceHolder >= m_placeHolders.count() )
		m_curPlaceHolder = 0;
	
	setPlaceHolder(m_curPlaceHolder);
}

/*!
	\brief Move to previous placeholder
	
	\see setPlaceHolder
*/
void QEditor::previousPlaceHolder()
{
	if ( m_placeHolders.isEmpty() )
		return;
	
	if ( m_curPlaceHolder <= 0 )
		m_curPlaceHolder = m_placeHolders.count();
	
	--m_curPlaceHolder;
	
	setPlaceHolder(m_curPlaceHolder);
}

/*!
	\return the code completion engine set to this editor, if any
*/
QCodeCompletionEngine* QEditor::completionEngine() const
{
	return m_completionEngine;
}

/*!
	\brief Set a code completion engine to the editor
	
	\warning Most completion engines can only be attached
	to a single editor due to issues in the widget used to
	dispaly matches so you got to clone them and, as a consequence
	QEditor will take ownership of the completion engines
	and delete them.
*/
void QEditor::setCompletionEngine(QCodeCompletionEngine *e)
{
	if ( m_completionEngine )
	{
		m_completionEngine->setEditor(0);
		m_completionEngine->deleteLater();
	}
	
	m_completionEngine = e;
	
	if ( m_completionEngine )
	{
		m_completionEngine->setEditor(this);
	}
}

/*!
	\return the language definition set to this editor, if any
*/
QLanguageDefinition* QEditor::languageDefinition() const
{
	return m_definition;
}

/*!
	\brief Set a language definition to the editor
*/
void QEditor::setLanguageDefinition(QLanguageDefinition *d)
{
	m_definition = d;
	
	if ( m_doc )
		m_doc->setLanguageDefinition(d);
	
	if ( m_definition )
	{
		bool cuc = d->singleLineComment().count();
		
		QCE_ENABLE_ACTION("comment", cuc)
		QCE_ENABLE_ACTION("uncomment", cuc)
	} else {
		QCE_ENABLE_ACTION("comment", false)
		QCE_ENABLE_ACTION("uncomment", false)
	}
}

/*!
	\return the line at a given viewport position
*/
QDocumentLine QEditor::lineAtPosition(const QPoint& p) const
{
	return m_doc ? m_doc->lineAt(p) : QDocumentLine();
}

/*!
	\return The cursor object nearest to the given viewport position
*/
QDocumentCursor QEditor::cursorForPosition(const QPoint& p) const
{
	//qDebug("cursor for : (%i, %i)", p.x(), p.y());
	
	return m_doc ? m_doc->cursorAt(p) : QDocumentCursor();
}

/*!
	\brief Set the cursor to that nearest to a given viewport position
*/
void QEditor::setCursorPosition(const QPoint& p)
{
	//qDebug("cursor for : (%i, %i)", p.x(), p.y());
	
	QDocumentCursor c = cursorForPosition(p);
	
	if ( c.isValid() )
	{
		setCursor(c);
	}
}

/*!
	\brief Emitted whenever the position of the cursor changes
*/
void QEditor::emitCursorPositionChanged()
{
	emit cursorPositionChanged();
	emit copyAvailable(m_cursor.hasSelection());
	
	if ( m_definition )
		m_definition->match(m_cursor);
	
	if ( m_doc->impl()->hasMarks() )
		QLineMarksInfoCenter::instance()->cursorMoved(this);
	
}

/*!
	\brief Undo the last editing operation, if any on the stack
*/
void QEditor::undo()
{
	if ( m_doc )
	{
		if ( m_definition )
			m_definition->clearMatches(m_doc);
		
		m_doc->undo();
		
		selectionChange();
		ensureCursorVisible();
		setFlag(CursorOn, true);
		emitCursorPositionChanged();
		repaintCursor();
	}
}

/*!
	\brief Redo the last undone editing operation, if any on the stack
*/
void QEditor::redo()
{
	if ( m_doc )
	{
		if ( m_definition )
			m_definition->clearMatches(m_doc);
		
		m_doc->redo();
		
		selectionChange();
		ensureCursorVisible();
		setFlag(CursorOn, true);
		emitCursorPositionChanged();
		repaintCursor();
	}
}

/*!
	\brief Cut the selected text, if any
*/
void QEditor::cut()
{
	copy();
	
	bool macro = m_mirrors.count();
	
	if ( macro )
		m_doc->beginMacro();
	
	m_cursor.removeSelectedText();
	
	for ( int i = 0; i < m_mirrors.count(); ++i )
		m_mirrors[i].removeSelectedText();
	
	if ( macro )
		m_doc->endMacro();
	
	clearCursorMirrors();
	
	ensureCursorVisible();
	setFlag(CursorOn, true);
	emitCursorPositionChanged();
	repaintCursor();
}

/*!
	\brief Copy the selected text, if any
	
	\note Column selection may be created but the can only be copied properly in a QCE editor
*/
void QEditor::copy()
{
	if ( !m_cursor.hasSelection() )
		return;
	
	QMimeData *d = createMimeDataFromSelection();
	QApplication::clipboard()->setMimeData(d);
	
	//qDebug("%s", qPrintable(m_cursor.selectedText()));
	//QApplication::clipboard()->setText(m_cursor.selectedText());
}

/*!
	\brief Paste text from the clipboard to the current cursor position
	
	\note May paste column selections from other QCE editors
*/
void QEditor::paste()
{
	const QMimeData *d = QApplication::clipboard()->mimeData();
	
	if ( d )
		insertFromMimeData(d);
}

static bool unindent(const QDocumentCursor& cur)
{
	QDocumentLine beg(cur.line());
	int r = 0, n = 0, t = QDocument::tabStop();
	QString txt = beg.text().left(beg.firstChar());
	
	while ( txt.count() && (n < t) )
	{
		if ( txt.at(txt.length() - 1) == '\t' )
			n += t - (n % t);
		else
			++n;
		
		++r;
		txt.chop(1);
	}
	
	if ( !r )
		return false;
	
	QDocumentCursor c(cur);
	c.setSilent(true);
	c.movePosition(1, QDocumentCursor::StartOfBlock, QDocumentCursor::MoveAnchor);
	c.movePosition(r, QDocumentCursor::Right, QDocumentCursor::KeepAnchor);
	c.removeSelectedText();
	
	return true;
}

static void insert(const QDocumentCursor& cur, const QString& txt)
{
	QDocumentCursor c(cur);
	c.setSilent(true);
	c.setColumnNumber(0);
	c.insertText(txt);
}

static void removeFromStart(const QDocumentCursor& cur, const QString& txt)
{
	QDocumentLine l = cur.line();
	int pos = l.firstChar();
	
	if ( l.text().mid(pos, txt.length()) != txt )
		return;
	
	QDocumentCursor c(cur.document(), cur.lineNumber(), pos);
	c.setSilent(true);
	c.movePosition(txt.length(),
					QDocumentCursor::NextCharacter,
					QDocumentCursor::KeepAnchor);
	c.removeSelectedText();
}

/*!
	\brief Indent the selection
*/
void QEditor::indentSelection()
{
	QString txt("\t");
	
	if ( m_mirrors.count() )
	{
		m_doc->beginMacro();
		
		insert(m_cursor, txt);
		
		foreach ( const QDocumentCursor& m, m_mirrors )
			insert(m, txt);
		
		m_doc->endMacro();
		
	} else if ( !m_cursor.hasSelection() ) {
		insert(m_cursor, txt);
	} else {
		QDocumentSelection s = m_cursor.selection();
		QDocumentCursor c(m_doc, s.startLine);
		c.setSilent(true);
		c.beginEditBlock();
		
		while ( c.isValid() && (c.lineNumber() <= s.endLine) )
		{
			c.insertText(txt);
			c.movePosition(1, QDocumentCursor::NextLine);
			
			if ( c.atEnd() )
				break;
		}
		
		c.endEditBlock();
	}
}

/*!
	\brief Unindent the selection
*/
void QEditor::unindentSelection()
{
	if ( !m_cursor.line().firstChar() )
		return;
	
	if ( m_mirrors.count() )
	{
		m_doc->beginMacro();
		
		unindent(m_cursor);
		
		foreach ( const QDocumentCursor& m, m_mirrors )
			unindent(m);
		
		m_doc->endMacro();
		
	} else if ( !m_cursor.hasSelection() ) {
		unindent(m_cursor);
	} else {
		QDocumentSelection s = m_cursor.selection();
		
		m_doc->beginMacro();
		
		for ( int i = s.startLine; i <= s.endLine; ++i )
		{
			unindent(QDocumentCursor(m_doc, i));
		}
		
		m_doc->endMacro();
	}
}

/*!
	\brief Comment the selection (or the current line) using single line comments supported by the language 
*/
void QEditor::commentSelection()
{
	if ( !m_definition )
		return;
	
	QString txt = m_definition->singleLineComment();
	
	if ( txt.isEmpty() )
		return;
	
	if ( m_mirrors.count() )
	{
		m_doc->beginMacro();
		
		insert(m_cursor, txt);
		
		foreach ( const QDocumentCursor& m, m_mirrors )
			insert(m, txt);
		
		m_doc->endMacro();
		
	} else if ( !m_cursor.hasSelection() ) {
		insert(m_cursor, txt);
	} else {
		QDocumentSelection s = m_cursor.selection();
		QDocumentCursor c(m_doc, s.startLine);
		c.setSilent(true);
		c.beginEditBlock();
		
		while ( c.isValid() && (c.lineNumber() <= s.endLine) )
		{
			c.insertText(txt);
			c.movePosition(1, QDocumentCursor::NextLine);
			
			if ( c.atEnd() )
				break;
		}
		
		c.endEditBlock();
	}
}

/*!
	\brief Uncomment the selection (or current line), provided it has been commented with single line comments
*/
void QEditor::uncommentSelection()
{
	if ( !m_definition )
		return;
	
	QString txt = m_definition->singleLineComment();
	
	if ( txt.isEmpty() )
		return;
	
	if ( m_mirrors.count() )
	{
		m_doc->beginMacro();
		
		removeFromStart(m_cursor, txt);
		
		foreach ( const QDocumentCursor& m, m_mirrors )
			removeFromStart(m, txt);
		
		m_doc->endMacro();
		
	} else if ( !m_cursor.hasSelection() ) {
		removeFromStart(m_cursor, txt);
	} else {
		QDocumentSelection s = m_cursor.selection();
		
		m_doc->beginMacro();
		
		for ( int i = s.startLine; i <= s.endLine; ++i )
		{
			removeFromStart(QDocumentCursor(m_doc, i), txt);
		}
		
		m_doc->endMacro();
	}
}

/*!
	\brief Select the whole text
*/
void QEditor::selectAll()
{
	clearCursorMirrors();
	
	m_cursor.movePosition(1, QDocumentCursor::Start);
	m_cursor.movePosition(1, QDocumentCursor::End, QDocumentCursor::KeepAnchor);
	
	emitCursorPositionChanged();
	selectionChange(true);
	
	viewport()->update();
}

/*!
	\internal
*/
bool QEditor::event(QEvent *e)
{
	bool r = QAbstractScrollArea::event(e);
	
	if ( (e->type() == QEvent::Resize || e->type() == QEvent::Show) && m_doc )
		verticalScrollBar()->setMaximum(qMax(0, 1 + (m_doc->height() - viewport()->height()) / m_doc->fontMetrics().lineSpacing()));
	
	if ( e->type() == QEvent::Resize && flag(LineWrap) )
	{
		//qDebug("resize adjust (1) : wrapping to %i", viewport()->width());
		m_doc->setWidthConstraint(wrapWidth());
		ensureCursorVisible();
	}
	
	return r;
}

/*!
	\internal
*/
void QEditor::paintEvent(QPaintEvent *e)
{
	if ( !m_doc )
		return;
	
	QPainter p(viewport());
	const int yOffset = verticalOffset();
	const int xOffset = horizontalOffset();
	
	#ifdef Q_GL_EDITOR
	//QRect r(e->rect());
	QRect r(0, 0, viewport()->width(), viewport()->height());
	#else
	QRect r(e->rect());
	#endif
	
	//qDebug() << r;
	
	//p.setClipping(false);
	p.translate(-xOffset, -yOffset);
	
	QDocument::PaintContext ctx;
	ctx.xoffset = xOffset;
	ctx.yoffset = r.y() + yOffset;
	ctx.width = viewport()->width();
	ctx.height = qMin(r.height(), viewport()->height());
	ctx.palette = palette();
	ctx.cursors << m_cursor.handle();
	ctx.fillCursorRect = true;
	ctx.blinkingCursor = flag(CursorOn);
	
	if ( m_cursor.hasSelection() )
	{
		//qDebug("atempting to draw selected text");
		QDocumentSelection s = m_cursor.selection();
		
		ctx.selections << s;
	}
	
	// cursor mirrors :D
	foreach ( const QDocumentCursor& m, m_mirrors )
	{
		if ( ctx.blinkingCursor )
			ctx.extra << m.handle();
		
		if ( m.hasSelection() )
		{
			QDocumentSelection s = m.selection();
			
			ctx.selections << s;
		}
	}
	
	if ( m_dragAndDrop.isValid() )
	{
		ctx.extra << m_dragAndDrop.handle();
	}
	
	p.save();
	m_doc->draw(&p, ctx);
	p.restore();
	
	if ( m_curPlaceHolder >= 0 && m_curPlaceHolder < m_placeHolders.count() )
	{
		const PlaceHolder& ph = m_placeHolders.at(m_curPlaceHolder);
		
		foreach ( const QDocumentCursor& m, ph.mirrors )
		{
			if ( m.isValid() )
				p.drawConvexPolygon(m.documentRegion());
		}
	}
	
	if ( viewport()->height() > m_doc->height() )
	{
		p.fillRect(	0,
					m_doc->height(),
					viewport()->width(),
					viewport()->height() - m_doc->height(),
					palette().base()
				);
	}
}

/*!
	\internal
*/
void QEditor::timerEvent(QTimerEvent *e)
{
	int id = e->timerId();
	
	if ( id == m_blink.timerId() )
	{
		bool on = !flag(CursorOn);
		
		if ( m_cursor.hasSelection() )
			on &= style()->styleHint(QStyle::SH_BlinkCursorWhenTextSelected,
									0,
									this) != 0;
		
		setFlag(CursorOn, on);
		
		repaintCursor();
		
	} else if ( id == m_drag.timerId() ) {
		m_drag.stop();
		//startDrag();
	} else if ( id == m_click.timerId() ) {
		m_click.stop();
	}
}

static int max(const QList<QDocumentCursor>& l)
{
	int ln = 0;
	
	foreach ( const QDocumentCursor& c, l )
		if ( c.lineNumber() > ln )
			ln = c.lineNumber();
	
	return ln;
}

static int min(const QList<QDocumentCursor>& l)
{
	// beware the sign bit...
	int ln = 0x7fffffff;
	
	foreach ( const QDocumentCursor& c, l )
		if ( (c.lineNumber() < ln) || (ln < 0) )
			ln = c.lineNumber();
	
	return ln;
}

static bool protectedCursor(const QDocumentCursor& c)
{
	QDocumentLine l = c.line();
	
	bool prot = 
			l.hasFlag(QDocumentLine::Hidden)
		||
			l.hasFlag(QDocumentLine::CollapsedBlockStart)
		||
			l.hasFlag(QDocumentLine::CollapsedBlockEnd)
		;
	
	if ( c.hasSelection() && !prot )
	{
		l = c.anchorLine();
		prot = 
				l.hasFlag(QDocumentLine::Hidden)
			||
				l.hasFlag(QDocumentLine::CollapsedBlockStart)
			||
				l.hasFlag(QDocumentLine::CollapsedBlockEnd)
			;
	}
	
	/*
	if ( prot )
		qDebug("line %i is protected (%i, %i, %i)", c.lineNumber(), 
			l.hasFlag(QDocumentLine::Hidden),
			l.hasFlag(QDocumentLine::CollapsedBlockStart),
			l.hasFlag(QDocumentLine::CollapsedBlockEnd)
		);
	*/
	
	return prot;
}

/*!
	\internal
*/
void QEditor::keyPressEvent(QKeyEvent *e)
{
	if ( m_binding )
	{
		if ( m_binding->keyPressEvent(e, this) )
			return;
	}
	
	forever
	{
		bool leave = false;
		
		// try mirrors bindings first
		if ( (e->modifiers() & Qt::AltModifier) && (e->modifiers() & Qt::ControlModifier) )
		{
			int ln = - 1;
			QDocumentLine l;
			
			if ( e->key() == Qt::Key_Up )
			{
				ln = m_cursor.lineNumber();
				
				if ( m_mirrors.count() )
					ln = qMin(ln, min(m_mirrors));
				
				//qDebug("first %i", ln);
				
				l = m_doc->line(--ln);
			} else if ( e->key() == Qt::Key_Down ) {
				ln = m_cursor.lineNumber();
				
				if ( m_mirrors.count() )
					ln = qMax(ln, max(m_mirrors));
				
				l = m_doc->line(++ln);
			}
			
			if ( l.isValid() )
			{
				addCursorMirror(QDocumentCursor(m_doc, ln, m_cursor.anchorColumnNumber()));
				repaintCursor();
				emitCursorPositionChanged();
				
				break;
			}
		}
		
		selectionChange();
		
		// placeholders handling
		bool bHandled = false;
		
		if ( m_placeHolders.count() && e->modifiers() == Qt::ControlModifier )
		{
			if ( e->key() == Qt::Key_Up || e->key() == Qt::Key_Left )
			{
				bHandled = true;
				previousPlaceHolder();
			} else if ( e->key() == Qt::Key_Down || e->key() == Qt::Key_Right ) {
				bHandled = true;
				nextPlaceHolder();
			}
		}
		
		// regular moves
		if ( !bHandled )
		{
			if ( moveKeyEvent(m_cursor, e, &leave) )
			{
				e->accept();
				
				//setFlag(CursorOn, true);
				//ensureCursorVisible();
				
				if ( !leave )
					for ( int i = 0; !leave && (i < m_mirrors.count()); ++i )
						moveKeyEvent(m_mirrors[i], e, &leave);
				
				if ( leave && m_mirrors.count() )
				{
					for ( int i = 0; i < m_mirrors.count(); ++i )
					{
						m_mirrors[i].setAutoUpdated(false);
					}
					
					clearCursorMirrors();
					viewport()->update();
				} else {
					repaintCursor();
					selectionChange();
				}
				
				bHandled = true;
			}
		}
		
		bool bOk = true;
		if ( !bHandled )
		{
			int offset = 0;
			bool pke = isProcessingKeyEvent(e, &offset);
			bool prot = protectedCursor(m_cursor);
			
			foreach ( const QDocumentCursor& c, m_mirrors )
				prot |= protectedCursor(c);
			
			if ( !pke || prot )
			{
				bHandled = false;
			} else {
				
				// clear matches to avoid offsetting and subsequent remanence of matches
				if ( m_definition )
					m_definition->clearMatches(m_doc);
				
				if ( m_mirrors.isEmpty() )
				{
					bHandled = processCursor(m_cursor, e, bOk);
					
					// this signal is NOT emitted when cursor mirrors are used ON PURPOSE
					// as it is the "standard" entry point for code completion, which cannot
					// work properly with cursor mirrors (art least not always and not simply)
					if ( bHandled )
						emit textEdited(e);
				} else {
					// begin macro [synchronization of undo/redo ops]
					m_doc->beginMacro();
					
					processCursor(m_cursor, e, bOk);
					
					for ( int i = 0; bOk && (i < m_mirrors.count()); ++i )
					{
						bHandled = processCursor(m_mirrors[i], e, bOk);
					}
					
					// end macro
					m_doc->endMacro();
				}
			}
		}
		
		if ( !bHandled )
		{
			QAbstractScrollArea::keyPressEvent(e);
			
			break;
		}
		
		e->accept();
		emitCursorPositionChanged();
		setFlag(CursorOn, true);
		ensureCursorVisible();
		repaintCursor();
		selectionChange();
		break;
	}
	
	if ( m_binding )
		m_binding->postKeyPressEvent(e, this);
}

/*!
	\internal
*/
void QEditor::inputMethodEvent(QInputMethodEvent* e)
{
	if ( m_binding )
	{
		if ( m_binding->inputMethodEvent(e, this) )
			return;
	}
	
	/*
	if ( m_doc->readOnly() )
	{
		e->ignore();
		return;
	}
	*/
	
	m_cursor.beginEditBlock();
	
	if ( e->commitString().count() )
		m_cursor.insertText(e->commitString());
	
	m_cursor.endEditBlock();

	if ( m_binding )
		m_binding->postInputMethodEvent(e, this);
}

/*!
	\internal
*/
void QEditor::mouseMoveEvent(QMouseEvent *e)
{
	if ( m_binding )
	{
		if ( m_binding->mouseMoveEvent(e, this) )
			return;
	}
	
	forever
	{
		if ( !(e->buttons() & Qt::LeftButton) )
			break;
		
		if ( !( flag(MousePressed) || m_doubleClick.hasSelection() ) )
			break;
		
		if ( flag(MaybeDrag) )
		{
			m_drag.stop();
			
			if (	(e->globalPos() - m_dragPoint).manhattanLength() >
					QApplication::startDragDistance()
				)
				startDrag();
			
			//emit clearAutoCloseStack();
			break;
		}
		
		repaintCursor();
		selectionChange();
		
		const QPoint mousePos = mapToContents(e->pos());
		
		if ( m_scroll.isActive() )
		{
			if ( viewport()->rect().contains(e->pos()) )
				m_scroll.stop();
		} else {
			if ( !viewport()->rect().contains(e->pos()) )
				m_scroll.start(100, this);
		}
		
		QDocumentCursor newCursor = cursorForPosition(mousePos);
		
		if ( newCursor.isNull() )
			break;
		
		if ( flag(Persistent) )
		{
			//persistent.setPosition(newCursorPos, QTextCursor::KeepAnchor);
		} else if ( e->modifiers() & Qt::ControlModifier ) {
			
			// get column number for column selection
			int org = m_cursor.anchorColumnNumber();
			int dst = newCursor.columnNumber();
			// TODO : adapt to line wrapping...
			
			clearCursorMirrors();
			//m_cursor.clearSelection();
			int min = qMin(m_cursor.lineNumber(), newCursor.lineNumber());
			int max = qMax(m_cursor.lineNumber(), newCursor.lineNumber());
			
			if ( min != max )
			{
				for ( int l = min; l <= max; ++l )
				{
					if ( l != m_cursor.lineNumber() )
						addCursorMirror(QDocumentCursor(m_doc, l, org));
					
				}
				
				if ( e->modifiers() & Qt::ShiftModifier )
				{
					m_cursor.setColumnNumber(dst, QDocumentCursor::KeepAnchor);
					
					for ( int i = 0; i < m_mirrors.count(); ++i )
						m_mirrors[i].setColumnNumber(dst, QDocumentCursor::KeepAnchor);
				}
			} else {
				m_cursor.setSelectionBoundary(newCursor);
			}
		} else {
			m_cursor.setSelectionBoundary(newCursor);
			//setFlag(FoldedCursor, isCollapsed());
		}
		
		selectionChange(true);
		ensureCursorVisible();
		//emit clearAutoCloseStack();
		emitCursorPositionChanged();
		
		repaintCursor();
		break;
	}

	if ( m_binding )
		m_binding->postMouseMoveEvent(e, this);
}

/*!
	\internal
*/
void QEditor::mousePressEvent(QMouseEvent *e)
{
	if ( m_binding )
	{
		if ( m_binding->mousePressEvent(e, this) )
			return;
	}
	
	forever
	{
		if ( !(e->buttons() & Qt::LeftButton) )
			break;
		
		QPoint p = mapToContents(e->pos());
		
		setFlag(MousePressed, true);
		setFlag(MaybeDrag, false);
		
		repaintCursor();
		selectionChange();
		
		if ( m_click.isActive() &&
			(( e->globalPos() - m_clickPoint).manhattanLength() <
				QApplication::startDragDistance() ))
		{
	#if defined(Q_WS_MAC)
			m_cursor.select(QDocumentCursor::LineUnderCursor);
			m_doubleClick = m_cursor;
	#else
			m_cursor.movePosition(1, QDocumentCursor::StartOfBlock);
			m_cursor.movePosition(1, QDocumentCursor::EndOfBlock, QDocumentCursor::KeepAnchor);
	#endif
			
			m_click.stop();
		} else {
			QDocumentCursor cursor = cursorForPosition(p);
			
			if ( cursor.isNull() )
				break;
			
			if ( e->modifiers() == Qt::ShiftModifier )
			{
				clearCursorMirrors();
				m_cursor.setSelectionBoundary(cursor);
			} else if ( e->modifiers() & Qt::ControlModifier && ((e->modifiers() & Qt::ShiftModifier) || (e->modifiers() & Qt::AltModifier)) ) {
				//m_mirrors << cursor;
				if ( e->modifiers() & Qt::ShiftModifier )
				{
					// get column number for column selection
					int org = m_cursor.anchorColumnNumber();
					int dst = cursor.columnNumber();
					// TODO : fix and adapt to line wrapping...
					
					clearCursorMirrors();
					//m_cursor.clearSelection();
					int min = qMin(m_cursor.lineNumber(), cursor.lineNumber());
					int max = qMax(m_cursor.lineNumber(), cursor.lineNumber());
					
					if ( min != max )
					{
						for ( int l = min; l <= max; ++l )
						{
							if ( l != m_cursor.lineNumber() )
								addCursorMirror(QDocumentCursor(m_doc, l, org));
							
						}
						
						if ( e->modifiers() & Qt::ShiftModifier )
						{
							m_cursor.setColumnNumber(dst, QDocumentCursor::KeepAnchor);
							
							for ( int i = 0; i < m_mirrors.count(); ++i )
								m_mirrors[i].setColumnNumber(dst, QDocumentCursor::KeepAnchor);
						}
					} else {
						m_cursor.setSelectionBoundary(cursor);
					}
				} else if ( (e->modifiers() & Qt::AltModifier) ) {
					addCursorMirror(cursor);
				}
			} else {
				
				const QDocumentCursor& cur = m_cursor;
				
				if ( m_cursor.hasSelection() )
				{
					bool inSel = cur.isWithinSelection(cursor);
					
					if ( !inSel )
					{
						foreach ( const QDocumentCursor& m, m_mirrors )
						{
							inSel = m.isWithinSelection(cursor);
							
							if ( inSel )
								break;
						}
					}
					
					if ( inSel )
					{
						setFlag(MaybeDrag, true);
						
						m_dragPoint = e->globalPos();
						m_drag.start(QApplication::startDragTime(), this);
						
						break;
					}
				}
				
	// 			m_cursor = cursor;
	// 			clearCursorMirrors();
				m_doubleClick = QDocumentCursor();
				setCursor(cursor);
				break;
			}
		}
		
		ensureCursorVisible();
		//emit clearAutoCloseStack();
		emitCursorPositionChanged();
		repaintCursor();
		selectionChange();
		break;
	}

	if ( m_binding )
		m_binding->postMousePressEvent(e, this);
}

/*!
	\internal
*/
void QEditor::mouseReleaseEvent(QMouseEvent *e)
{
	if ( m_binding )
	{
		if ( m_binding->mouseReleaseEvent(e, this) )
			return;
	}
	
	m_scroll.stop();
	
	repaintCursor();
	selectionChange();
	
	if ( flag(MaybeDrag) )
	{
		setFlag(MousePressed, false);
		setCursorPosition(mapToContents(e->pos()));
		
		if ( flag(Persistent) )
		{
			//persistent.clearSelection();
		} else {
			m_cursor.clearSelection();
			//setFlag(FoldedCursor, isCollapsed());
		}
	}
	
	if ( flag(MousePressed) )
	{
		setFlag(MousePressed, false);
		
		setClipboardSelection();
	} else if (	e->button() == Qt::MidButton
				&& QApplication::clipboard()->supportsSelection()) {
		setCursorPosition(mapToContents(e->pos()));
		//setCursorPosition(viewport()->mapFromGlobal(e->globalPos()));
		
		const QMimeData *md = QApplication::clipboard()
								->mimeData(QClipboard::Selection);
		
		if ( md )
			insertFromMimeData(md);
	}
	
	repaintCursor();
	
	if ( m_drag.isActive() )
		m_drag.stop();
	
	selectionChange();

	if ( m_binding )
		m_binding->postMouseReleaseEvent(e, this);
}

/*!
	\internal
*/
void QEditor::mouseDoubleClickEvent(QMouseEvent *e)
{
	if ( m_binding )
	{
		if ( m_binding->mouseDoubleClickEvent(e, this) )
			return;
	}
	
	forever
	{
		if ( e->button() != Qt::LeftButton )
		{
			e->ignore();
			break;
		}
		
		setFlag(MaybeDrag, false);
		
		repaintCursor();
		selectionChange();
		clearCursorMirrors();
		setCursorPosition(mapToContents(e->pos()));
		
		//setFlag(FoldedCursor, isCollapsed());
		
		if ( m_cursor.isValid() )
		{
			if ( flag(Persistent) )
			{
				//persistent.select(QDocumentCursor::WordUnderCursor);
			} else {
				m_cursor.select(QDocumentCursor::WordUnderCursor);
			}
			
			setClipboardSelection();
			//emit clearAutoCloseStack();
			emitCursorPositionChanged();
			
			repaintCursor();
			selectionChange();
		} else {
			//qDebug("invalid cursor");
		}
		
		m_doubleClick = m_cursor;
		
		m_clickPoint = e->globalPos();
		m_click.start(qApp->doubleClickInterval(), this);
		break;
	}

	if ( m_binding )
		m_binding->postMouseDoubleClickEvent(e, this);
}

/*!
	\internal
*/
void QEditor::dragEnterEvent(QDragEnterEvent *e)
{
	if (
			e
		&&
			e->mimeData()
		&&
			(
				e->mimeData()->hasFormat("text/plain")
			||
				e->mimeData()->hasFormat("text/html")
			)
		&&
			!e->mimeData()->hasFormat("text/uri-list")
		)
		e->acceptProposedAction();
	else
		return;
	
	m_dragAndDrop = QDocumentCursor();
}

/*!
	\internal
*/
void QEditor::dragLeaveEvent(QDragLeaveEvent *)
{
	const QRect crect = cursorRect(m_dragAndDrop);
	m_dragAndDrop = QDocumentCursor();
	
	if ( crect.isValid() )
		viewport()->update(crect);
	
}

/*!
	\internal
*/
void QEditor::dragMoveEvent(QDragMoveEvent *e)
{
	if (
			e
		&&
			e->mimeData()
		&&
			(
				e->mimeData()->hasFormat("text/plain")
			||
				e->mimeData()->hasFormat("text/html")
			)
		&&
			!e->mimeData()->hasFormat("text/uri-list")
		)
		e->acceptProposedAction();
	else
		return;
	
	QDocumentCursor c = cursorForPosition(mapToContents(e->pos()));
	
	if ( c.isValid() )
	{
		QRect crect = cursorRect(m_dragAndDrop);
		
		if ( crect.isValid() )
			viewport()->update(crect);
		
		m_dragAndDrop = c;
		
		crect = cursorRect(m_dragAndDrop);
		viewport()->update(crect);
	}
	
	//e->acceptProposedAction();
}

/*!
	\internal
*/
void QEditor::dropEvent(QDropEvent *e)
{
	m_dragAndDrop = QDocumentCursor();
	
	QDocumentCursor c(cursorForPosition(mapToContents(e->pos())));
	
	if ( (e->source() == this) && (m_cursor.isWithinSelection(c)) )
		return;
	
	if (
			e
		&&
			e->mimeData()
		&&
			(
				e->mimeData()->hasFormat("text/plain")
			||
				e->mimeData()->hasFormat("text/html")
			)
		&&
			!e->mimeData()->hasFormat("text/uri-list")
		&&
			!flag(FoldedCursor)
		)
	{
		e->acceptProposedAction();
	} else {
		return;
	}
	
	//repaintSelection();
	
	m_doc->beginMacro();
	//m_cursor.beginEditBlock();
	
	if (
			(e->dropAction() == Qt::MoveAction)
		&&
			(
				(e->source() == this)
			||
				(e->source() == viewport())
			)
		)
	{
		m_cursor.removeSelectedText();
		
		for ( int i = 0; i < m_mirrors.count(); ++i )
			m_mirrors[i].removeSelectedText();
		
	} else {
		//qDebug("action : %i", e->dropAction());
		m_cursor.clearSelection();
	}
	
	clearCursorMirrors();
	m_cursor.moveTo(cursorForPosition(mapToContents(e->pos())));
	insertFromMimeData(e->mimeData());
	//m_cursor.endEditBlock();
	
	m_doc->endMacro();
	
	selectionChange();
}

/*!
	\internal
*/
void QEditor::changeEvent(QEvent *e)
{
	QAbstractScrollArea::changeEvent(e);
	
	if (
			e->type() == QEvent::ApplicationFontChange
		||
			e->type() == QEvent::FontChange
		)
	{
		if ( !m_doc )
			return;
		
		m_doc->setFont(font());
		//setTabStop(iTab);
		
	}  else if ( e->type() == QEvent::ActivationChange ) {
		if ( !isActiveWindow() )
			m_scroll.stop();
	}
}

/*!
	\internal
*/
void QEditor::showEvent(QShowEvent *e)
{
	QAbstractScrollArea::showEvent(e);
	
	//ensureCursorVisible();
	
	// => moved to focusInEvent()
	//setFlag(CursorOn, true);
	//m_blink.start(QApplication::cursorFlashTime() / 2, this);
	
	QCodeEdit *ce = QCodeEdit::manager(this);
	
	if ( ce )
		ce->panelLayout()->update();
	
	if ( flag(LineWrap) )
	{
		m_doc->setWidthConstraint(wrapWidth());
	}
}

/*!
	\internal
	\brief Zoom in/out upon ctrl+wheel
*/
void QEditor::wheelEvent(QWheelEvent *e)
{
	if ( e->modifiers() & Qt::ControlModifier )
	{
		const int delta = e->delta();
		
		if ( delta > 0 )
			zoom(-1);
		else if ( delta < 0 )
			zoom(1);
		
		//viewport()->update();
		
		return;
	}
	
	QAbstractScrollArea::wheelEvent(e);
	updateMicroFocus();
	//viewport()->update();
}

/*!
	\internal
*/
void QEditor::resizeEvent(QResizeEvent *)
{
	const QSize viewportSize = viewport()->size();
	
	if ( flag(LineWrap) )
	{
		//qDebug("resize t (2) : wrapping to %i", viewport()->width());
		
		m_doc->setWidthConstraint(wrapWidth());
	} else {
		horizontalScrollBar()->setMaximum(qMax(0, m_doc->width() - viewportSize.width()));
		horizontalScrollBar()->setPageStep(viewportSize.width());
	}
	
	const int ls = m_doc->fontMetrics().lineSpacing();
	verticalScrollBar()->setMaximum(qMax(0, 1 + (m_doc->height() - viewportSize.height()) / ls));
	verticalScrollBar()->setPageStep(viewportSize.height() / ls);
	
	//qDebug("page step : %i", viewportSize.height() / ls);
	
	//if ( isCursorVisible() && flag(LineWrap) )
	//	ensureCursorVisible();
}

/*!
	\internal
*/
void QEditor::focusInEvent(QFocusEvent *e)
{
	setFlag(CursorOn, true);
	m_blink.start(QApplication::cursorFlashTime() / 2, this);
	//ensureCursorVisible();
	
	QAbstractScrollArea::focusInEvent(e);
}

/*!
	\internal
*/
void QEditor::focusOutEvent(QFocusEvent *e)
{
	setFlag(CursorOn, false);
	m_blink.stop();
	
	QAbstractScrollArea::focusOutEvent(e);
}

/*!
	\brief Context menu event
	
	All the (managed) actions added to the editor are showed in it by default.
*/
void QEditor::contextMenuEvent(QContextMenuEvent *e)
{
	if ( m_binding )
	{
		if ( m_binding->contextMenuEvent(e, this) )
		{
			return;
		}
	}

	if ( !pMenu )
	{
		e->ignore();
		return;
	}
	
	selectionChange();
	
	e->accept();
	
	pMenu->exec(e->globalPos());
}

/*!
	\brief Close event
	
	When build with qmdilib support (e.g in Edyuk) this check for
	modifications and a dialog pops up to offer various options
	(like saving, discarding or canceling)
*/
void QEditor::closeEvent(QCloseEvent *e)
{
	#ifdef _QMDI_
	bool bOK = true;
	
	if ( isContentModified() )
		bOK = server()->maybeSave(this);
	
	if ( bOK )
	{
		e->accept();
		notifyDeletion();
	} else {
		e->ignore();
	}
	#else
	QAbstractScrollArea::closeEvent(e);
	#endif
}

#ifndef _QMDI_
/*!
	\return Whether the document has been modified.
*/
bool QEditor::isContentModified() const
{
	return m_doc ? !m_doc->isClean() : false;
}
#endif

/*!
	\brief Notify that the content is clean (modifications undone or document saved)
	
	\note Don't mess with this. The document knows better.
*/
void QEditor::setContentClean(bool y)
{
	setContentModified(!y);
}

/*!
	\brief Notify that the content has been modified
	
	\note Don't mess with this. The document knows better.
*/
void QEditor::setContentModified(bool y)
{
	#ifdef _QMDI_
	qmdiClient::setContentModified(y);
	#endif
	
	setWindowModified(y);
	emit contentModified(y);
}

/*!
	\brief Changes the file name
	
	This method does not affect files on disk (no save/load/move occurs)
*/
void QEditor::setFileName(const QString& f)
{
	QString prev = fileName();
	
	if ( f == prev )
		return;
	
	/*
	QStringList l = m_watcher->files();
	
	if ( l.count() )
		m_watcher->removePaths(l);
	*/
	
	watcher()->removeWatch(QString(), this);
	
	#ifdef _QMDI_
	qmdiClient::setFileName(f);
	#else
	m_fileName = f;
	m_name = QFileInfo(f).fileName();
	#endif
	
	//if ( fileName().count() )
	//	m_watcher->addPath(fileName());
	
	if ( fileName().count() )
		watcher()->addWatch(fileName(), this);
	
	setTitle(name().count() ? name() : "untitled");
}

/*!
	\brief Set the title of the widget
	
	Take care of adding a "[*]" prefix so that document changes are visible
	on title bars.
*/
void QEditor::setTitle(const QString& title)
{
	QString s(title);
	
	if ( !s.contains("[*]") )
		s.prepend("[*]");
	
	setWindowTitle(s);
	emit titleChanged(title);
}

#ifndef _QMDI_
/*!
	\return The name of the file being edited (without its path)
*/
QString QEditor::name() const
{
	return m_name;
}

/*!
	\return The full filename of the file being edited
*/
QString QEditor::fileName() const
{
	return m_fileName;
}
#endif

/*!
	\brief Prevent tab key press to be considered as widget navigation
*/
bool QEditor::focusNextPrevChild(bool)
{
	// make sure we catch tabs :)
	
	return false;
}

/*!
	\brief Start a drag and drop operation using the current selection
*/
void QEditor::startDrag()
{
	setFlag(MousePressed, false);
	QMimeData *data = createMimeDataFromSelection();
	
	QDrag *drag = new QDrag(this);
	drag->setMimeData(data);
	
	Qt::DropActions actions = Qt::CopyAction | Qt::MoveAction;
	Qt::DropAction action = drag->start(actions);
	
	if ( (action == Qt::MoveAction) && (drag->target() != this) )
	{
		m_cursor.removeSelectedText();
		
		for ( int i = 0; i < m_mirrors.count(); ++i )
			m_mirrors[i].removeSelectedText();
	}
}

/*!
	\brief Handle cursor movements upon key event
*/
bool QEditor::moveKeyEvent(QDocumentCursor& cursor, QKeyEvent *e, bool *leave)
{
	QDocumentCursor::MoveMode mode = e->modifiers() & Qt::ShiftModifier
								? QDocumentCursor::KeepAnchor
								: QDocumentCursor::MoveAnchor;
	
	if ( flag(LineWrap) && flag(CursorJumpPastWrap) )
		mode |= QDocumentCursor::ThroughWrap;
	
	QDocumentCursor::MoveOperation op = QDocumentCursor::NoMove;
#ifdef Q_WS_MAC
	// There can be only one modifier (+ shift), but we also need to make sure
	// that we have a "move key" pressed before we reject it.
	bool twoModifiers
		= ((e->modifiers() & (Qt::ControlModifier | Qt::AltModifier))
			== (Qt::ControlModifier | Qt::AltModifier))
		|| ((e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier))
			== (Qt::ControlModifier | Qt::MetaModifier))
		|| ((e->modifiers() & (Qt::AltModifier | Qt::MetaModifier))
			== (Qt::AltModifier | Qt::MetaModifier));
#else
	if (e->modifiers() & (Qt::AltModifier |
		Qt::MetaModifier | Qt::KeypadModifier) )
	{
		e->ignore();
		if ( leave ) *leave = false;
		return false;
	}
#endif
	
	switch ( e->key() )
	{
#ifndef Q_WS_MAC  // Use the default Windows bindings.
        case Qt::Key_Up:
            op = QDocumentCursor::Up;
            break;
        case Qt::Key_Down:
            op = QDocumentCursor::Down;
            /*
            if (mode == QDocumentCursor::KeepAnchor) {
                QTextBlock block = cursor.block();
                QTextLine line = currentTextLine(cursor);
                if (!block.next().isValid()
                    && line.isValid()
                    && line.lineNumber() == block.layout()->lineCount() - 1)
                    op = QDocumentCursor::End;
            }
            */
            break;
        case Qt::Key_Left:
            op = e->modifiers() & Qt::ControlModifier
                 ? QDocumentCursor::WordLeft
                 : QDocumentCursor::Left;
            break;
        case Qt::Key_Right:
            op = e->modifiers() & Qt::ControlModifier
                 ? QDocumentCursor::WordRight
                 : QDocumentCursor::Right;
            break;
        case Qt::Key_Home:
            op = e->modifiers() & Qt::ControlModifier
                 ? QDocumentCursor::Start
                 : QDocumentCursor::StartOfLine;
            break;
        case Qt::Key_End:
            op = e->modifiers() & Qt::ControlModifier
                 ? QDocumentCursor::End
                 : QDocumentCursor::EndOfLine;
            break;
#else
/*
	Except for pageup and pagedown, Mac OS X has very different behavior, we
	don't do it all, but here's the breakdown:
	
	Shift still works as an anchor, but only one of the other keys can be dow
	Ctrl (Command), Alt (Option), or Meta (Control).
	
	Command/Control + Left/Right -- Move to left or right of the line
					+ Up/Down -- Move to top bottom of the file.
					(Control doesn't move the cursor)
	
	Option	+ Left/Right -- Move one word Left/right.
			+ Up/Down  -- Begin/End of Paragraph.
	
	Home/End Top/Bottom of file. (usually don't move the cursor, but will select)
*/
        case Qt::Key_Up:
            if (twoModifiers) {
                QApplication::beep();
                if ( leave ) *leave = false;
                return true;
            } else {
                if (e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier))
                    op = QDocumentCursor::Start;
                else if (e->modifiers() & Qt::AltModifier)
                    op = QDocumentCursor::StartOfBlock;
                else
                    op = QDocumentCursor::Up;
            }
            break;
        case Qt::Key_Down:
            if (twoModifiers) {
                QApplication::beep();
                if ( leave ) *leave = false;
                return true;
            } else {
                if (e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier))
                {
                    op = QDocumentCursor::End;
                } else if (e->modifiers() & Qt::AltModifier) {
                    op = QDocumentCursor::EndOfBlock;
                } else {
                    op = QDocumentCursor::Down;
                    /*
                    if (mode == QDocumentCursor::KeepAnchor) {
                        QTextBlock block = cursor.block();
                        QTextLine line = currentTextLine(cursor);
                        if (!block.next().isValid()
                            && line.isValid()
                            && line.lineNumber() ==
                                block.layout()->lineCount() - 1)
                            op = QDocumentCursor::End;
                    }
                    */
                }
            }
            break;
        case Qt::Key_Left:
            if (twoModifiers) {
                QApplication::beep();
                if ( leave ) *leave = false;
                return true;
            } else {
                if (e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier))
                    op = QDocumentCursor::StartOfLine;
                else if (e->modifiers() & Qt::AltModifier)
                    op = QDocumentCursor::WordLeft;
                else
                    op = QDocumentCursor::Left;
            }
            break;
        case Qt::Key_Right:
            if ( twoModifiers )
            {
                QApplication::beep();
                if ( leave ) *leave = false;
                return true;
            } else {
                if (e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier))
                    op = QDocumentCursor::EndOfLine;
                else if (e->modifiers() & Qt::AltModifier)
                    op = QDocumentCursor::WordRight;
                else
                    op = QDocumentCursor::Right;
            }
            break;
        case Qt::Key_Home:
            if (e->modifiers() & (Qt::ControlModifier |
                                  Qt::MetaModifier | Qt::AltModifier) )
            {
                QApplication::beep();
                if ( leave ) *leave = false;
                return true;
            } else {
                op = QDocumentCursor::Start;
            }
            break;
        case Qt::Key_End:
            if (e->modifiers() & (Qt::ControlModifier |
                                  Qt::MetaModifier | Qt::AltModifier))
            {
                QApplication::beep();
                if ( leave ) *leave = false;
                return true;
            } else {
                op = QDocumentCursor::End;
            }
            break;
#endif
		case Qt::Key_PageDown:
			if ( leave ) *leave = true;
			pageDown(mode);
			return true;
			
		case Qt::Key_PageUp:
			if ( leave ) *leave = true;
			pageUp(mode);
			return true;
			
		case Qt::Key_Insert :
			if ( leave ) *leave = false;
			setFlag(Overwrite, !flag(Overwrite));
			
			// hack to make sure status panel gets updated...
			// TODO : emit signals on flag change?
			emitCursorPositionChanged();
			return false;
			
		default:
			return false;
	}
	
	QDocumentLine prev = cursor.line();
	int prevcol = cursor.columnNumber();
	
	//const bool moved = 
	cursor.movePosition(1, op, mode);
	
	if ( prev != cursor.line() )
	{
		if ( m_curPlaceHolder >= 0 && m_curPlaceHolder < m_placeHolders.count() )
		{
			// allow mirror movement out of line while in placeholder
			PlaceHolder& ph = m_placeHolders[m_curPlaceHolder];
			if ( ph.cursor.isWithinSelection(cursor) )
				return true;
			for ( int i = 0; i < ph.mirrors.count(); ++i )
				if ( ph.mirrors.at(i).isWithinSelection(cursor) )
					return true;
		}
		//moved = true;
		if ( leave ) *leave = true;
		m_curPlaceHolder = -1;
	}
	
	return true;
}

/*!
	\brief Go up by one page
	
	\note This method clears all cursor mirrors and suspend placeholder edition.
*/
void QEditor::pageUp(QDocumentCursor::MoveMode moveMode)
{
	clearCursorMirrors();
	m_curPlaceHolder = -1;
	
	if ( m_cursor.atStart() )
		return;
	
	int n = viewport()->height() / QDocument::fontMetrics().lineSpacing();
	
	repaintCursor();
	m_cursor.movePosition(n, QDocumentCursor::Up, moveMode);
	
	ensureCursorVisible();
	emitCursorPositionChanged();
	//updateMicroFocus();
}

/*!
	\brief Go down by one page
	
	\note This method clears all cursor mirrors.
*/
void QEditor::pageDown(QDocumentCursor::MoveMode moveMode)
{
	clearCursorMirrors();
	m_curPlaceHolder = -1;
	
	if ( m_cursor.atEnd() )
		return;
	
	int n = viewport()->height() / QDocument::fontMetrics().lineSpacing();
	
	repaintCursor();
	m_cursor.movePosition(n, QDocumentCursor::Down, moveMode);
	
	ensureCursorVisible();
	emitCursorPositionChanged();
}

/*!
	\brief Determine whether a given key event is an editing operation
*/
bool QEditor::isProcessingKeyEvent(QKeyEvent *e, int *offset)
{
	if ( flag(FoldedCursor) )
		return false;
	
	switch ( e->key() )
	{
		case Qt::Key_Backspace :
			//--*offset;
			break;
			
		case Qt::Key_Delete :
			//--*offset;
			break;
			
		case Qt::Key_Enter :
		case Qt::Key_Return :
			if ( offset )
				++*offset;
			break;
			
		default :
		{
			QString text = e->text();
			
			if ( text.isEmpty() || !(text.at(0).isPrint() || (text.at(0) == '\t')) )
				return false;
			
			//if ( offset )
			//	*offset += text.length();
			
			break;
		}
	}
	
	return true;
}

/*!
	\internal
	\brief Process a key event for a given cursor
	
	This method only take care of editing operations, not movements.
*/
bool QEditor::processCursor(QDocumentCursor& c, QKeyEvent *e, bool& b)
{
	if ( !b )
		return false;
	
	bool hasSelection = c.hasSelection();
	
	switch ( e->key() )
	{
		case Qt::Key_Backspace :
			if ( flag(FoldedCursor) )
				return false;
			
			if ( hasSelection )
				c.removeSelectedText();
			else
				c.deletePreviousChar();
			
			break;
			
		case Qt::Key_Delete :
			if ( flag(FoldedCursor) )
				return false;
			
			if ( hasSelection )
			
				c.removeSelectedText();
			else
				c.deleteChar();
			
			//emit clearAutoCloseStack();
			break;
			
		case Qt::Key_Enter :
		case Qt::Key_Return :
		{
			if ( flag(FoldedCursor) )
				return false;
			
			c.beginEditBlock();
			
			if ( hasSelection )
				c.removeSelectedText();
			else if ( flag(Overwrite) )
				c.deleteChar();
			
			QString indent;
			
			if ( flag(AutoIndent) && (m_curPlaceHolder == -1) )
			{
				if ( m_definition )
				{
					indent = m_definition->indent(c);
				} else {
					// default : keep leading ws from previous line...
					QDocumentLine l = c.line();
					const int idx = qMin(l.firstChar(), c.columnNumber());
					
					indent = l.text();
					
					if ( idx != -1 )
						indent.resize(idx);
					
				}
			}
			
			if ( indent.count() )
			{
				indent.prepend("\n");
				c.insertText(indent);
			} else {
				c.insertLine();
			}
			
			c.endEditBlock();
			
			break;
		}
			
		default :
		{
			QString text = e->text();
			
			if ( text.isEmpty() || !(text.at(0).isPrint() || (text.at(0) == '\t')) )
			{
				b = false;
				return false;
			}
			
			if ( flag(ReplaceTabs) )
			{
				text.replace("\t", QString(m_doc->tabStop(), ' '));
			}
			
			c.beginEditBlock();
			insertText(c, text);
			c.endEditBlock();
			
			break;
		}
	}
	
	selectionChange();
	
	return true;
}

/*!
	\brief Insert some text at a given cursor position
	
	This function is provided to keep indenting/outdenting working when editing
*/
void QEditor::insertText(QDocumentCursor& c, const QString& text)
{
	bool hasSelection = c.hasSelection();
	
	if ( hasSelection )
		c.removeSelectedText();
	
	QStringList lines = text.split('\n', QString::KeepEmptyParts);
	
	if (
			flag(AutoIndent)
		&&
			(m_curPlaceHolder == -1)
		&&
			c.columnNumber()
		&&
			m_definition
		&&
			m_definition->unindent(c, lines.at(0))
		)
	{
		int firstNS = 0;
		QString txt = c.line().text();
		
		while ( (firstNS < txt.length()) && txt.at(firstNS).isSpace() )
			++firstNS;
		
		const int off = c.columnNumber() - firstNS;
		
		if ( off > 0 )
			c.movePosition(off, QDocumentCursor::PreviousCharacter);
		
		//qDebug("%i spaces", firstNS);
		
		const int ts = m_doc->tabStop();
		
		do
		{
			--firstNS;
			c.movePosition(1, QDocumentCursor::Left, QDocumentCursor::KeepAnchor);
		} while ( QDocument::screenLength(txt.constData(), firstNS, ts) % ts );
		
		//qDebug("%i left => \"%s\"", firstNS, qPrintable(c.selectedText()));
		
		c.removeSelectedText();
		
		if ( off > 0 )
			c.movePosition(off, QDocumentCursor::NextCharacter);
		
	}
	
	if ( !hasSelection && flag(Overwrite) )
		c.deleteChar();
	
	if ( true ) //lines.count() == 1 )
	{
		c.insertText(text);
	} else {
		#if 0
		for ( int i = 0; i < lines.count(); ++i )
		{
			QString indent;
			// TODO : adjust indent based on indent around insertion spot
			/*
			if ( flag(AutoIndent) && (m_curPlaceHolder == -1) )
			{
				if ( m_definition )
				{
					indent = m_definition->indent(c);
				} else {
					// default : keep leading ws from previous line...
					QDocumentLine l = c.line();
					const int idx = l.firstChar();
					
					indent = l.text();
					
					if ( idx != -1 )
						indent.resize(idx);
					
				}
			}
			*/
			
			if ( indent.count() )
			{
				indent.prepend("\n");
				c.insertText(indent);
			} else {
				c.insertLine();
			}
			
			insertText(c, lines.at(i));
			//c.insertText(lines.at(i));
		}
		#endif
	}
}

/*!
	\brief Write some text at the current cursor position
	
	This function is provided to make editing operations easier
	from the outside and to keep them compatible with cursor
	mirrors.
*/
void QEditor::write(const QString& s)
{
	m_doc->beginMacro();
	
	insertText(m_cursor, s);
	
	for ( int i = 0; i < m_mirrors.count(); ++i )
		insertText(m_mirrors[i], s);
	
	m_doc->endMacro();
	
	emitCursorPositionChanged();
	setFlag(CursorOn, true);
	ensureCursorVisible();
	repaintCursor();
	selectionChange();
}

/*!
	\brief Zoom
	\param n relative zoom factor
	
	Zooming is achieved by changing the point size of the font as follow :
	
	fontPointSize += \a n
*/
void QEditor::zoom(int n)
{
	if ( !m_doc )
		return;
	
	QFont f = m_doc->font();
	f.setPointSize(qMax(1, f.pointSize() + n));
	m_doc->setFont(f);
}

/*!
	\brief Obtain the value of panel margins
	\param l left margin
	\param t top margin
	\param r right margin
	\param b bottom margin
*/
void QEditor::getPanelMargins(int *l, int *t, int *r, int *b) const
{
	m_margins.getCoords(l, t, r, b);
}

/*!
	\brief Change the viewport margins to make room for panels
	\param l left margin
	\param t top margin
	\param r right margin
	\param b bottom margin
*/
void QEditor::setPanelMargins(int l, int t, int r, int b)
{
	m_margins.setCoords(l, t, r, b);
	
	setViewportMargins(l, t, r, b);

	if ( flag(LineWrap) )
	{
		//qDebug("panel adjust : wrapping to %i", viewport()->width());
		m_doc->setWidthConstraint(wrapWidth());
	}
}

/*!
	\deprecated
	\brief Does not do anything anymore...
*/
void QEditor::selectionChange(bool force)
{
	return;
	// TODO : repaint only selection rect
	
	if ( false )//force )
	{
		//qDebug("repainting selection... [%i]", force);
		viewport()->update();
	} else if ( m_cursor.hasSelection() ) {
		viewport()->update(selectionRect());
	}
	
	m_selection = m_cursor.hasSelection();
}

/*!
	\brief Request repaint (using QWidget::update()) for the region occupied by the cursor
*/
void QEditor::repaintCursor()
{
	if ( m_mirrors.count() )
		viewport()->update();
	
	QRect r = cursorRect();
	
	if ( m_crect != r )
	{
		viewport()->update(m_crect.translated(horizontalOffset(), 0));
		m_crect = r;
		viewport()->update(m_crect.translated(horizontalOffset(), 0));
	} else {
		viewport()->update(m_crect.translated(horizontalOffset(), 0));
	}
}

/*!
	\return whether the cursor is currently visible
*/
bool QEditor::isCursorVisible() const
{
	QPoint pos = m_cursor.documentPosition();
	
	const QRect cursor(pos.x(), pos.y(), 1, QDocument::fontMetrics().lineSpacing());
	const QRect display(horizontalOffset(), verticalOffset(), viewport()->width(), viewport()->height());
	
	//qDebug() << pos << " belongs to " << display << " ?";
	
	return display.contains(pos); //cursor);
}

/*!
	\brief Ensure that the current cursor is visible
*/
void QEditor::ensureCursorVisible()
{
	QPoint pos = m_cursor.documentPosition();
	
	const int ls = QDocument::fontMetrics().lineSpacing();
	
	int ypos = pos.y(),
		yval = verticalOffset(),
		ylen = viewport()->height(),
		yend = ypos + ls;
	
	if ( ypos < yval )
		verticalScrollBar()->setValue(ypos / ls);
	else if ( yend > (yval + ylen) )
		verticalScrollBar()->setValue(1 + (yend - ylen) / ls);
	
	int xval = horizontalOffset(),
		xlen = viewport()->width(),
		xpos = pos.x();
	
	if ( xpos < xval )
	{
		//qDebug("scroll leftward");
		horizontalScrollBar()->setValue(qMax(0, xpos - 4));
	} else if ( xpos > (xval + xlen - 4) ) {
		//qDebug("scroll rightward : %i", xpos - xlen + 4);
		horizontalScrollBar()
			->setValue(qMax(horizontalScrollBar()->value(), xpos - xlen + 4));
	}
}

/*!
	\brief ensure that a given line is visible by updating scrollbars if needed
*/
void QEditor::ensureVisible(int line)
{
	if ( !m_doc )
		return;
	
	const int ls = QDocument::fontMetrics().lineSpacing();
	int ypos = m_doc->y(line),
		yval = verticalOffset(),
		ylen = viewport()->height(),
		yend = ypos + ls;
	
	if ( ypos < yval )
		verticalScrollBar()->setValue(ypos / ls);
	else if ( yend > (yval + ylen) )
		verticalScrollBar()->setValue(1 + (yend - ylen) / ls);
	
}

/*!
	\brief Ensure that a given rect is visible by updating scrollbars if needed
*/
void QEditor::ensureVisible(const QRect &rect)
{
	if ( !m_doc )
		return;
	
	const int ls = QDocument::fontMetrics().lineSpacing();
	int ypos = rect.y(),
		yval = verticalOffset(),
		ylen = viewport()->height(),
		yend = ypos + rect.height();
	
	if ( ypos < yval )
		verticalScrollBar()->setValue(ypos / ls);
	else if ( yend > (yval + ylen) )
		verticalScrollBar()->setValue(1 + (yend - ylen) / ls);
	
	//verticalScrollBar()->setValue(rect.y());
}

/*!
	\return the rectangle occupied by the current cursor
	
	This will either return a cursorRect for the current cursor or
	the selectionRect() if the cursor has a selection.
	
	The cursor position, which would be the top left corner of the actual
	rectangle occupied by the cursor can be obtained using QDocumentCursor::documentPosition()
	
	The behavior of this method may surprise newcomers but it is actually quite sensible
	as this rectangle is mainly used to specify the update rect of the widget and the whole
	line needs to be updated to properly update the line background whenever the cursor move
	from a line to another.
*/
QRect QEditor::cursorRect() const
{
	return m_cursor.hasSelection() ? selectionRect() : cursorRect(m_cursor);
}

/*!
	\return the rectangle occupied by the selection in viewport coordinates
	
	If the current cursor does not have a selection, its cursorRect() is returned.
	
	The returned rectangle will always be bigger than the actual selection has
	it is actually the union of all the rectangles occupied by all lines the selection
	spans over.
*/
QRect QEditor::selectionRect() const
{
	if ( !m_cursor.hasSelection() )
		return cursorRect(m_cursor);
	
	QDocumentSelection s = m_cursor.selection();
	
	if ( s.startLine == s.endLine )
		return cursorRect(m_cursor);
	
	int y = m_doc->y(s.startLine);
	QRect r = m_doc->lineRect(s.endLine);
	int height = r.y() + r.height() - y;
	
	r = QRect(0, y, viewport()->width(), height);
	r.translate(-horizontalOffset(), -verticalOffset());
	return r;
}

/*!
	\return the rectangle occupied by the given line, in viewport coordinates
	
	The width of the returned rectangle will always be the viewport width.
*/
QRect QEditor::lineRect(int line) const
{
	if ( !m_doc )
		return QRect();
	
	QRect r = m_doc->lineRect(line);
	r.setWidth(viewport()->width());
	r.translate(-horizontalOffset(), -verticalOffset());
	
	return r;
}

/*!
	\overload
	
	\note This function relies on QDocumentLine::lineNumber() so avoid
	it whenever possible as it is much slower than providing a line number
	directly.
*/
QRect QEditor::lineRect(const QDocumentLine& l) const
{
	//qFatal("bad practice...");
	
	if ( !m_doc )
		return QRect();
	
	QRect r = m_doc->lineRect(l);
	r.setWidth(viewport()->width());
	r.translate(-horizontalOffset(), -verticalOffset());
	
	return r;
}

/*!
	\return The line rect of the given cursor
*/
QRect QEditor::cursorRect(const QDocumentCursor& c) const
{
	return lineRect(c.lineNumber());
}
/*!
	\brief creates a valid QMimeData object depending on the selection
*/
QMimeData* QEditor::createMimeDataFromSelection() const
{
	QMimeData *d = new QMimeData;
	
	if ( !m_cursor.hasSelection() )
	{
		qWarning("Generated empty MIME data");
		return d;
	}
	
	if ( m_mirrors.isEmpty() )
	{
		d->setText(m_cursor.selectedText());
	} else {
		QString serialized = m_cursor.selectedText();
		
		foreach ( const QDocumentCursor& m, m_mirrors )
		{
			serialized += '\n';
			serialized += m.selectedText();
		}
		
		d->setText(serialized);
		d->setData("text/column-selection", serialized.toLocal8Bit());
	}
	
	//qDebug("generated selection from : \"%s\"", qPrintable(d->text()));
	
	return d;
}

/*!
	\brief Inserts the content of a QMimeData object at the cursor position
	
	\note Only plain text is supported... \see QMimeData::hasText()
*/
void QEditor::insertFromMimeData(const QMimeData *d)
{
	bool s = m_cursor.hasSelection();
	
	if ( d && m_cursor.isValid() && !d->hasFormat("text/uri-list") )
	{
		
		if ( d->hasFormat("text/column-selection") )
		{
			clearCursorMirrors();
			
			QStringList columns = QString::fromLocal8Bit(
										d->data("text/column-selection")
									).split('\n');
			
			m_doc->beginMacro();
			
			if ( s )
				m_cursor.removeSelectedText();
			
			int col = m_cursor.columnNumber();
			//m_cursor.insertText(columns.takeFirst());
			insertText(m_cursor, columns.takeFirst());
			QDocumentCursor c = m_cursor;
			
			while ( columns.count() )
			{
				// check for end of doc and add line if needed...
				c.setColumnNumber(c.line().length());
				
				if ( c.atEnd() )
					c.insertText("\n");
				else
					c.movePosition(1, QDocumentCursor::NextCharacter);
				
				// align
				c.setColumnNumber(qMin(col, c.line().length()));
				
				// copy content of clipboard
				//c.insertText(columns.takeFirst());
				insertText(c, columns.takeFirst());
				addCursorMirror(c);
			}
			
			m_doc->endMacro();
			
		} else {
			m_doc->beginMacro();
			
			//if ( s )
			//{
			//	m_cursor.removeSelectedText();
			//}
			
			QString txt;
			
			if ( d->hasFormat("text/plain") )
				txt = d->text();
			else if ( d->hasFormat("text/html") )
				txt = d->html();
			
			insertText(m_cursor, txt);
			
			for ( int i = 0; i < m_mirrors.count(); ++i )
			{
				insertText(m_mirrors[i], txt);
			}
			
			m_doc->endMacro();
		}
		
		ensureCursorVisible();
		setFlag(CursorOn, true);
		
		emitCursorPositionChanged();
	}
}

/*!
	\brief Removes all cursor mirrors
*/
void QEditor::clearCursorMirrors()
{
	m_curPlaceHolder = -1;
	repaintCursor();

	for ( int i = 0; i < m_mirrors.count(); ++i )
	{
		m_mirrors[i].setAutoUpdated(false);
	}
	
	m_mirrors.clear();
}

/*!
	\brief Add a cursor mirror
*/
void QEditor::addCursorMirror(const QDocumentCursor& c)
{
	if ( c.isNull() || (c == m_cursor) || m_mirrors.contains(c) )
		return;
	
	m_mirrors << c;
	
	// necessary for smooth mirroring
	m_mirrors.last().setSilent(true);
	m_mirrors.last().setAutoUpdated(true);
}

/*!
	\internal
	\brief Copy the selection to the clipboard
*/
void QEditor::setClipboardSelection()
{
	QClipboard *clipboard = QApplication::clipboard();
	
	if ( !clipboard->supportsSelection() )
		return;
	
	if ( //( flag(Persistent) && !persistent.hasSelection() ) ||
		( !flag(Persistent) && !m_cursor.hasSelection() ) )
		return;
	
	QMimeData *data = createMimeDataFromSelection();
	
	clipboard->setMimeData(data, QClipboard::Selection);
}

/*!
	\internal
	\brief Scroll contents
	
	Refer to QAbstractScrollArea doc for more info.
*/
void QEditor::scrollContentsBy(int dx, int dy)
{
	#ifdef Q_GL_EDITOR
	viewport()->update();
	#else
	const int ls = m_doc->fontMetrics().lineSpacing();
	viewport()->scroll(dx, dy * ls);
	#endif
}

/*!
	\internal
	\brief Workaround inconsistent width determination of viewport width
	accross platfroms when scrollbars are visible...
*/
int QEditor::wrapWidth() const
{
	#ifdef Q_WS_WIN
	//if ( verticalScrollBar()->isVisible() )
	//	return viewport()->width() - verticalScrollBar()->width();
	#endif
	return viewport()->width();
}

/*!
	\internal
	\brief Slot called whenever document width changes
	
	Horizontal scrollbar is updated here.
	
	\note ensureCursorVisible() is NOT called.
*/
void QEditor::documentWidthChanged(int newWidth)
{
	if ( flag(LineWrap) )
	{
		horizontalScrollBar()->setMaximum(0);
		return;
	}
	
	int nv = qMax(0, newWidth - wrapWidth());
	
	horizontalScrollBar()->setMaximum(nv);
	
	//ensureCursorVisible();
}

/*!
	\internal
	\brief Slot called whenever document height changes
	
	Vertical scrollbar is updated here (maximum is changed
	and value is modified if needed to ensure that the cursor is visible)
*/
void QEditor::documentHeightChanged(int newHeight)
{
	if ( flag(LineWrap) )
	{
		m_doc->setWidthConstraint(wrapWidth());
	}
	const int ls = m_doc->fontMetrics().lineSpacing();
	verticalScrollBar()->setMaximum(qMax(0, 1 + (newHeight - viewport()->height()) / ls));
	//ensureCursorVisible();
}

/*!
	\internal
	\brief Request paint event upon modification
	\param i first modified line
	\param n number of modified lines
*/
void QEditor::repaintContent(int i, int n)
{
	if ( !m_doc )
		return;
	
	#ifdef Q_GL_EDITOR
	viewport()->update();
	#else
	if ( n <= 0 )
	{
		viewport()->update();
	}
	
	QRect frect = m_doc->lineRect(i);
	
	const int yoff = verticalOffset() + viewport()->height();
	
	if ( frect.y() > yoff )
		return;
	
	if ( n == 1 )
	{
		frect.translate(0, -verticalOffset());
		//qDebug() << frect;
		viewport()->update(frect);
		return;
	}
	
	QRect lrect = m_doc->lineRect(i + n - 1);
	
	if ( (n > 0) && (lrect.y() + lrect.height()) < verticalOffset() )
		return;
	
	//qDebug("repainting %i lines starting from %ith one", n, i);
	
	//rect.setWidth(viewport()->width());
	//rect.setHeight(qMin(viewport()->height(), rect.height() * n));
	
	const int paintOffset = frect.y() - verticalOffset();
	const int paintHeight = lrect.y() + lrect.height() - frect.y();
	const int maxPaintHeight = viewport()->height() - paintOffset;
	
	QRect rect = QRect(
				frect.x(),
				paintOffset,
				viewport()->width(),
					(n <= 0)
				?
					maxPaintHeight
				:
					qMin(maxPaintHeight, paintHeight)
			);
	
	//qDebug() << rect;
	
	viewport()->update(rect);
	#endif
}

/*!
	\internal
	\brief Update function called upon editing action
	\param i First modified line
	\param n Number of modified lines
	
	If more than one line has been modified this function
	causes a repaint from the first visible line to the end
	of the viewport due to the way QAbstractScrollArea
	handles scrolling.
	
	\note This function used to update formatting but
	the highlighting has been moved to QDocument recently
*/
void QEditor::updateContent (int i, int n)
{
	if ( !m_doc )
		return;
	
	//qDebug("updating %i, %i", i, n);
	
	bool cont = n > 1;
	
	repaintContent(i, cont ? -1 : n);
}

/*!
	\internal
*/
void QEditor::markChanged(QDocumentLineHandle *l, int mark, bool on)
{
	emit markChanged(fileName(), l, mark, on);
}

/*! @} */
