/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-25
 * Description : a tool to convert RAW file to DNG
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dngwriterhost.h"

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DNGWriterHost::DNGWriterHost(DNGWriter::Private* const priv,
                             dng_memory_allocator* const allocator)
    : dng_host(allocator),
      m_priv  (priv)
{
}

void DNGWriterHost::SniffForAbort()
{
    if (m_priv->cancel)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "DNGWriter: Canceled by user..." ;

        m_priv->cleanup();
        ThrowUserCanceled();
    }
}

} // namespace Digikam
