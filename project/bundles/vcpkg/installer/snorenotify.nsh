;; ============================================================
 ;
 ; This file is a part of digiKam project
 ; https://www.digikam.org
 ;
 ; Date        : 2025-08-30
 ; Description : Functions to create application shortcute with SnoreToast.exe.
 ;
 ; SPDX-FileCopyrightText: 2025 by Maik Qualmann <metzpinguin at gmail dot com>
 ;
 ; SPDX-License-Identifier: GPL-2.0-or-later
 ;
 ; ============================================================ ;;

!ifndef SNORE_NOTIFY_INCLUDED
!define SNORE_NOTIFY_INCLUDED

!include "LogicLib.nsh"
!include "WordFunc.nsh"

Function SnoreWinVer

    ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion
    ${VersionCompare} "6.2" $R0 $R0

    ${If} $R0 == 1

        Push "NotWin8"

    ${Else}

        Push "AtLeastWin8"

    ${EndIf}

FunctionEnd

!macro SnoreShortcut path exe appID

    Call SnoreWinVer
    Pop $0

    ${If} $0 == "AtLeastWin8"

        nsExec::ExecToLog '"${SnoreToastExe}" -install "${path}" "${exe}" "${appID}"'

    ${Else}

        CreateShortCut "${path}" "${exe}"

    ${EndIf}

!macroend

!endif ;SNORE_NOTIFY_INCLUDED
