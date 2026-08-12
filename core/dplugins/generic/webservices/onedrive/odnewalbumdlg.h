/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Onedrive web service
 *
 * SPDX-FileCopyrightText: 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "wsnewalbumdialog.h"

using namespace Digikam;

namespace DigikamGenericOneDrivePlugin
{

class ODFolder;

class ODNewAlbumDlg : public WSNewAlbumDialog
{
    Q_OBJECT

public:

    explicit ODNewAlbumDlg(QWidget* const parent, const QString& toolName);
    ~ODNewAlbumDlg() override = default;

    void getFolderTitle(ODFolder& folder);
};

} // namespace DigikamGenericOneDrivePlugin
