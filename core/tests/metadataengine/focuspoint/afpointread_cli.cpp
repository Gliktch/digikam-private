/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : a command line tool to read focus points metadata with ExifTool
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QString>
#include <QCoreApplication>

// Local includes

#include "digikam_debug.h"
#include "focuspoints_extractor.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    if (argc != 2)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "afpointread - CLI tool to read focus points metadata with ExifTool from image";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: <image to read>";
        return -1;
    }

    FocusPointsExtractor* const fpreader      = new FocusPointsExtractor(qApp, QString::fromUtf8(argv[1]));
    FocusPointsExtractor::ListAFPoints points = fpreader->get_af_points();

    qCDebug(DIGIKAM_TESTS_LOG) << "Make/Model from" << argv[1] << ":" << fpreader->make() << "/" << fpreader->model();
    qCDebug(DIGIKAM_TESTS_LOG) << "Original Image Size:" << fpreader->originalSize();

    if (!points.isEmpty())
    {
        int id = 1;

        for (const FocusPoint& fp : std::as_const(points))
        {
            qCDebug(DIGIKAM_TESTS_LOG) << id << "AF Focus region found in" << argv[1] << ":" << fp;
            qCDebug(DIGIKAM_TESTS_LOG) << id << "AF Focus coordinates in image" << argv[1] << ":" << fp.getRectBySize(fpreader->originalSize());
            ++id;
        }
    }
    else
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "No AF Focus region found in" << argv[1];
    }

    delete fpreader;

    return 0;
}
