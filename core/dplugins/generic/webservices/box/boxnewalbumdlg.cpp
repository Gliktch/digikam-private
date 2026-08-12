/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Box web service
 *
 * SPDX-FileCopyrightText: 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "boxnewalbumdlg.h"

// Local includes

#include "digikam_debug.h"
#include "boxitem.h"

namespace DigikamGenericBoxPlugin
{

BOXNewAlbumDlg::BOXNewAlbumDlg(QWidget* const parent, const QString& toolName)
    : WSNewAlbumDialog(parent, toolName)
{
    hideDateTime();
    hideDesc();
    hideLocation();
    getMainWidget()->setMinimumSize(300, 0);
}

void BOXNewAlbumDlg::getFolderTitle(BOXFolder& folder)
{
    folder.title = QLatin1Char('/') + getTitleEdit()->text();
}

} // namespace DigikamGenericBoxPlugin

#include "moc_boxnewalbumdlg.cpp"
