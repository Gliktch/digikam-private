/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-14
 * Description : A QImage loader for DImg framework.
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

DImgQImageLoader::DImgQImageLoader(DImg* const image)
    : DImgLoader(image)
{
}

bool DImgQImageLoader::hasAlpha() const
{
    return m_hasAlpha;
}

bool DImgQImageLoader::sixteenBit() const
{
    return m_sixteenBit;
}

bool DImgQImageLoader::isReadOnly() const
{
    return false;
}

} // namespace DigikamQImageDImgPlugin
