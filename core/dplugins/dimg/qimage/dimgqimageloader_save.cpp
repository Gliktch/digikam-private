/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : A QImage loader for DImg framework - save method.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2024 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgqimageloader.h"

// Qt includes

#include <QImage>
#include <QByteArray>
#include <QColorSpace>
#include <QImageReader>

// Local includes

#include "digikam_debug.h"
#include "dimgloaderobserver.h"

namespace DigikamQImageDImgPlugin
{

bool DImgQImageLoader::save(const QString& filePath, DImgLoaderObserver* const observer)
{
    QVariant qualityAttr = imageGetAttribute(QLatin1String("quality"));
    int quality          = qualityAttr.isValid() ? qualityAttr.toInt() : 90;

    if      (quality > 100)
    {
        quality = 100;
    }
    else if (quality < 0)
    {
        quality = 90;
    }

    QVariant formatAttr      = imageGetAttribute(QLatin1String("format"));
    QByteArray format        = formatAttr.toByteArray();
    QImage image             = m_image->copyQImage();
    QByteArray iccRawProfile = m_image->getIccProfile().data();

    if (!iccRawProfile.isEmpty())
    {
        image.setColorSpace(QColorSpace::fromIccProfile(iccRawProfile));
    }

    if (observer)
    {
        observer->progressInfo(0.1F);
    }

    // Saving is opaque to us. No support for stopping from observer,
    // progress info are only pseudo values

    bool success = image.save(filePath, format.toUpper().constData(), quality);

    if (observer && success)
    {
        observer->progressInfo(1.0F);
    }

    imageSetAttribute(QLatin1String("format"), format.toUpper());

    saveMetadata(filePath);

    return success;
}

} // namespace DigikamQImageDImgPlugin
