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

#include "configwindow.h"

#include "config.h"

#include <QTextEdit>
#include <QComboBox>
#include <QSettings>
#include <QBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QCoreApplication>

ConfigWindow::ConfigWindow(const QString &myMachinesPathParent, int tabPosition, QWidget *parent)
    : QDialog((QWidget*)parent)
{
    myMachinesPath = myMachinesPathParent;

    setWindowTitle(tr("QtEmu Config"));
    resize(400, 200);
    setSizeGripEnabled(true);

    QGroupBox *generalGroupBox = new QGroupBox(tr("General"), this);

    QLabel *myMachinesPathLabel = new QLabel(tr("Default \"MyMachines\" Path:"));
    myMachinePathLineEdit = new QLineEdit(myMachinesPath);
    myMachinesPathLabel->setBuddy(myMachinePathLineEdit);

    QSettings settings("QtEmu", "QtEmu");
    QString iconTheme = settings.value("iconTheme", "oxygen").toString();

    QPushButton *pathSelectButton = new QPushButton(QIcon(":/images/" + iconTheme + "/open.png"), QString());
    connect(pathSelectButton, SIGNAL(clicked()), this, SLOT(setNewPath()));

    QHBoxLayout *pathLayout = new QHBoxLayout;
    pathLayout->addWidget(myMachinePathLineEdit);
    pathLayout->addWidget(pathSelectButton);

    QLabel *tabPositionLabel = new QLabel(tr("Tabbar position:"));

    comboTabPosition = new QComboBox;
    comboTabPosition->addItem(tr("Top"));
    comboTabPosition->addItem(tr("Bottom"));
    comboTabPosition->addItem(tr("Left"));
    comboTabPosition->addItem(tr("Right"));
    comboTabPosition->setCurrentIndex(tabPosition);
    tabPositionLabel->setBuddy(comboTabPosition);

    QLabel *iconThemeLabel = new QLabel(tr("Icon theme (*):"));

    comboIconTheme = new QComboBox;
    comboIconTheme->addItem("Oxygen");
    comboIconTheme->addItem("Crystal");
    comboIconTheme->setCurrentIndex(tabPosition);
    iconThemeLabel->setBuddy(comboIconTheme);

    QLabel *languageLabel = new QLabel(tr("Language (*):"));

    languagePosition = new QComboBox; //make the language name not tranlatable (no tr()!) and write them translated
    languagePosition->addItem("English");
    languagePosition->addItem("Deutsch");
    languagePosition->addItem(QString::fromUtf8("Türkçe"));
    languagePosition->addItem(QString::fromUtf8("Русский"));
    languagePosition->addItem(QString::fromUtf8("Česky"));
    languagePosition->addItem(QString::fromUtf8("Español"));
    languagePosition->addItem(QString::fromUtf8("Français"));
    languagePosition->addItem(QString::fromUtf8("Italiano"));
    languagePosition->addItem(QString::fromUtf8("Português do Brasil"));
    languagePosition->addItem(QString::fromUtf8("Polski"));

    QString language = settings.value("language", QString(QLocale::system().name())).toString();
    int index;
    if (language == "en")
        index = 0;
    else if (language == "de")
        index = 1;
    else if (language == "tr")
        index = 2;
    else if (language == "ru")
        index = 3;
    else if (language == "cz")
        index = 4;
    else if (language == "es")
        index = 5;
    else if (language == "fr")
        index = 6;
    else if (language == "it")
        index = 7;
    else if (language == "pt-BR")
        index = 8;
    else if (language == "pl")
        index = 9;
    else
        index = 0;

    languagePosition->setCurrentIndex(index);
    connect(languagePosition, SIGNAL(currentIndexChanged(int)), this, SLOT(languageChange(int)));
    languageLabel->setBuddy(languagePosition);

    QLabel *restartLabel = new QLabel(tr("<i>(*) Change requires restart of QtEmu.</i>"));

    QGridLayout *generalLayout = new QGridLayout;
    generalLayout->addWidget(myMachinesPathLabel, 1, 0);
    generalLayout->addLayout(pathLayout, 1, 1);
    generalLayout->addWidget(tabPositionLabel, 2, 0);
    generalLayout->addWidget(comboTabPosition, 2, 1);
    generalLayout->addWidget(iconThemeLabel, 3, 0);
    generalLayout->addWidget(comboIconTheme, 3, 1);
    generalLayout->addWidget(languageLabel, 4, 0);
    generalLayout->addWidget(languagePosition, 4, 1);
    generalLayout->addWidget(restartLabel, 5, 0);
    generalGroupBox->setLayout(generalLayout);

    QGroupBox *qemuGroupBox = new QGroupBox(tr("Start and stop QEMU"), this);

    QLabel *beforeStartExeLabel = new QLabel(tr("Execute before start:"));
    beforeStartExeTextEdit = new QTextEdit;
    beforeStartExeLabel->setBuddy(beforeStartExeTextEdit);

    QLabel *commandLabel = new QLabel(tr("QEMU start command:"));
    commandLineEdit = new QLineEdit;
    commandLabel->setBuddy(commandLineEdit);

    QLabel *afterExitExeLabel = new QLabel(tr("Execute after exit:"));
    afterExitExeTextEdit = new QTextEdit;
    afterExitExeLabel->setBuddy(afterExitExeTextEdit);

    QGridLayout *qemuLayout = new QGridLayout;
    qemuLayout->addWidget(beforeStartExeLabel, 1, 0, Qt::AlignTop);
    qemuLayout->addWidget(beforeStartExeTextEdit, 1, 1);
    qemuLayout->addWidget(commandLabel, 2, 0);
    qemuLayout->addWidget(commandLineEdit, 2, 1);
    qemuLayout->addWidget(afterExitExeLabel, 4, 0, Qt::AlignTop);
    qemuLayout->addWidget(afterExitExeTextEdit, 4, 1);

    qemuLayout->setRowStretch(4, 1);
    qemuGroupBox->setLayout(qemuLayout);

    QPushButton *okButton = new QPushButton(tr("OK"));
    connect(okButton, SIGNAL(clicked()), this, SLOT(writeSettings()));

    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(generalGroupBox);
    mainLayout->addWidget(qemuGroupBox);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    loadSettings();
}

void ConfigWindow::setNewPath()
{
    QString newPath = QFileDialog::getExistingDirectory(this, tr("Select a folder for \"MyMachines\""),
                                                        myMachinesPath);
    if (!newPath.isEmpty())
        myMachinePathLineEdit->setText(newPath);
}

void ConfigWindow::languageChange(int index)
{
    QSettings settings("QtEmu", "QtEmu");
    QString languageString;
    switch(index)
    {
        case 0: languageString = "en";
           break;
        case 1: languageString = "de";
           break;
        case 2: languageString = "tr";
           break;
        case 3: languageString = "ru";
           break;
        case 4: languageString = "cz";
           break;
        case 5: languageString = "es";
           break;
        case 6: languageString = "fr";
           break;
        case 7: languageString = "it";
           break;
        case 8: languageString = "pt-BR";
           break;
        case 9: languageString = "pl";
           break;
        default: languageString = "en";
    }
    settings.setValue("language", languageString);
}

void ConfigWindow::loadSettings()
{
    QSettings settings("QtEmu", "QtEmu");
    beforeStartExeTextEdit->setPlainText(settings.value("beforeStart").toString());
#ifndef Q_OS_WIN32
    commandLineEdit->setText(settings.value("command", "qemu").toString());
#elif defined(Q_OS_WIN32)
    commandLineEdit->setText(settings.value("command", QCoreApplication::applicationDirPath() + "/qemu/qemu.exe").toString());
#endif
    afterExitExeTextEdit->setPlainText(settings.value("afterExit").toString());

    comboIconTheme->setCurrentIndex(comboIconTheme->findText(settings.value("iconTheme", "oxygen").toString(), Qt::MatchContains));
    
}

void ConfigWindow::writeSettings()
{
    QSettings settings("QtEmu", "QtEmu");
    settings.setValue("beforeStart", beforeStartExeTextEdit->toPlainText());
    settings.setValue("command", commandLineEdit->text());
    settings.setValue("afterExit", afterExitExeTextEdit->toPlainText());
    settings.setValue("iconTheme", comboIconTheme->currentText().toLower());

    accept();
}
