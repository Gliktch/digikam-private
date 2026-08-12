/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-07
 * Description : a dialog to create a new remote album for export tools
 *
 * SPDX-FileCopyrightText: 2013      by Pankaj Kumar <me at panks dot me>
 * SPDX-FileCopyrightText: 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "flickrnewalbumdlg.h"

// Qt includes

#include <QFormLayout>
#include <QApplication>
#include <QStyle>

// Local includes

#include "digikam_debug.h"
#include "flickritem.h"

namespace DigikamGenericFlickrPlugin
{

FlickrNewAlbumDlg::FlickrNewAlbumDlg(QWidget* const parent, const QString& toolName)
    : WSNewAlbumDialog(parent, toolName)
{
    hideDateTime();
    hideLocation();
    getMainWidget()->setMinimumSize(350,0);
}

void FlickrNewAlbumDlg::getFolderProperties(FPhotoSet& folder)
{
    folder.title       = getTitleEdit()->text();
    folder.description = getDescEdit()->toPlainText();

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Folder Title "        << folder.title
                                     << " Folder Description " << folder.description;
}

} // namespace DigikamGenericFlickrPlugin

#include "moc_flickrnewalbumdlg.cpp"
