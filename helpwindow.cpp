/****************************************************************************
**
** Copyright (C) 2006-2008 Urs Wolfer <uwolfer @ fwo.ch>
**
** This file is part of QtEmu.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU Library General Public License
** along with this library; see the file COPYING.LIB.  If not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
** Boston, MA 02110-1301, USA.
**
****************************************************************************/

#include "helpwindow.h"

#include <QFile>
#include <QCoreApplication>
#include <QMessageBox>
#include <QSettings>
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QTimer>
#include <QPushButton>
#include <QLocale>

HelpWindow::HelpWindow(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("QtEmu Help"));
    resize(850, 600);
    setSizeGripEnabled(true);

    QTextBrowser *textBrowser = new QTextBrowser(this);

    QUrl url = getHelpFile();
    if (!url.isEmpty())
        textBrowser->setSource(url);
    else //there is no help available
        QTimer::singleShot(0, this, SLOT(close()));

    textBrowser->scroll(0, 0);

    QPushButton *closeButton = new QPushButton(tr("Close"));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(closeButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(textBrowser);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);
}

QUrl HelpWindow::getHelpFile()
{
QUrl url = QUrl(getHelpLocation().toString() + "main.htm");
    if(url.isEmpty())
        QMessageBox::critical(this, tr("Help not found"),
                                tr("Help not found. It is probably not installed."));
return url;
}

QUrl HelpWindow::getHelpLocation()
{
    QSettings settings("QtEmu", "QtEmu");
    QString locale = settings.value("language", QString(QLocale::system().name())).toString();
    if (locale != "en")
    {
        //check for case when qtemu executable is in same dir (linux / win)
        QUrl testUrl = QUrl(QCoreApplication::applicationDirPath() + "/help/" + locale);
        if (QFile::exists(testUrl.toString()))
            return testUrl;
    
        //check for case when qtemu executable is in bin/ (installed on linux)
        testUrl = QUrl(QCoreApplication::applicationDirPath() + "/../share/qtemu/help/" + locale);
        if (QFile::exists(testUrl.toString()))
            return testUrl;
    }

    //check for case when qtemu executable is in same dir (linux / win)
    QUrl testUrl = QUrl(QCoreApplication::applicationDirPath() + "/help/");
    if (QFile::exists(testUrl.toString()))
        return testUrl;

    //check for case when qtemu executable is in bin/ (installed on linux)
    testUrl = QUrl(QCoreApplication::applicationDirPath() + "/../share/qtemu/help/");
    if (QFile::exists(testUrl.toString()))
        return testUrl;

    //qDebug(testUrl.toString().toLocal8Bit().constData());
    return QUrl();
}
