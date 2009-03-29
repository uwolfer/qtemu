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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QMenu;
class QTabWidget;
class QPushButton;
class QWidget;
class QLabel;
class QStringList;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void createNew();
    void open();
    void configure();
    void start();
    void pause();
    void stop();
    void restart();
    void about();
    void help();
    void changeMachineState(int value = -1);

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createMainTab();
    void readSettings();
    void writeSettings();
    void loadFile(const QString &fileName);

    QTabWidget *tabWidget;
    QMenu *fileMenu;
    QMenu *powerMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    QToolBar *powerToolBar;
    QAction *newAct;
    QAction *openAct;
    QAction *confAct;
    QAction *exitAct;
    QAction *startAct;
    QAction *stopAct;
    QAction *restartAct;
    QAction *pauseAct;
    QAction *helpAct;
    QAction *aboutAct;
    QWidget *mainTabWidget;
    QLabel *mainTabLabel;
    QPushButton *newButton;
    QPushButton *openButton;

    QString myMachinesPath;
    QString iconTheme;

};

#endif
