/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-28
 * Description : a command line tool to get list of tags from ExifToml database
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include <QVariant>

// Local includes

#include "digikam_debug.h"
#include "exiftoolparser.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    // Create ExifTool parser instance.

    ExifToolParser* const parser = new ExifToolParser(qApp);
    ExifToolParser::ExifToolData parsed;

    if (parser->tagsDatabase())
    {
        parsed           = parser->currentData();
        QStringList keys = parsed.keys();
        keys.sort();
        QString desc;
        QString type;
        QString writable;
        QString output;
        QTextStream stream(&output);
        QStringList tagsLst;

        const int section1 = -65;   // Tag name.
        const int section2 = -10;   // Tag type.
        const int section3 = -15;   // Tag is writable.
        const int section4 = -30;   // Tag description.
        QString sep        = QString().fill(QLatin1Char('-'), qAbs(section1 + section2 + section3 + section4) + 8);

        // Header

        stream << sep
               << Qt::endl
               << QString::fromLatin1("%1").arg(QLatin1String("Name"),        section1) << " | "
               << QString::fromLatin1("%1").arg(QLatin1String("Type"),        section2) << " | "
               << QString::fromLatin1("%1").arg(QLatin1String("Writable"),    section3) << " | "
               << QString::fromLatin1("%1").arg(QLatin1String("Description"), section4)
               << Qt::endl
               << sep
               << Qt::endl;

        for (const QString& tag : std::as_const(keys))
        {
            ExifToolParser::ExifToolData::iterator it = parsed.find(tag);

            if (it != parsed.end())
            {
                desc     = it.value()[0].toString();
                type     = it.value()[1].toString();
                writable = it.value()[2].toString();

                tagsLst
                    << QString::fromLatin1("%1 | %2 | %3 | %4")
                        .arg(tag,      section1)
                        .arg(type,     section2)
                        .arg(writable, section3)
                        .arg(desc,     section4)
                ;
            }
        }

        for (const QString& tag : std::as_const(tagsLst))
        {
            stream << tag << Qt::endl;
        }

        stream << sep << Qt::endl;

        qCDebug(DIGIKAM_TESTS_LOG).noquote() << output;

    }

    return 0;
}
