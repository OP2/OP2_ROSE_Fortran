/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qtgradientviewdialog.h"
#include "qtgradientmanager.h"
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

QtGradientViewDialog::QtGradientViewDialog(QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);
    m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    connect(m_ui.gradientView, SIGNAL(currentGradientChanged(const QString &)),
            this, SLOT(slotGradientSelected(const QString &)));
    connect(m_ui.gradientView, SIGNAL(gradientActivated(const QString &)),
            this, SLOT(slotGradientActivated(const QString &)));
}

void QtGradientViewDialog::setGradientManager(QtGradientManager *manager)
{
    m_ui.gradientView->setGradientManager(manager);
}

QGradient QtGradientViewDialog::getGradient(bool *ok, QtGradientManager *manager, QWidget *parent, const QString &caption)
{
    QtGradientViewDialog dlg(parent);
    dlg.setGradientManager(manager);
    dlg.setWindowTitle(caption);
    QGradient grad = QLinearGradient();
    const int res = dlg.exec();
    if (res == QDialog::Accepted)
        grad = dlg.m_ui.gradientView->gradientManager()->gradients().value(dlg.m_ui.gradientView->currentGradient());
    if (ok)
        *ok = res == QDialog::Accepted;
    return grad;
}

void QtGradientViewDialog::slotGradientSelected(const QString &id)
{
    m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!id.isEmpty());
}

void QtGradientViewDialog::slotGradientActivated(const QString &id)
{
    Q_UNUSED(id)
    accept();
}

#include "moc_qtgradientviewdialog.cxx"

QT_END_NAMESPACE
