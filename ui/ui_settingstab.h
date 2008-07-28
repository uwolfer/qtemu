/********************************************************************************
** Form generated from reading ui file 'settingstab.ui'
**
** Created: Mon Jul 28 12:36:04 2008
**      by: Qt User Interface Compiler version 4.4.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_SETTINGSTAB_H
#define UI_SETTINGSTAB_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QFrame>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_SettingsTab
{
public:
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_3;
    QHBoxLayout *horizontalLayout_2;
    QSpinBox *cpuSpinBox;
    QLabel *label;
    QFrame *line;
    QLabel *label_2;
    QHBoxLayout *horizontalLayout_3;
    QSlider *memorySlider;
    QSpinBox *memorySpinBox;
    QFrame *line_2;
    QCheckBox *virtCheckBox;
    QGroupBox *groupBox_2;
    QGroupBox *groupBox_3;
    QGroupBox *groupBox_7;
    QSpacerItem *verticalSpacer;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox_4;
    QGroupBox *groupBox_5;
    QGroupBox *groupBox_6;
    QSpacerItem *verticalSpacer_2;

    void setupUi(QFrame *SettingsTab)
    {
    if (SettingsTab->objectName().isEmpty())
        SettingsTab->setObjectName(QString::fromUtf8("SettingsTab"));
    SettingsTab->setWindowModality(Qt::NonModal);
    SettingsTab->resize(497, 473);
    SettingsTab->setFrameShape(QFrame::StyledPanel);
    SettingsTab->setFrameShadow(QFrame::Raised);
    horizontalLayout = new QHBoxLayout(SettingsTab);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    verticalLayout = new QVBoxLayout();
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    groupBox = new QGroupBox(SettingsTab);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    verticalLayout_3 = new QVBoxLayout(groupBox);
    verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
    label_3 = new QLabel(groupBox);
    label_3->setObjectName(QString::fromUtf8("label_3"));

    verticalLayout_3->addWidget(label_3);

    horizontalLayout_2 = new QHBoxLayout();
    horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
    cpuSpinBox = new QSpinBox(groupBox);
    cpuSpinBox->setObjectName(QString::fromUtf8("cpuSpinBox"));

    horizontalLayout_2->addWidget(cpuSpinBox);

    label = new QLabel(groupBox);
    label->setObjectName(QString::fromUtf8("label"));

    horizontalLayout_2->addWidget(label);


    verticalLayout_3->addLayout(horizontalLayout_2);

    line = new QFrame(groupBox);
    line->setObjectName(QString::fromUtf8("line"));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    verticalLayout_3->addWidget(line);

    label_2 = new QLabel(groupBox);
    label_2->setObjectName(QString::fromUtf8("label_2"));

    verticalLayout_3->addWidget(label_2);

    horizontalLayout_3 = new QHBoxLayout();
    horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
    memorySlider = new QSlider(groupBox);
    memorySlider->setObjectName(QString::fromUtf8("memorySlider"));
    memorySlider->setMaximum(1024);
    memorySlider->setSingleStep(16);
    memorySlider->setPageStep(128);
    memorySlider->setOrientation(Qt::Horizontal);

    horizontalLayout_3->addWidget(memorySlider);

    memorySpinBox = new QSpinBox(groupBox);
    memorySpinBox->setObjectName(QString::fromUtf8("memorySpinBox"));
    memorySpinBox->setMaximum(1024);

    horizontalLayout_3->addWidget(memorySpinBox);


    verticalLayout_3->addLayout(horizontalLayout_3);

    line_2 = new QFrame(groupBox);
    line_2->setObjectName(QString::fromUtf8("line_2"));
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFrameShadow(QFrame::Sunken);

    verticalLayout_3->addWidget(line_2);

    virtCheckBox = new QCheckBox(groupBox);
    virtCheckBox->setObjectName(QString::fromUtf8("virtCheckBox"));

    verticalLayout_3->addWidget(virtCheckBox);


    verticalLayout->addWidget(groupBox);

    groupBox_2 = new QGroupBox(SettingsTab);
    groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));

    verticalLayout->addWidget(groupBox_2);

    groupBox_3 = new QGroupBox(SettingsTab);
    groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));

    verticalLayout->addWidget(groupBox_3);

    groupBox_7 = new QGroupBox(SettingsTab);
    groupBox_7->setObjectName(QString::fromUtf8("groupBox_7"));

    verticalLayout->addWidget(groupBox_7);

    verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout->addItem(verticalSpacer);


    horizontalLayout->addLayout(verticalLayout);

    verticalLayout_2 = new QVBoxLayout();
    verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
    groupBox_4 = new QGroupBox(SettingsTab);
    groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));

    verticalLayout_2->addWidget(groupBox_4);

    groupBox_5 = new QGroupBox(SettingsTab);
    groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));

    verticalLayout_2->addWidget(groupBox_5);

    groupBox_6 = new QGroupBox(SettingsTab);
    groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));

    verticalLayout_2->addWidget(groupBox_6);

    verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout_2->addItem(verticalSpacer_2);


    horizontalLayout->addLayout(verticalLayout_2);

    label_3->setBuddy(cpuSpinBox);
    label->setBuddy(cpuSpinBox);
    label_2->setBuddy(memorySpinBox);
    QWidget::setTabOrder(cpuSpinBox, memorySlider);
    QWidget::setTabOrder(memorySlider, memorySpinBox);
    QWidget::setTabOrder(memorySpinBox, virtCheckBox);

    retranslateUi(SettingsTab);
    QObject::connect(memorySlider, SIGNAL(valueChanged(int)), memorySpinBox, SLOT(setValue(int)));
    QObject::connect(memorySpinBox, SIGNAL(valueChanged(int)), memorySlider, SLOT(setValue(int)));

    QMetaObject::connectSlotsByName(SettingsTab);
    } // setupUi

    void retranslateUi(QFrame *SettingsTab)
    {
    SettingsTab->setWindowTitle(QApplication::translate("SettingsTab", "Settings", 0, QApplication::UnicodeUTF8));
    groupBox->setTitle(QApplication::translate("SettingsTab", "Cpu / Memory", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("SettingsTab", "Number of virtual CPUs:", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("SettingsTab", "Virtual CPU(s)", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("SettingsTab", "Memory for this virtual machine:", 0, QApplication::UnicodeUTF8));
    virtCheckBox->setText(QApplication::translate("SettingsTab", "Enable Virtualization", 0, QApplication::UnicodeUTF8));
    groupBox_2->setTitle(QApplication::translate("SettingsTab", "Network", 0, QApplication::UnicodeUTF8));
    groupBox_3->setTitle(QApplication::translate("SettingsTab", "Usb", 0, QApplication::UnicodeUTF8));
    groupBox_7->setTitle(QApplication::translate("SettingsTab", "Sound", 0, QApplication::UnicodeUTF8));
    groupBox_4->setTitle(QApplication::translate("SettingsTab", "Hard Disk", 0, QApplication::UnicodeUTF8));
    groupBox_5->setTitle(QApplication::translate("SettingsTab", "Removable Media", 0, QApplication::UnicodeUTF8));
    groupBox_6->setTitle(QApplication::translate("SettingsTab", "Additional", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(SettingsTab);
    } // retranslateUi

};

namespace Ui {
    class SettingsTab: public Ui_SettingsTab {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGSTAB_H
