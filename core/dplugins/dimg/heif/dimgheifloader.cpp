/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-26
 * Description : A HEIF IO file for DImg framework
 *
 * SPDX-FileCopyrightText: 2019-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dimgheifloader.h"

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "dimgloaderobserver.h"

namespace Digikam
{

DImgHEIFLoader::DImgHEIFLoader(DImg* const image)
    : DImgLoader(image)
{
}

bool DImgHEIFLoader::hasAlpha() const
{
    return m_hasAlpha;
}

bool DImgHEIFLoader::sixteenBit() const
{
    return m_sixteenBit;
}

bool DImgHEIFLoader::isReadOnly() const
{

#ifdef HAVE_X265

    return false;

#else

    return true;

#endif

}

bool DImgHEIFLoader::isHeifSuccess(const struct heif_error* const error)
{
    if (error->code == 0)
    {
        return true;
    }

    qCWarning(DIGIKAM_DIMG_LOG_HEIF) << "Error while processing HEIF image:" << error->message;

    return false;
}

} // namespace Digikam
