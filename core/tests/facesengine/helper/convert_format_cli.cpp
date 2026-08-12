/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2020-08-23
 * Description : Convert images format to PNG
 *
 * SPDX-FileCopyrightText: 2020 by Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QImage>
#include <QElapsedTimer>

// Local includes

#include "digikam_debug.h"

QCommandLineParser* parseOptions(const QCoreApplication& app)
{
    QCommandLineParser* const parser = new QCommandLineParser();
    parser->addOption(QCommandLineOption(QLatin1String("source"),      QLatin1String("source folder"),        QLatin1String("path relative to original data folder")));
    parser->addOption(QCommandLineOption(QLatin1String("destination"), QLatin1String("destination location"), QLatin1String("path relative to result data folder")));
    parser->addHelpOption();
    parser->process(app);

    return parser;
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser* const parser = parseOptions(app);

    if (
        !parser->isSet(QLatin1String("source")) &&
        !parser->isSet(QLatin1String("destination"))
    )
    {
        qCWarning(DIGIKAM_TESTS_LOG) << "Folders are not set !!!";

        return 1;
    }

    QDir source(parser->value(QLatin1String("source")));
    QDir destination(parser->value(QLatin1String("destination")));

    if (!source.exists())
    {
        return 1;
    }

    QFileInfoList subDirs = source.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::Name);

    QElapsedTimer timer;
    timer.start();

    for (int i = 0 ; i < subDirs.size() ; ++i)
    {
        QDir subDir(subDirs[i].absoluteFilePath());

        QString path = destination.absolutePath() + QLatin1String("/") + subDir.dirName();

        (void)destination.mkpath(path);

        QFileInfoList filesInfo = subDir.entryInfoList(QDir::Files | QDir::Readable);

        for (int j = 0; j < filesInfo.size(); ++j)
        {
            QImage img(filesInfo[j].absoluteFilePath());

            img.save(path               +
                     QLatin1String("/") +
                     subDir.dirName()   +
                     QLatin1String("_") +
                     QString::number(j) +
                     QLatin1String(".png"),
                     "PNG");
        }
    }

    return 0;
}
