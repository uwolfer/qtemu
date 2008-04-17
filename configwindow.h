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

#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

#include <QDialog>

class QLineEdit;
class QComboBox;
class QTextEdit;
class QCheckBox;

class ConfigWindow : public QDialog
{
    Q_OBJECT

public:
    ConfigWindow(const QString &myMachinesPath, int tabPosition, QWidget *parent = 0);

    QLineEdit *myMachinePathLineEdit;
    QComboBox *comboTabPosition;
    QComboBox *languagePosition;

private:
    QString myMachinesPath;
    QTextEdit *beforeStartExeTextEdit;
    QTextEdit *afterExitExeTextEdit;
    QLineEdit *commandLineEdit;
    QComboBox *comboIconTheme;

private slots:
    void setNewPath();
    void languageChange(int index);
    void loadSettings();
    void writeSettings();
};

#endif
