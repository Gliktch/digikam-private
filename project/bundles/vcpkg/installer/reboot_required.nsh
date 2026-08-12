;; ============================================================
 ;
 ; This file is a part of digiKam project
 ; https://www.digikam.org
 ;
 ; Date        : 2010-11-08
 ; Description : Functions to check if reboot is required.
 ;               Note: NSIS >= 3 is required to be compatible with Windows 10.
 ;
 ; SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 ;
 ; SPDX-License-Identifier: GPL-2.0-or-later
 ;
 ; ============================================================ ;;

!ifndef REBOOT_REQUIRED_INCLUDED
!define REBOOT_REQUIRED_INCLUDED

Function DirectoryLeave

    Call NotifyIfRebootRequired

FunctionEnd

;-------------------------------------------

Function NotifyIfRebootRequired

    Call IsRebootRequired
    Exch $0

    ${If} $0 == 1

        ;TODO: consider adding a RunOnce entry for the installer to HKCU instead of telling the user they need to run the installer
        ;themselves (can't add to HKLM because basic user wouldn't have access, only admins do).
        ;this would require using the UAC plugin to handle elevation by starting as a normal user, elevating, and then dropping back to normal when writing to HKCU
        ;TODO: need to internationalize string (see VLC / clementine / etc)

        MessageBox MB_YESNO|MB_ICONSTOP|MB_TOPMOST|MB_SETFOREGROUND "You must reboot to complete uninstallation of a previous install of ${MY_PRODUCT} before ${MY_PRODUCT} ${VERSION} can be installed.$\r$\n$\r$\n\
            Would you like to reboot now?$\r$\n$\r$\n\
            (You will have to run the installer again after reboot to continue setup)" /SD IDNO IDNO noInstall
            Reboot

    ${Else}

        Goto done

    ${EndIf}

    noInstall:
        Abort

    done:
        Pop $0

FunctionEnd

Function IsRebootRequired

    Push $0
    Push $1
    Push $2
    Push $3

    ReadRegStr $0 HKLM "SYSTEM\CurrentControlSet\Control\Session Manager" "PendingFileRenameOperations"

    ${If} $0 != ""

        StrLen $2 "$INSTDIR"
        ${StrStr} $1 "$0" "$INSTDIR"
        StrCpy $3 $1 $2
        ${AndIf} $3 == "$INSTDIR"
        StrCpy $0 1

    ${Else}

        StrCpy $0 0

    ${EndIf}

    Pop $3
    Pop $2
    Pop $1
    Exch $0

FunctionEnd

!endif ;REBOOT_REQUIRED_INCLUDED
