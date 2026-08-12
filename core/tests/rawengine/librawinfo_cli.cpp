/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-09-07
 * Description : a command line tool to show RawEngine info
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QString>

// Local includes

#include "drawdecoder.h"
#include "digikam_debug.h"

using namespace Digikam;

int main(int /*argc*/, char** /*argv*/)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Libraw version    : " << DRawDecoder::librawVersion();
    qCDebug(DIGIKAM_TESTS_LOG) << "Use OpenMP        : " << DRawDecoder::librawUseGomp();
    qCDebug(DIGIKAM_TESTS_LOG) << "Raw files list    : " << DRawDecoder::rawFilesList();
    qCDebug(DIGIKAM_TESTS_LOG) << "Raw files version : " << DRawDecoder::rawFilesVersion();
    qCDebug(DIGIKAM_TESTS_LOG) << "Supported camera  : " << DRawDecoder::supportedCamera();

    return 0;
}
