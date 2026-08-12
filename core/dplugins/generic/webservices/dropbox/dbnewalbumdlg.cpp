/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export images to Dropbox web service
 *
 * SPDX-FileCopyrightText: 2013      by Pankaj Kumar <me at panks dot me>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbnewalbumdlg.h"

// Local includes

#include "digikam_debug.h"
#include "dbitem.h"

namespace DigikamGenericDropBoxPlugin
{

DBNewAlbumDlg::DBNewAlbumDlg(QWidget* const parent, const QString& toolName)
    : WSNewAlbumDialog(parent, toolName)
{
    hideDateTime();
    hideDesc();
    hideLocation();
    getMainWidget()->setMinimumSize(300, 0);
}

void DBNewAlbumDlg::getFolderTitle(DBFolder& folder)
{
    folder.title = QLatin1Char('/') + getTitleEdit()->text();
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "getFolderTitle:" << folder.title;
}

} // namespace DigikamGenericDropBoxPlugin

#include "moc_dbnewalbumdlg.cpp"
