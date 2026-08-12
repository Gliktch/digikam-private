/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-06-29
 * Description : a tool to export images to Twitter (x.com) social network
 *
 * SPDX-FileCopyrightText: 2018 by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "twitternewalbumdlg.h"

 // Local includes

#include "digikam_debug.h"
#include "twitteritem.h"

namespace DigikamGenericTwitterPlugin
{

TwNewAlbumDlg::TwNewAlbumDlg(QWidget* const parent, const QString& toolName)
    : WSNewAlbumDialog(parent, toolName)
{
    hideDateTime();
    hideDesc();
    hideLocation();
    getMainWidget()->setMinimumSize(300, 0);
}

void TwNewAlbumDlg::getAlbumProperties(TwAlbum& album)
{
    album.title       = getTitleEdit()->text();
    album.description = getDescEdit()->toPlainText();
}

} // namespace DigikamGenericTwitterPlugin

#include "moc_twitternewalbumdlg.cpp"
