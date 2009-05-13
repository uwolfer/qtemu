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

#ifndef MACHINECONFIG_H
#define MACHINECONFIG_H

#include <QObject>
#include <QDomElement>
#include <QVariant>

class QFile;
class QDomDocument;
class QStringList;


/**
	@author Ben Klopfenstein <benklop@gmail.com>
*/
class MachineConfig : public QObject
{
Q_OBJECT
public:
    explicit MachineConfig(QObject *parent = 0, const QString &store = QString());

    ~MachineConfig();

    bool loadConfig(const QString &fileName);
    bool saveConfig(const QString &fileName) const;
    bool convertConfig(const QString &fileName) const;

    //if nodeType and nodeName are not specified, they are assumed to be "machine" and ""
    QVariant getOption(const QString &nodeType, const QString &nodeName, const QString &optionName, const QVariant defaultValue = QVariant());

    QStringList getAllOptionNames(const QString &nodeType, const QString &nodeName) const;

    int getNumOptions(const QString &nodeType, const QString &nodeName) const;

public slots:
    //if nodeType and nodeName are not specified, they are assumed to be "machine" and ""
    void setOption(const QString &nodeType, const QString &nodeName, const QString &optionName, const QVariant &value);

    void clearOption(const QString &nodeType, const QString &nodeName, const QString &optionName);
signals:
    void optionChanged(const QString &nodeType, const QString &nodeName, const QString &optionName, const QVariant &value);

private:
    QFile *configFile;
    QDomDocument domDocument;
    QDomElement root;

};

#endif
