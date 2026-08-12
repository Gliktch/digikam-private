/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-10-22
 * Description : a widget to filter album contents by type mime
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mimefilter.h"

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

MimeFilter::MimeFilter(QWidget* const parent)
    : QComboBox(parent)
{
    insertItem(AllFiles,       i18n("All Files"));
    insertItem(ImageFiles,     i18n("Image Files"));
    insertItem(RAWFiles,       i18n("RAW Files"));
    insertItem(DNGFiles,       i18n("DNG Files"));
    insertItem(NoRAWFiles,     i18n("No RAW Files"));
    insertItem(JPGFiles,       i18n("JPEG Files"));
    insertItem(JPEG2000Files,  i18n("JPEG-2000 Files"));
    insertItem(JPEGXLFiles,    i18n("JPEG-XL Files"));
    insertItem(WEBPFiles,      i18n("WEBP Files"));
    insertItem(PNGFiles,       i18n("PNG Files"));
    insertItem(TIFFiles,       i18n("TIFF Files"));
    insertItem(PGFFiles,       i18n("PGF Files"));
    insertItem(HEIFFiles,      i18n("HEIF Files"));
    insertItem(AVIFFiles,      i18n("AVIF Files"));
    insertItem(MoviesFiles,    i18n("Video Files"));
    insertItem(AudioFiles,     i18n("Audio Files"));
    insertItem(RasterGraphics, i18n("Raster Graphics"));

    setToolTip(i18n("Filter by file type"));
    setWhatsThis(i18n("Select the file types (MIME types) that you want shown. "
                      "Note: \"Raster Graphics\" are file formats from raster graphics editors, "
                      "such as Photoshop, The Gimp, Krita, etc."));

    setMimeFilter(AllFiles);
}

void MimeFilter::setMimeFilter(int filter)
{
    setCurrentIndex(filter);

    Q_EMIT activated(filter);
}

int MimeFilter::mimeFilter()
{
    return currentIndex();
}

} // namespace Digikam

#include "moc_mimefilter.cpp"
