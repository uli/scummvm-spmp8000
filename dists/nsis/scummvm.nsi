# ScummVM - Graphic Adventure Engine
#
# ScummVM is the legal property of its developers, whose names
# are too numerous to list here. Please refer to the COPYRIGHT
# file distributed with this source distribution.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

#!define _DEBUG

Name ScummVM

# Included files
!include MUI2.nsh

#########################################################################################
# Folders
#########################################################################################
#!define top_srcdir   ""    ; passed through command line
#!define build_dir    ""    ; passed through command line
#!define text_dir     ""    ; passed through command line
!define engine_data  "${top_srcdir}\dists\engine-data"
!define theme_data   "${top_srcdir}\gui\themes"

#########################################################################################
# General Symbol Definitions
#########################################################################################
!define REGKEY      "Software\$(^Name)\$(^Name)"
!define VERSION     "1.3.0git"
!define COMPANY     "ScummVM Team"
!define URL         "http://scummvm.org/"
!define DESCRIPTION "ScummVM Installer. Look! A three headed monkey (TM)!"
!define COPYRIGHT   "Copyright � 2001-2011 The ScummVM Team"

#########################################################################################
# Installer configuration
#########################################################################################
OutFile          ${build_dir}\scummvm-${VERSION}-win32.exe
InstallDir       $PROGRAMFILES\ScummVM                            ; Default installation folder
InstallDirRegKey HKCU "Software\ScummVM\ScummVM" "InstallPath"    ; Get installation folder from registry if available
                                                                  ; The application name needs to be refered directly instead of through ${REGKEY}
                                                                  ; because lang strings aren't initialized at the point InstallDirRegKey is called

CRCCheck on
XPStyle  on
#TargetMinimalOS 5.0    ; Minimal version of windows for installer: Windows 2000 or more recent
                        ; (will build unicode installer with NSIS 2.50+)

VIProductVersion 1.3.0.0
VIAddVersionKey  ProductName      $(^Name)
VIAddVersionKey  ProductVersion  "${VERSION}"
VIAddVersionKey  CompanyName     "${COMPANY}"
VIAddVersionKey  CompanyWebsite  "${URL}"
VIAddVersionKey  FileVersion     "${VERSION}"
VIAddVersionKey  FileDescription "${DESCRIPTION}"
VIAddVersionKey  LegalCopyright  "${COPYRIGHT}"

BrandingText "$(^Name) ${VERSION}"   ; Change branding text on the installer to show our name and version instead of NSIS's

# Show Details when installing/uninstalling files
ShowInstDetails   show
ShowUninstDetails show

!ifdef _DEBUG
	SetCompress off                      ; for debugging the installer, lzma takes forever
	RequestExecutionLevel user
!else
	SetCompressor /FINAL /SOLID lzma
	SetCompressorDictSize 64
	RequestExecutionLevel admin          ; for installation into program files folders
!endif

#########################################################################################
# MUI Symbol Definitions
#########################################################################################
!define MUI_WELCOMEFINISHPAGE_BITMAP "graphics\left.bmp"
!define MUI_ICON                     "graphics\scummvm-install.ico"
!define MUI_UNICON                   "graphics\scummvm-install.ico"

#Start menu
!define MUI_STARTMENUPAGE_REGISTRY_ROOT      HKCU
!define MUI_STARTMENUPAGE_REGISTRY_KEY       ${REGKEY}
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME StartMenuGroup
!define MUI_STARTMENUPAGE_DEFAULTFOLDER      $(^Name)

# Finish page
!define MUI_FINISHPAGE_RUN        "$INSTDIR\scummvm.exe"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.txt"
!define MUI_FINISHPAGE_RUN_NOTCHECKED
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED

!define MUI_LICENSEPAGE_RADIOBUTTONS

!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_UNFINISHPAGE_NOAUTOCLOSE

#########################################################################################
# Installer pages
#########################################################################################
# Variables
Var StartMenuGroup

;Remember the installer language
!define MUI_LANGDLL_REGISTRY_ROOT      HKCU
!define MUI_LANGDLL_REGISTRY_KEY       ${REGKEY}
!define MUI_LANGDLL_REGISTRY_VALUENAME "InstallerLanguage"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE ${top_srcdir}\COPYING
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuGroup
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

# Installer languages
!insertmacro MUI_LANGUAGE "English"    ;first language is the default language

!ifndef _DEBUG    ; Skip other languages when building debug builds
;!insertmacro MUI_LANGUAGE "Afrikaans"
;!insertmacro MUI_LANGUAGE "Albanian"
;!insertmacro MUI_LANGUAGE "Arabic"
;!insertmacro MUI_LANGUAGE "Belarusian"
;!insertmacro MUI_LANGUAGE "Bosnian"
;!insertmacro MUI_LANGUAGE "Breton"
;!insertmacro MUI_LANGUAGE "Bulgarian"
!insertmacro MUI_LANGUAGE "Catalan"
;!insertmacro MUI_LANGUAGE "Croatian"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "Danish"
;!insertmacro MUI_LANGUAGE "Dutch"
;!insertmacro MUI_LANGUAGE "Esperanto"
;!insertmacro MUI_LANGUAGE "Estonian"
;!insertmacro MUI_LANGUAGE "Farsi"
;!insertmacro MUI_LANGUAGE "Finnish"
!insertmacro MUI_LANGUAGE "French"
;!insertmacro MUI_LANGUAGE "Galician"
!insertmacro MUI_LANGUAGE "German"
;!insertmacro MUI_LANGUAGE "Greek"
;!insertmacro MUI_LANGUAGE "Hebrew"
!insertmacro MUI_LANGUAGE "Hungarian"
;!insertmacro MUI_LANGUAGE "Icelandic"
;!insertmacro MUI_LANGUAGE "Indonesian"
;!insertmacro MUI_LANGUAGE "Irish"
!insertmacro MUI_LANGUAGE "Italian"
;!insertmacro MUI_LANGUAGE "Japanese"
;!insertmacro MUI_LANGUAGE "Korean"
;!insertmacro MUI_LANGUAGE "Kurdish"
;!insertmacro MUI_LANGUAGE "Latvian"
;!insertmacro MUI_LANGUAGE "Lithuanian"
;!insertmacro MUI_LANGUAGE "Luxembourgish"
;!insertmacro MUI_LANGUAGE "Macedonian"
;!insertmacro MUI_LANGUAGE "Malay"
;!insertmacro MUI_LANGUAGE "Mongolian"
!insertmacro MUI_LANGUAGE "Norwegian"
!insertmacro MUI_LANGUAGE "NorwegianNynorsk"
!insertmacro MUI_LANGUAGE "Polish"
;!insertmacro MUI_LANGUAGE "Portuguese"
!insertmacro MUI_LANGUAGE "PortugueseBR"
;!insertmacro MUI_LANGUAGE "Romanian"
!insertmacro MUI_LANGUAGE "Russian"
;!insertmacro MUI_LANGUAGE "Serbian"
;!insertmacro MUI_LANGUAGE "SerbianLatin"
;!insertmacro MUI_LANGUAGE "SimpChinese"
;!insertmacro MUI_LANGUAGE "Slovak"
;!insertmacro MUI_LANGUAGE "Slovenian"
!insertmacro MUI_LANGUAGE "Spanish"
;!insertmacro MUI_LANGUAGE "SpanishInternational"
!insertmacro MUI_LANGUAGE "Swedish"
;!insertmacro MUI_LANGUAGE "Thai"
;!insertmacro MUI_LANGUAGE "TradChinese"
;!insertmacro MUI_LANGUAGE "Turkish"
!insertmacro MUI_LANGUAGE "Ukrainian"
;!insertmacro MUI_LANGUAGE "Uzbek"
!endif

;Reserve Files (will make sure the file will be stored first in the data block
;               making the installer start faster when compressing in solid mode)
!insertmacro MUI_RESERVEFILE_LANGDLL

#########################################################################################
# Installer sections
#########################################################################################
Section "ScummVM" SecMain
	SetOutPath $INSTDIR
	SetOverwrite on

	# Text files
	File /oname=AUTHORS.txt      "${text_dir}\AUTHORS"
	File /oname=COPYING.LGPL.txt "${text_dir}\COPYING.LGPL"
	File /oname=COPYING.txt      "${text_dir}\COPYING"
	File /oname=COPYRIGHT.txt    "${text_dir}\COPYRIGHT"
	File /oname=NEWS.txt         "${text_dir}\NEWS"
	File /oname=README.txt       "${text_dir}\README"
	File /oname=README-SDL.txt   "${build_dir}\README-SDL"

	# Engine data
	File "${engine_data}\drascula.dat"
	File "${engine_data}\hugo.dat"
	File "${engine_data}\kyra.dat"
	File "${engine_data}\lure.dat"
	File "${engine_data}\m4.dat"
	File "${engine_data}\queen.tbl"
	File "${engine_data}\sky.cpt"
	File "${engine_data}\teenagent.dat"
	File "${engine_data}\toon.dat"

	File "${top_srcdir}\dists\pred.dic"

	# Themes
	File "${theme_data}\scummclassic.zip"
	File "${theme_data}\scummmodern.zip"
	File "${theme_data}\translations.dat"

	# Main exe and dlls
	File "${build_dir}\scummvm.exe"
	File "${build_dir}\SDL.dll"

	WriteRegStr HKCU "${REGKEY}" InstallPath "$INSTDIR"    ; Store installation folder
SectionEnd

# Write Start menu entries and uninstaller
Section -post SecMainPost
	SetOutPath $INSTDIR
	WriteUninstaller $INSTDIR\uninstall.exe
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
	CreateShortCut "$SMPROGRAMS\$StartMenuGroup\$(^Name).lnk" "$INSTDIR\$(^Name).exe" "" "$INSTDIR\$(^Name).exe" 0    ; Create shortcut with icon
	CreateShortcut "$SMPROGRAMS\$StartMenuGroup\Uninstall $(^Name).lnk" $INSTDIR\uninstall.exe
	!insertmacro MUI_STARTMENU_WRITE_END
	WriteRegStr   HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" DisplayName "$(^Name)"
	WriteRegStr   HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" DisplayVersion "${VERSION}"
	WriteRegStr   HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" Publisher "${COMPANY}"
	WriteRegStr   HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" URLInfoAbout "${URL}"
	WriteRegStr   HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" DisplayIcon $INSTDIR\uninstall.exe
	WriteRegStr   HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" UninstallString $INSTDIR\uninstall.exe
	WriteRegStr   HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" InstallLocation $INSTDIR
	WriteRegDWORD HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" NoModify 1
	WriteRegDWORD HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)" NoRepair 1
SectionEnd

# Installer functions
Function .onInit
	!insertmacro MUI_LANGDLL_DISPLAY

!ifdef _DEBUG
	LogSet on    ; Will write a log file to the install folder (when using the special NSIS logging build)
!endif
FunctionEnd

#########################################################################################
# Uninstaller sections
#########################################################################################
Section /o -un.Main SecUninstall
	Delete /REBOOTOK $INSTDIR\AUTHORS.txt
	Delete /REBOOTOK $INSTDIR\COPYING.txt
	Delete /REBOOTOK $INSTDIR\COPYING.LGPL.txt
	Delete /REBOOTOK $INSTDIR\COPYRIGHT.txt
	Delete /REBOOTOK $INSTDIR\NEWS.txt
	Delete /REBOOTOK $INSTDIR\README.txt
	Delete /REBOOTOK $INSTDIR\README-SDL.txt

	Delete /REBOOTOK $INSTDIR\drascula.dat
	Delete /REBOOTOK $INSTDIR\hugo.dat
	Delete /REBOOTOK $INSTDIR\kyra.dat
	Delete /REBOOTOK $INSTDIR\lure.dat
	Delete /REBOOTOK $INSTDIR\m4.dat
	Delete /REBOOTOK $INSTDIR\queen.tbl
	Delete /REBOOTOK $INSTDIR\sky.cpt
	Delete /REBOOTOK $INSTDIR\teenagent.dat
	Delete /REBOOTOK $INSTDIR\toon.dat

	Delete /REBOOTOK $INSTDIR\pred.dic

	Delete /REBOOTOK $INSTDIR\scummclassic.zip
	Delete /REBOOTOK $INSTDIR\scummmodern.zip
	Delete /REBOOTOK $INSTDIR\translations.dat

	Delete /REBOOTOK $INSTDIR\scummvm.exe
	Delete /REBOOTOK $INSTDIR\SDL.dll
SectionEnd

Section -un.post SecUninstallPost
	DeleteRegKey HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\$(^Name)"
	Delete /REBOOTOK "$SMPROGRAMS\$StartMenuGroup\Uninstall $(^Name).lnk"
	Delete /REBOOTOK  $INSTDIR\uninstall.exe
	DeleteRegValue HKCU "${REGKEY}" StartMenuGroup
	DeleteRegValue HKCU "${REGKEY}" InstallPath
	DeleteRegValue HKCU "${REGKEY}" InstallerLanguage
	DeleteRegKey /IfEmpty HKCU "${REGKEY}"
	RmDir /REBOOTOK $SMPROGRAMS\$StartMenuGroup
	RmDir /REBOOTOK $INSTDIR    ; will only remove if empty (pass /r flag for recursive behavior)
	Push $R0
	StrCpy $R0 $StartMenuGroup 1
	StrCmp $R0 ">" no_smgroup
no_smgroup:
	Pop $R0
SectionEnd

# Uninstaller functions
Function un.onInit
	!insertmacro MUI_UNGETLANGUAGE
	ReadRegStr   $INSTDIR HKCU "${REGKEY}" InstallPath
	!insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuGroup
FunctionEnd
