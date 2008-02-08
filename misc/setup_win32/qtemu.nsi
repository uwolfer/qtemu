;/****************************************************************************
;**
;** Copyright (C) 2006-2007 Urs Wolfer <uwolfer @ fwo.ch>
;**
;** This file is part of QtEmu.
;**
;** This program is free software; you can redistribute it and/or modify
;** it under the terms of the GNU General Public License as published by
;** the Free Software Foundation; either version 2 of the License, or
;** (at your option) any later version.
;**
;** This program is distributed in the hope that it will be useful,
;** but WITHOUT ANY WARRANTY; without even the implied warranty of
;** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
;** GNU General Public License for more details.
;**
;** You should have received a copy of the GNU Library General Public License
;** along with this library; see the file COPYING.LIB.  If not, write to
;** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
;** Boston, MA 02110-1301, USA.
;**
;****************************************************************************/

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;General

  ;Name and file
  Name "QtEmu"
  OutFile "qtemu-1.0.5.exe"
  !define MUI_ICON "qtemu.ico"
  !define MUI_UNICON "uninstall.ico"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\QtEmu"

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\QtEmu" ""

  ; use lzma compression
  SetCompressor /SOLID lzma

  ; adds xp style support
  XPStyle on

;--------------------------------
;Interface Settings

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "header.bmp"
  !define MUI_WELCOMEFINISHPAGE_BITMAP "welcome.bmp"
  !define MUI_ABORTWARNING

;--------------------------------
;Language Selection Dialog Settings

  ;Remember the installer language
  !define MUI_LANGDLL_REGISTRY_ROOT "HKCU" 
  !define MUI_LANGDLL_REGISTRY_KEY "Software\QtEmu" 
  !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE $(MUILicense)
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES

  ;!define MUI_FINISHPAGE_LINK "Visit the QtEmu site for the latest news, FAQs and support."
  ;!define MUI_FINISHPAGE_LINK_LOCATION "http://qtemu.sf.net/"

  !define MUI_FINISHPAGE_RUN "$INSTDIR\qtemu.exe"
  !define MUI_FINISHPAGE_NOREBOOTSUPPORT

  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English" # first language is the default language
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Turkish"
  !insertmacro MUI_LANGUAGE "Russian"
  !insertmacro MUI_LANGUAGE "Czech"
  !insertmacro MUI_LANGUAGE "Spanish"
  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "Italian"
  !insertmacro MUI_LANGUAGE "Polish"
  !insertmacro MUI_LANGUAGE "PortugueseBR"

;--------------------------------
;License Language String

  LicenseLangString MUILicense ${LANG_ENGLISH} "gpl.txt"
  LicenseLangString MUILicense ${LANG_GERMAN} "gpl.txt"
  LicenseLangString MUILicense ${LANG_TURKISH} "gpl.txt"
  LicenseLangString MUILicense ${LANG_RUSSIAN} "gpl.txt"
  LicenseLangString MUILicense ${LANG_CZECH} "gpl.txt"
  LicenseLangString MUILicense ${LANG_SPANISH} "gpl.txt"
  LicenseLangString MUILicense ${LANG_FRENCH} "gpl.txt"
  LicenseLangString MUILicense ${LANG_ITALIAN} "gpl.txt"
  LicenseLangString MUILicense ${LANG_POLISH} "gpl.txt"
  LicenseLangString MUILicense ${LANG_PORTUGUESEBR} "gpl.txt"

;--------------------------------
;Reserve Files

  ;These files should be inserted before other files in the data block
  ;Keep these lines before any File command
  ;Only for solid compression (by default, solid compression is enabled for BZIP2 and LZMA)

  !insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------
;Installer Sections

Section "QtEmu" SecQtEmu

  ;user cannot deactivate it
  SectionIn RO

  SetOutPath "$INSTDIR"

  File ..\qtemu.exe
  File qtemu.exe.manifest
  File qtemu.ico

  ;Create uninstaller
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\QtEmu" DisplayName "QtEmu"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\QtEmu" UninstallString '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\QtEmu" DisplayIcon $INSTDIR\qtemu.ico
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ;create desktop shortcut
  CreateShortCut "$DESKTOP\QtEmu.lnk" "$INSTDIR\qtemu.exe" ""

  ;create start-menu items
  CreateDirectory "$SMPROGRAMS\QtEmu"
  CreateShortCut "$SMPROGRAMS\QtEmu\Uninstall.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\QtEmu\QtEmu.lnk" "$INSTDIR\qtemu.exe" "" "$INSTDIR\qtemu.exe" 0

SectionEnd


Section "Qemu" SecQemu

  SetOutPath "$INSTDIR\qemu"

  File ..\qemu\bios.bin
  File ..\qemu\fmod.dll
  File ..\qemu\libusb0.dll
  File ..\qemu\linux_boot.bin
  File ..\qemu\pxe-ne2k_pci.bin
  File ..\qemu\pxe-pcnet.bin
  File ..\qemu\pxe-rtl8139.bin
  File ..\qemu\qemu.exe
  File ..\qemu\qemu-img.exe
  File ..\qemu\qemu-system-x86_64.exe
  File ..\qemu\README-en.txt
  File ..\qemu\SDL.dll
  File ..\qemu\vgabios.bin
  File ..\qemu\vgabios-cirrus.bin

  File /r ..\qemu\keymaps
  File /r ..\qemu\License

  ;Store installation folder
  WriteRegStr HKCU "Software\QtEmu" "" $INSTDIR

SectionEnd

Section "Translations" SecTranslations

  SetOutPath "$INSTDIR"

  File /r ..\translations

SectionEnd

Section "Help" SecHelp

  SetOutPath "$INSTDIR"

  File /r ..\help

SectionEnd

SubSection "Libraries" SecLibs

  Section "Qt Library" SecQtLibs

    SetOutPath "$INSTDIR"

    File ..\QtCore4.dll
    File ..\QtGui4.dll
    File ..\QtXml4.dll

  SectionEnd

  Section "MinGW Library" SecMingwLibs

    SetOutPath "$INSTDIR"

    File ..\mingwm10.dll

  SectionEnd

SubSectionEnd

;--------------------------------
;Installer Functions

Function .onInit

  !insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd

;--------------------------------
;Descriptions

  ;USE A LANGUAGE STRING IF YOU WANT YOUR DESCRIPTIONS TO BE LANGAUGE SPECIFIC

  ;Assign descriptions to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecQtEmu} "QtEmu"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecQemu} "Qemu executable and files (recommended)."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecTranslations} "QtEmu Translations."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecHelp} "QtEmu User Documentation."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecLibs} "Required libraries (recommended)."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecQtLibs} "Not required, if you have Qt 4.3 installed on your system."
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMingwLibs} "Not required, if you have MinGW installed on your system."
  !insertmacro MUI_FUNCTION_DESCRIPTION_END


;--------------------------------
;Uninstaller Section

Section "Uninstall"

  RMDir /r "$INSTDIR"

  DeleteRegKey /ifempty HKCU "Software\QtEmu"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\QtEmu"

  Delete "$DESKTOP\QtEmu.lnk"
  RMDir /r  "$SMPROGRAMS\QtEmu"
  
SectionEnd

;--------------------------------
;Uninstaller Functions

Function un.onInit

  !insertmacro MUI_UNGETLANGUAGE

FunctionEnd
