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

#include "mainwindow.h"

#include "machinetab.h"
#include "machinewizard.h"
#include "helpwindow.h"
#include "configwindow.h"
#include "config.h"
#include "machineprocess.h"

#include <QSettings>
#include <QTabWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QIcon>
#include <QAction>
#include <QCloseEvent>
#include <QFileDialog>
#include <QTimer>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QLabel>
#include <QStatusBar>
#include <QToolBar>
#include <QMessageBox>
#include <QToolButton>

MainWindow::MainWindow()
{
    QSettings settings("QtEmu", "QtEmu");
    iconTheme = settings.value("iconTheme", "oxygen").toString();

    setWindowTitle(tr("QtEmu"));
    setWindowIcon(QPixmap(":/images/" + iconTheme + "/qtemu.png"));

    tabWidget = new QTabWidget;
    tabWidget->setTabPosition(QTabWidget::West);
    setCentralWidget(tabWidget);

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    createMainTab();

    readSettings();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    int runningMachines = 0;
    int savingMachines = 0;
    for (int i = 1; i<(tabWidget->count());i++)
    {
       MachineTab *tab = static_cast<MachineTab *>(tabWidget->widget(i));
       if(tab->machineProcess->state() == MachineProcess::Running)
       {
           runningMachines++;
       }
       else if(tab->machineProcess->state() == MachineProcess::Saving)
       {
           savingMachines++;
       }
    }
    if (savingMachines != 0)
    {
        QMessageBox::critical(this, tr("Virtual Machine Saving State!"),
                              tr("You have virtual machines currently saving their state.<br />"
                                 "Quitting now would very likely damage your Virtual Machine!!"),
                              QMessageBox::Cancel);
        event->ignore();
        return;
    }

    if (runningMachines == 0 || QMessageBox::question(this, tr("Exit confirmation"),
                              tr("You have virtual machines currently running. Are you sure you want to quit?<br />"
                                 "Quitting QtEmu will leave your virtual machines running. QtEmu will<br />"
                                 "automatically reconnect to your virtual machines next time you run it."),
                              QMessageBox::Close | QMessageBox::Cancel, QMessageBox::Cancel)
      == QMessageBox::Close)
    {
        writeSettings();
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::createNew()
{
    QString machine = MachineWizard::newMachine(myMachinesPath, this);
    if (!machine.isEmpty())
        loadFile(machine);
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a virtual machine"),
                                                    myMachinesPath,
                                                    tr("QtEmu machines")+" (*.qte)");
    if (!fileName.isEmpty())
        loadFile(fileName);
}

void MainWindow::configure()
{
    ConfigWindow *config = new ConfigWindow(myMachinesPath, tabWidget->tabPosition(), this);
    if (config->exec() == QDialog::Accepted)
    {
        myMachinesPath = config->myMachinePathLineEdit->text();

        QTabWidget::TabPosition position;
        switch(config->comboTabPosition->currentIndex())
        {
            case 0: position = QTabWidget::North;
                    break;
            case 1: position = QTabWidget::South;
                    break;
            case 2: position = QTabWidget::West;
                    break;
            case 3: position = QTabWidget::East;
                    break;
            default: position = QTabWidget::West;
        }
        tabWidget->setTabPosition(position);
    }
}

void MainWindow::start()
{
    MachineTab *tab = qobject_cast<MachineTab *>(tabWidget->currentWidget());
    QPushButton *startButton = qobject_cast<QPushButton *>(tab->startButton);
    startButton->click();
}

void MainWindow::pause()
{
    MachineTab *tab = qobject_cast<MachineTab *>(tabWidget->currentWidget());
    QPushButton *pauseButton = qobject_cast<QPushButton *>(tab->pauseButton);
    pauseButton->click();
}

void MainWindow::stop()
{
    MachineTab *tab = qobject_cast<MachineTab *>(tabWidget->currentWidget());
    QPushButton *stopButton = qobject_cast<QPushButton *>(tab->stopButton);
    stopButton->click();
}

void MainWindow::restart()
{
    //stop();
    //QTimer::singleShot(500, this, SLOT(start()));
    MachineTab *tab = qobject_cast<MachineTab *>(tabWidget->currentWidget());
    tab->restart();
}

void MainWindow::about()
{
     QMessageBox::about(this, tr("About QtEmu"),
            tr("<h2>QtEmu</h2>Version %1<br>"
               "<b><i>QtEmu</i></b> is a graphical user interface for "
               "<a href=http://qemu.org>QEMU</a>.<br><br>Copyright &copy; "
               "2006-2009 Urs Wolfer <a href=mailto:uwolfer%2fwo.ch>uwolfer%2fwo.ch</a>.<br />"
               "Copyright &copy; 2008-2009 Ben Klopfenstein <a href=mailto:benklop%2gmail.com>benklop%2gmail.com</a>.<br />"
               "All rights reserved.<br><br>"
               "The program is provided AS IS with NO WARRANTY OF ANY KIND.<br><br>"
               "The icons have been taken from the KDE Crystal and Oxygen themes which are LGPL licensed.")
               .arg(VERSION).arg("@"));
}

void MainWindow::help()
{
    HelpWindow *help = new HelpWindow(this);
    help->show();
    help->raise();
    help->activateWindow();
}

void MainWindow::createActions()
{
//file actions
    newAct = new QAction(QIcon(":/images/" + iconTheme + "/new.png"), tr("&New Machine"), this);
    newAct->setShortcut(tr("Ctrl+N"));
    newAct->setStatusTip(tr("Create a new machine"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(createNew()));

    openAct = new QAction(QIcon(":/images/" + iconTheme + "/open.png"), tr("&Open Machine..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open an existing machine"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    confAct = new QAction(tr("Confi&gure"), this);
    confAct->setShortcut(tr("Ctrl+G"));
    confAct->setStatusTip(tr("Customize the application"));
    connect(confAct, SIGNAL(triggered()), this, SLOT(configure()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

//power actions
    startAct = new QAction(QIcon(":/images/" + iconTheme + "/start.png"), tr("&Start"), this);
    startAct->setShortcut(tr("Ctrl+S"));
    startAct->setStatusTip(tr("Start this virtual machine"));
    startAct->setEnabled(false);
    connect(startAct, SIGNAL(triggered()), this, SLOT(start()));

    stopAct = new QAction(QIcon(":/images/" + iconTheme + "/stop.png"), tr("S&top"), this);
    stopAct->setShortcut(tr("Ctrl+T"));
    stopAct->setStatusTip(tr("Kill this machine"));
    stopAct->setEnabled(false);
    connect(stopAct, SIGNAL(triggered()), this, SLOT(stop()));

    restartAct = new QAction(QIcon(":/images/" + iconTheme + "/restart.png"), tr("&Restart"), this);
    restartAct->setShortcut(tr("Ctrl+R"));
    restartAct->setStatusTip(tr("Restart this machine"));
    restartAct->setEnabled(false);
    connect(restartAct, SIGNAL(triggered()), this, SLOT(restart()));

    pauseAct = new QAction(QIcon(":/images/" + iconTheme + "/pause.png"), tr("&Pause"), this);
    pauseAct->setShortcut(tr("Ctrl+P"));
    pauseAct->setStatusTip(tr("Pause this machine"));
    pauseAct->setEnabled(false);
    connect(pauseAct, SIGNAL(triggered()), this, SLOT(pause()));

    helpAct = new QAction(tr("QtEmu &Help "), this);
    helpAct->setShortcut(tr("F1"));
    helpAct->setStatusTip(tr("Show Help"));
    connect(helpAct, SIGNAL(triggered()), this, SLOT(help()));

    aboutAct = new QAction(tr("&About QtEmu"), this);
    aboutAct->setStatusTip(tr("Show the About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addSeparator();
    fileMenu->addAction(confAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    menuBar()->addSeparator();

    powerMenu = menuBar()->addMenu(tr("&Power"));
    powerMenu->addAction(startAct);
    powerMenu->addAction(pauseAct);
    powerMenu->addAction(stopAct);
    powerMenu->addAction(restartAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(helpAct);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAct);
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);

    powerToolBar = addToolBar(tr("Power"));
    powerToolBar->addAction(startAct);
    powerToolBar->addAction(pauseAct);
    powerToolBar->addAction(stopAct);
    powerToolBar->addAction(restartAct);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createMainTab()
{
    mainTabWidget = new QWidget();

    mainTabLabel = new QLabel(mainTabWidget);
    mainTabLabel->setText(tr("<h1>QtEmu</h1>"
                             "QtEmu is a graphical user interface for QEMU. It has the ability "
                             "to run operating systems virtually in a window on native systems."));
    mainTabLabel->setWordWrap(true);

    newButton = new QPushButton(mainTabWidget);
    //newButton->setDefaultAction(newAct);
    newButton->setIconSize(QSize(32, 32));
    //newButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    //newButton->setAutoRaise(true);
    newButton->setIcon(QIcon(":/images/" + iconTheme + "/new.png"));
    newButton->setText(tr("Create a new virtual machine. A wizard will help you \n"
                          "prepare for a new operating system"));
    connect(newButton, SIGNAL(clicked()), this, SLOT(createNew()));

    openButton = new QPushButton(mainTabWidget);
    //openButton->setDefaultAction(openAct);
    openButton->setIconSize(QSize(32, 32));
    //openButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    //openButton->setAutoRaise(true);
    openButton->setIcon(QIcon(":/images/" + iconTheme + "/open.png"));
    openButton->setText(tr("Open an existing virtual machine"));
    connect(openButton, SIGNAL(clicked()), this, SLOT(open()));

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->setSpacing(20);
    buttonLayout->addWidget(mainTabLabel);
    buttonLayout->addWidget(newButton);
    buttonLayout->addWidget(openButton);
    buttonLayout->addStretch(1);
    mainTabWidget->setLayout(buttonLayout);

    tabWidget->addTab(mainTabWidget, QIcon(":/images/" + iconTheme + "/qtemu.png"), tr("Main"));
}

void MainWindow::readSettings()
{
    QSettings settings("QtEmu", "QtEmu");
    QPoint pos = settings.value("pos", QPoint(0, 50)).toPoint();
    QSize size = settings.value("size", QSize(350, 700)).toSize();
    resize(size);
    move(pos);

    myMachinesPath = settings.value("machinesPath", QString(QDir::homePath()+'/'+tr("MyMachines"))).toString();

    QTabWidget::TabPosition position;
    switch(settings.value("tabPosition", 2).toInt())
    {
        case 0: position = QTabWidget::North;
                break;
        case 1: position = QTabWidget::South;
                break;
        case 2: position = QTabWidget::West;
                break;
        case 3: position = QTabWidget::East;
                break;
        default: position = QTabWidget::West;
    }
    tabWidget->setTabPosition(position);

    int countMachines = settings.beginReadArray("machines");
    for (int i = 0; i < countMachines; ++i)
    {
        settings.setArrayIndex(i);
        loadFile(settings.value("path").toString());
    }
    settings.endArray();

    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(changeMachineState(int)));

    // workaround: check every second if user has finished machine with close button
    QTimer *checkTimer = new QTimer(this);
    connect(checkTimer, SIGNAL(timeout()), this, SLOT(changeMachineState()));
    checkTimer->start(1000);

    tabWidget->setCurrentIndex(settings.value("activeTab", 0).toInt());
}

void MainWindow::writeSettings()
{
    QSettings settings("QtEmu", "QtEmu");
    settings.setValue("pos", pos());
    settings.setValue("size", size());

    settings.setValue("machinesPath", myMachinesPath);

    settings.setValue("tabPosition", tabWidget->tabPosition());

    settings.beginWriteArray("machines");
    for (int i = 1; i < tabWidget->count(); ++i) //do not start with the main tab
    {
        settings.setArrayIndex(i-1);
        settings.setValue("path", tabWidget->tabToolTip(i));
    }
    settings.endArray();

    settings.setValue("activeTab", tabWidget->currentIndex());
}

void MainWindow::loadFile(const QString &fileName)
{
    MachineTab *machineTab = new MachineTab(tabWidget, fileName, myMachinesPath);

    int index = tabWidget->addTab(machineTab, QIcon(":/images/" + iconTheme + "/qtemu.png"), machineTab->machineName());
    tabWidget->setCurrentIndex(index);
    tabWidget->setTabToolTip(index, fileName);

    statusBar()->showMessage(tr("Machine loaded"), 2000);
}

void MainWindow::changeMachineState(int value)
{
    if (tabWidget->currentIndex() != 0) //do not check the main tab
    {
        MachineTab *tab = qobject_cast<MachineTab *>(tabWidget->currentWidget());
        QPushButton *startButton = qobject_cast<QPushButton *>(tab->startButton);
        if (value != -1)
            startButton->setFocus();
        QPushButton *stopButton = qobject_cast<QPushButton *>(tab->stopButton);
        connect(startButton, SIGNAL(clicked()), this, SLOT(changeMachineState()));
        connect(stopButton, SIGNAL(clicked()), this, SLOT(changeMachineState()));
        if (!startButton->isEnabled()&&tab->isEnabled())
        {
            stopAct->setEnabled(true);
            pauseAct->setEnabled(true);
            startAct->setEnabled(false);
            restartAct->setEnabled(true);
        }
        else if (tab->isEnabled())
        {
            stopAct->setEnabled(false);
            pauseAct->setEnabled(false);
            startAct->setEnabled(true);
            restartAct->setEnabled(false);
        }
        else
        {
            stopAct->setEnabled(false);
            pauseAct->setEnabled(false);
            startAct->setEnabled(false);
            restartAct->setEnabled(false);
        }
    }
    else //main tab is active
    {
        stopAct->setEnabled(false);
        pauseAct->setEnabled(false);
        startAct->setEnabled(false);
        restartAct->setEnabled(false);
    }
}

