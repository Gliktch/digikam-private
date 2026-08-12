/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-07-03
 * Description : Queue common settings container.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020      by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "queuesettings.h"

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_config.h"

namespace Digikam
{

QueueSettings::QueueSettings()
{
    KSharedConfig::Ptr config          = KSharedConfig::openConfig();
    KConfigGroup group                 = config->group(QLatin1String("ImageViewer Settings"));

    ioFileSettings.JPEGCompression     = group.readEntry(QLatin1String("JPEGCompression"),     75);
    ioFileSettings.JPEGSubSampling     = group.readEntry(QLatin1String("JPEGSubSampling"),     1);
    ioFileSettings.PNGCompression      = group.readEntry(QLatin1String("PNGCompression"),      9);
    ioFileSettings.TIFFCompression     = group.readEntry(QLatin1String("TIFFCompression"),     false);

#ifdef HAVE_JASPER

    ioFileSettings.JPEG2000Compression = group.readEntry(QLatin1String("JPEG2000Compression"), 75);
    ioFileSettings.JPEG2000LossLess    = group.readEntry(QLatin1String("JPEG2000LossLess"),    true);

#endif // HAVE_JASPER

    ioFileSettings.PGFCompression      = group.readEntry(QLatin1String("PGFCompression"),      3);
    ioFileSettings.PGFLossLess         = group.readEntry(QLatin1String("PGFLossLess"),         true);

#ifdef HAVE_X265

    ioFileSettings.HEIFCompression     = group.readEntry(QLatin1String("HEIFCompression"),     75);
    ioFileSettings.HEIFLossLess        = group.readEntry(QLatin1String("HEIFLossLess"),        true);

#endif // HAVE_X265

    ioFileSettings.JXLCompression      = group.readEntry(QLatin1String("JXLCompression"),      75);
    ioFileSettings.JXLLossLess         = group.readEntry(QLatin1String("JXLLossLess"),         true);

    ioFileSettings.WEBPCompression     = group.readEntry(QLatin1String("WEBPCompression"),     75);
    ioFileSettings.WEBPLossLess        = group.readEntry(QLatin1String("WEBPLossLess"),        true);

    ioFileSettings.AVIFCompression     = group.readEntry(QLatin1String("AVIFCompression"),     75);
    ioFileSettings.AVIFLossLess        = group.readEntry(QLatin1String("AVIFLossLess"),        true);
}

} // namespace Digikam
