/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-07-02
 * Description : Simple program to load a track for timing tests.
 *
 * SPDX-FileCopyrightText: 2014 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QDateTime>
#include <QTextStream>
#include <QCoreApplication>

// Local includes

#include "digikam_debug.h"
#include "trackmanager.h"
#include "trackreader.h"

using namespace Digikam;

namespace
{
    QTextStream qout(stdout);
    QTextStream qerr(stderr);
}

/**
 * @brief Test loading of a GPX file directly
 */
bool testSaxLoader(const QString& filename)
{
    TrackReader::TrackReadResult fileData = TrackReader::loadTrackFile(QUrl::fromLocalFile(filename));

    return fileData.isValid;
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    if (argc < 2)
    {
        qerr << QLatin1String("Need a filename as argument to load") << Qt::endl;
        return 1;
    }

    const QString filename = QString::fromLatin1(argv[1]);
    qerr << "Loading file: " << filename << Qt::endl;
    const bool success     = testSaxLoader(filename);

    if (!success)
    {
        qerr << "Loading failed" << Qt::endl;
        return 1;
    }

    qerr << "Loaded successfully." << Qt::endl;

    return 0;
}
