/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-10-23
 * Description : a command line tool to encode images as a
 *               video stream.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include <QCoreApplication>

// Local includes

#include "digikam_debug.h"
#include "metaengine.h"
#include "vidslidethread.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    // ---------------------------------------------
    // Get list of image files from CLI

    VidSlideSettings settings;
    settings.transition = TransitionMngr::HorizontalLines;

    if (argc > 1)
    {
        for (int i = 1 ; i < argc ; ++i)
        {
            settings.inputImages << QUrl::fromLocalFile(QString::fromUtf8(argv[i]));
        }
    }
    else
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "framesencoder - images to encode as video stream";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: <list of image files>";

        return -1;
    }

    // ---------------------------------------------
    // Common settings

    MetaEngine::initializeExiv2();

    VidSlideThread* const encoder = new VidSlideThread(&app);
    encoder->prepareFrames(&settings);
    encoder->start();

    QObject::connect(encoder, SIGNAL(signalComplete(bool)),
                     &app, SLOT(quit()));

    app.exec();

    return 0;
}
