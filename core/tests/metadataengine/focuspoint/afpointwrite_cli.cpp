/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : a command line tool to write focus points metadata with ExifTool
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QString>
#include <QCoreApplication>

// Local includes

#include "digikam_debug.h"
#include "focuspoints_writer.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    if (argc != 2)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "afpointwrite - CLI tool to write focus points metadata with ExifTool in image";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: <image to write>";
        return -1;
    }

    FocusPointsWriter* const fpwriter = new FocusPointsWriter(qApp, QString::fromUtf8(argv[1]));

    FocusPoint point =
    {
        0.5,
        0.5,
        0.5,
        0.5
    };

    fpwriter->writeFocusPoint(point);

    delete fpwriter;

    return 0;
}
