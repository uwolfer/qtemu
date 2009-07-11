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

#ifndef QTEMUENVIRONMENT_H
#define QTEMUENVIRONMENT_H

/**
	@author Ben Klopfenstein <benklop@gmail.com>
*/

class HalObject;

class QtEmuEnvironment{
public:
    QtEmuEnvironment();

    ~QtEmuEnvironment();

///returned as an array with 3 members: major version, minor version, and bugfix version.
    static int* getQemuVersion();

///just a plain old number
    static int getKvmVersion();

    static HalObject* getHal();

    static bool kvmSupport();
    static bool kqemuSupport();

private:
    static void getVersion();
    static int qemuVersion[3];
    static int kvmVersion;
    static bool versionChecked;
    static bool supportsKvm;
    static bool supportsKqemu;
    static HalObject *hal;
};
#endif
