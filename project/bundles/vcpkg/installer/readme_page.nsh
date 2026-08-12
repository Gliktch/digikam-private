;; ============================================================
 ;
 ; This file is a part of digiKam project
 ; https://www.digikam.org
 ;
 ; Date        : 2007-01-01
 ; Description : Functions to create README page.
 ;               For details: http://nsis.sourceforge.net/Readme_Page_Based_on_MUI_License_Page
 ;               Note: NSIS >= 3 is required to be compatible with Windows 10.
 ;
 ; SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 ;
 ; SPDX-License-Identifier: GPL-2.0-or-later
 ;
 ; ============================================================ ;;

!ifndef README_PAGE_NSH
!define README_PAGE_NSH

!macro MUI_EXTRAPAGE_README UN ReadmeFile

    !verbose push
    !verbose 3

        !define MUI_PAGE_HEADER_TEXT "Read Me"
        !define MUI_PAGE_HEADER_SUBTEXT "Please review the following important information."
        !define MUI_LICENSEPAGE_TEXT_TOP "About $(^name):"
        !define MUI_LICENSEPAGE_TEXT_BOTTOM "Click on scrollbar arrows or press Page Down to review the entire text."
        !define MUI_LICENSEPAGE_BUTTON "$(^NextBtn)"
        !insertmacro MUI_${UN}PAGE_LICENSE "${ReadmeFile}"

    !verbose pop

!macroend

;-------------------------------------------------------------------------------------

!define ReadmeRun "!insertmacro MUI_EXTRAPAGE_README"

!macro MUI_PAGE_README ReadmeFile

    !verbose push
    !verbose 3

        ${ReadmeRun} "" "${ReadmeFile}"

    !verbose pop

!macroend

!endif ;README_PAGE_NSH
