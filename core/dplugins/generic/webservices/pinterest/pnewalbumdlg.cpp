/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Pinterest web service
 *
 * SPDX-FileCopyrightText: 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "pnewalbumdlg.h"

// Local includes

#include "digikam_debug.h"
#include "pitem.h"

namespace DigikamGenericPinterestPlugin
{

PNewAlbumDlg::PNewAlbumDlg(QWidget* const parent, const QString& toolName)
    : WSNewAlbumDialog(parent, toolName)
{
    hideDateTime();
    hideDesc();
    hideLocation();
    getMainWidget()->setMinimumSize(300, 0);
}

void PNewAlbumDlg::getFolderTitle(PFolder& folder)
{
    folder.title = getTitleEdit()->text();
}

} // namespace DigikamGenericPinterestPlugin

#include "moc_pnewalbumdlg.cpp"
