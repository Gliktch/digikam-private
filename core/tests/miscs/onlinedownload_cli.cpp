/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-01-05
 * Description : an unit test to download version online.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QCoreApplication>
#include <QTest>
#include <QObject>

// Local includes

#include "digikam_debug.h"
#include "onlineversiondwnl.h"
#include "onlineversionchecker.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    if (argc < 3)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "onlinecheck <bool> <bool> - Check if new version is online";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: <bool> 0 for stable release only, 1 for pre-release.";
        qCDebug(DIGIKAM_TESTS_LOG) << "       <bool> 0 without debug symbols, 1 with debug symbols.";
        return -1;
    }

    bool preRelease = QString::fromLatin1(argv[1]).toInt();
    bool withDebug  = QString::fromLatin1(argv[2]).toInt();
    QString version;

    qCDebug(DIGIKAM_TESTS_LOG) << "Check for pre-release     :" << preRelease;
    qCDebug(DIGIKAM_TESTS_LOG) << "Version with debug symbols:" << withDebug;

    if (preRelease)
    {
        OnlineVersionChecker* const check = new OnlineVersionChecker(nullptr, preRelease);
        check->setCurrentBuildDate(QDateTime::fromString(QLatin1String("2021-01-01T00:00:00"), Qt::ISODate));
        check->checkForNewVersion();

        QTest::qWait(3000);

        version = check->preReleaseFileName();

        if (version.isEmpty())
        {
            qCWarning(DIGIKAM_TESTS_LOG) << "Cannot get pre-release version!";
            return (-1);
        }
    }
    else
    {
        version = QLatin1String("7.3.0");
    }

    OnlineVersionDwnl* const dwnl = new OnlineVersionDwnl(nullptr, preRelease, withDebug);
    dwnl->startDownload(version);

    QObject::connect(dwnl, &Digikam::OnlineVersionDwnl::signalDownloadProgress,
                     [=](qint64 brecv, qint64 btotal)
        {
            if (btotal)
            {
                qCDebug(DIGIKAM_TESTS_LOG) << "Downloaded" << brecv << "/" << btotal << "bytes" << "(" << brecv*100/btotal << "% )";
            }
        }
    );

    app.exec();

    return 0;
}
