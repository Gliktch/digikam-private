/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : demo-program for geolocation interface
 *
 * SPDX-FileCopyrightText: 2009-2010 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText:      2014 by Justus Schwartz <justus at gmx dot li>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

// Local includes

#include "digikam_debug.h"
#include "mainwindow.h"
#include "digikam_version.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("demopoints_single"), QLatin1String("Add built-in demo points as single markers")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("demopoints_group"),  QLatin1String("Add built-in demo points as groupable markers")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("single"),            QLatin1String("Do not group the displayed images")));
    parser.addPositionalArgument(QString::fromLatin1("images"), QLatin1String("List of images"), QString::fromLatin1("[images...]"));
    parser.process(app);

    // get the list of images to load on startup:

    QList<QUrl> imagesList;
    const auto files = parser.positionalArguments();

    for (const QString& file : files)
    {
        const QUrl argUrl = QUrl::fromLocalFile(file);
        qCDebug(DIGIKAM_TESTS_LOG) << argUrl;
        imagesList << argUrl;
    }

    MainWindow* const myMainWindow = new MainWindow(&parser);
    myMainWindow->show();
    myMainWindow->slotScheduleImagesForLoading(imagesList);

    return app.exec();
}
