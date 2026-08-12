/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-28
 * Description : a command line tool to check ExifTool with multicore.
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QCoreApplication>
#include <QVariant>
#include <QObject>
#include <QtConcurrentRun>

// Local includes

#include "digikam_debug.h"
#include "exiftoolparser.h"
#include "actionthreadbase.h"

using namespace Digikam;

bool s_exifToolParseThreaded(const QString& file)
{
    ActionThreadBase::setCurrentThreadName(QLatin1String(__FUNCTION__));       // To customize thread name

    QScopedPointer<ExifToolParser> const parser(new ExifToolParser(nullptr));

    // Read metadata from the file. Start ExifToolParser

    if (!parser->load(file))
    {
        qCWarning(DIGIKAM_TESTS_LOG) << "Cannot process" << file;

        return false;
    }

    QString path                         = parser->currentPath();
    ExifToolParser::ExifToolData parsed  = parser->currentData();

    qCDebug(DIGIKAM_TESTS_LOG).noquote() << "Processing source file:" << path;

    // Print returned and sorted tags.

    QFileInfo fi(file);
    QFile output(QString::fromLatin1("%1-exiftool.txt").arg(fi.fileName()));

    if (!output.open(QIODevice::WriteOnly))
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "Cannot open ExifTool output file to write...";

        return false;
    }

    QTextStream stream(&output);
    QStringList tagsLst;

    const int section1 = -40;   // ExifTool Tag name simplified
    const int section2 = -30;   // Tag value as string.
    QString sep        = QString().fill(QLatin1Char('-'), qAbs(section1 + section2) + 4);

    // Header

    stream << sep
           << Qt::endl
           << QString::fromLatin1("%1").arg(QLatin1String("ExifTool::group0.name"), section1) << " | "
           << QString::fromLatin1("%1").arg(QLatin1String("String Value"),          section2)
           << Qt::endl
           << sep
           << Qt::endl;

    for (ExifToolParser::ExifToolData::const_iterator it = parsed.constBegin() ;
         it != parsed.constEnd() ; ++it)
    {
        QString tagNameExifTool = it.key().section(QLatin1Char('.'), 0, 0) +
                                  QLatin1Char('.')                         +
                                  it.key().section(QLatin1Char('.'), -1);
        QString tagType         = it.value()[1].toString();
        QString data            = it.value()[0].toString();
        Q_UNUSED(tagType);

        if (data.size() > -section2)
        {
            data = data.left(-section2 - 3) + QLatin1String("...");
        }

        tagsLst
                << QString::fromLatin1("%1 | %2")
                .arg(tagNameExifTool, section1)
                .arg(data,            section2)
               ;
    }

    tagsLst.sort();

    for (const QString& tag : std::as_const(tagsLst))
    {
        stream << tag << Qt::endl;
    }

    stream << sep << Qt::endl;

    output.close();

    qCDebug(DIGIKAM_TESTS_LOG).noquote() << "Processed source file:" << path;

    return true;
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    if (argc != 2)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "exiftooloutpu_cli - CLI tool to check ExifTool with multicore";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: <dir>";

        return -1;
    }

    QDir imageDir(QString::fromUtf8(argv[1]));
    imageDir.setNameFilters(QStringList() << QLatin1String("*.jpg"));
    QStringList imageFiles = imageDir.entryList();

    qCDebug(DIGIKAM_TESTS_LOG) << "ExifTool parsing images:" << imageFiles;

    QList <QFuture<bool> > tasks;

    for (const QString& imageFile : std::as_const(imageFiles))
    {
        tasks.append(QtConcurrent::run(
                                       &s_exifToolParseThreaded,
                                       imageDir.path() + QLatin1Char('/') + imageFile
                                      )
        );
    }

    bool result = false;

    for (QFuture<bool> t : std::as_const(tasks))
    {
        t.waitForFinished();
        result |= t.result();
    }

    qCDebug(DIGIKAM_TESTS_LOG) << "ExifTool parsing completed:" << result;

    return 0;
}
