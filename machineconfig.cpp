/****************************************************************************
**
** Copyright (C) 2008 Ben Klopfenstein <benklop@gmail.com>
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

/****************************************************************************
**
** C++ Implementation: machineconfig
**
** Description: 
**
****************************************************************************/

#include "machineconfig.h"
#include <QFile>
#include <QVariant>
#include <QDomDocument>
#include <QDomElement>
#include <QDomText>
#include <QString>
#include <QTextStream>
#include <QStringList>

MachineConfig::MachineConfig(QObject *parent, const QString config)
 : QObject(parent)
{

    if(!config.isEmpty())
        loadConfig(config);

}


MachineConfig::~MachineConfig()
{
    saveConfig(configFile->fileName());
}

bool MachineConfig::loadConfig(const QString fileName)
{
        configFile = new QFile(fileName);
    if (!configFile->open(QFile::ReadOnly | QFile::Text))
    {
        qDebug("Cannot read file" + fileName.toAscii() + ", " + configFile->errorString().toAscii());
        return false;
    }

    QString errorStr;
    int errorLine;
    int errorColumn;
    //domDocument = new domDocument();

    if (!domDocument.setContent(configFile, true, &errorStr, &errorLine, &errorColumn))
    {
       qDebug("Parse error at line %1, column %2:\n" + errorStr.toAscii(), errorLine, errorColumn);
        return false;
    }

    root = domDocument.documentElement();
    if (root.tagName() != "qtemu")
    {
        qDebug("The file is not a QtEmu file.");
        return false;
    }
    else if (root.hasAttribute("version") && root.attribute("version") != "1.0")
    {
        qDebug("The file is not a QtEmu version 1.0 file.");
        return false;
    }
    return true;
}

bool MachineConfig::saveConfig(const QString fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        qDebug("Cannot write file " + fileName.toAscii() +":\n" + file.errorString().toAscii());
        return false;
    }

    QTextStream out(&file);
    domDocument.save(out, 4);
    return true;
}

void MachineConfig::setOption(const QString nodeType, const QString nodeName, const QString optionName, const QVariant value)
{

    //save the value to the config
    QDomElement child = root.firstChildElement(nodeType);

    if(nodeType!="machine")
    {
        child = child.firstChildElement();
        while(child.attribute("name",QString())!=nodeName)
        {
            QDomElement child = child.nextSiblingElement();
        }
    }

    if (!child.isNull())
    {
        QDomElement oldElement = child.firstChildElement(optionName);
        if (oldElement == QDomElement())
        {
            oldElement = domDocument.createElement(optionName);
            child.appendChild(oldElement);
        }

        QDomElement newElement = domDocument.createElement(optionName);
        QDomText newText = domDocument.createTextNode(value.toString());
        newElement.appendChild(newText);
        child.replaceChild(newElement, oldElement);
    }

    //if successful then
    emit optionChanged(ConfigEvent(nodeType, nodeName, optionName, value,QVariant()));
}



const QVariant MachineConfig::getOption(const QString nodeType, const QString nodeName, const QString optionName, const QVariant defaultValue)
{
    //return the value of node named nodeType's child with property name=nodeName's child named optionName
    QDomElement typeElement;
    QDomElement childElement;
    QDomElement optionElement;
    QVariant optionValue = defaultValue;
    //...
    if(nodeType == "machine")
    {
        childElement = root.firstChildElement("machine");
    }
    else
    {
        typeElement = root.firstChildElement(nodeType);
        childElement = typeElement.firstChildElement();
        while(!childElement.isNull())
        {
            if (optionElement.attribute("name") == nodeName)
                break;
            else
                childElement = childElement.nextSiblingElement();
        }
    }
    optionElement = childElement.firstChildElement(optionName);
    if(optionElement.text().size()!=0)
        optionValue = QVariant(optionElement.text());
    return optionValue;
}

const QStringList MachineConfig::getAllOptionNames(const QString nodeType, const QString nodeName)
{
    QDomElement typeElement;
    QDomElement childElement;
    QDomElement optionElement;
    QStringList optionNameList;
    if(nodeType == "machine")
    {
        childElement = root.firstChildElement("machine");
    }
    else
    {
        typeElement = root.firstChildElement(nodeType);
        childElement = typeElement.firstChildElement();
        while(!childElement.isNull())
        {
            if (optionElement.attribute("name") == nodeName)
                break;
            else
                childElement = childElement.nextSiblingElement();
        }
    }
    optionElement = childElement.firstChildElement();
    while(!optionElement.isNull())
    {
        optionNameList.append(optionElement.nodeName());
        optionElement = optionElement.nextSiblingElement();
    }
    return optionNameList;
}





