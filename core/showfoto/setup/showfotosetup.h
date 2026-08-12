/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-04-02
 * Description : showFoto setup dialog.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_config.h"
#include "dconfigdlg.h"

using namespace Digikam;

namespace ShowFoto
{

class ShowfotoSetup : public DConfigDlg
{
    Q_OBJECT

public:

    enum Page
    {
        LastPageUsed        = -1,

        EditorPage          = 0,
        MetadataPage,
        ToolTipPage,
        RawPage,
        IOFilesPage,
        ICCPage,

#ifdef HAVE_GEOLOCATION

        GeolocationPage,

#endif

        PluginsPage,
        MiscellaneousPage,

        SetupPageEnumLast
    };

public:

    explicit ShowfotoSetup(QWidget* const parent = nullptr, Page page = LastPageUsed);
    ~ShowfotoSetup() override;

    static bool execMetadataFilters(QWidget* const parent, int tab);
    static bool execExifTool(QWidget* const parent);

    static bool execLocalize(QWidget* const parent);

    /**
     * Show a setup dialog. Only the specified page will be available.
     */
    static bool execSinglePage(Page page);
    static bool execSinglePage(QWidget* const parent, Page page);

#ifdef HAVE_GEOLOCATION

    static bool execGeolocation(QWidget* const parent, int tab);

#endif

    static void onlineVersionCheck();

private Q_SLOTS:

    void slotOkClicked();
    void slotCancelClicked();
    void slotHelp();

private:

    ShowfotoSetup::Page activePageIndex();
    void showPage(ShowfotoSetup::Page page);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
