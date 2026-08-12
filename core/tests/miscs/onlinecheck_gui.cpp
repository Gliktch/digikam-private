/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-01-05
 * Description : an unit test to check version online.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QApplication>
#include <QTest>
#include <QDateTime>
#include <QCommandLineParser>

// KDE includes

#include <kaboutdata.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_version.h"
#include "daboutdata.h"
#include "onlineversiondlg.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    if (argc < 3)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "onlinecheck <bool> <bool> - Check if new version is online";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: <bool> 0 for stable release only, 1 for pre-release.";
        qCDebug(DIGIKAM_TESTS_LOG) << "       <bool> 0 without debug symbols, 1 with debug symbols.";
        return -1;
    }

    bool preRelease = QString::fromLatin1(argv[1]).toInt();
    bool withDebug  = QString::fromLatin1(argv[2]).toInt();

    qCDebug(DIGIKAM_TESTS_LOG) << "Check for pre-release     :" << preRelease;
    qCDebug(DIGIKAM_TESTS_LOG) << "Version with debug symbols:" << withDebug;

    KAboutData aboutData(QLatin1String("digikam"),
                         QLatin1String("digiKam"), // No need i18n here.
                         digiKamVersion());

    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    OnlineVersionDlg* const dlg = new OnlineVersionDlg(nullptr,
                                                       QLatin1String("7.0.0"),
                                                       QDateTime::fromString(QLatin1String("2021-01-01T00:00:00"), Qt::ISODate),
                                                       preRelease,
                                                       withDebug);

    return (dlg->exec());
}
