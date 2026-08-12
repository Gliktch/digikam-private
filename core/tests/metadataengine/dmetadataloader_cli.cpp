/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-10-23
 * Description : a command line tool to test DMetadata image loader
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QFileInfo>
#include <QString>
#include <QTextStream>
#include <QApplication>

// Local includes

#include "digikam_debug.h"
#include "dmetadata.h"

using namespace Digikam;

void s_printMetadataMap(const DMetadata::MetaDataMap& map)
{
    QString output;
    QTextStream stream(&output);
    stream << Qt::endl;

    qCDebug(DIGIKAM_TESTS_LOG) << "Found" << map.size() << "tags:" << Qt::endl;

    for (DMetadata::MetaDataMap::const_iterator it = map.constBegin() ;
         it != map.constEnd() ; ++it)
    {
        QString key     = it.key();
        QString value   = it.value();

        QString tagName = key.simplified();
        tagName.append(QString().fill(QLatin1Char(' '), 48 - tagName.length()));

        QString tagVal  = value.simplified();

        if (tagVal.length() > 48)
        {
            tagVal.truncate(48);
            tagVal.append(QString::fromLatin1("... (%1 bytes)").arg(value.length()));
        }

        stream << tagName << " : " << tagVal << Qt::endl;
    }

    qCDebug(DIGIKAM_TESTS_LOG).noquote() << output;
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    if (argc != 2)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "dmetadataloader_cli - CLI tool to check DMetadata image loader";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: <image>";
        return -1;
    }

    MetaEngine::initializeExiv2();

    QFileInfo input(QString::fromUtf8(argv[1]));

    DMetadata meta;
    bool ret = meta.load(input.filePath());

    if (!ret)
    {
        qCWarning(DIGIKAM_TESTS_LOG) << "Cannot load" << meta.getFilePath();
        return -1;
    }

    qCDebug(DIGIKAM_TESTS_LOG) << QString::fromUtf8("-- Exif metadata from %1 --").arg(meta.getFilePath());

    DMetadata::MetaDataMap map = meta.getExifTagsDataList();
    s_printMetadataMap(map);

    qCDebug(DIGIKAM_TESTS_LOG) << QString::fromUtf8("-- Iptc metadata from %1 --").arg(meta.getFilePath());

    map = meta.getIptcTagsDataList();
    s_printMetadataMap(map);

    if (meta.supportXmp())
    {
        qCDebug(DIGIKAM_TESTS_LOG) << QString::fromUtf8("-- Xmp metadata from %1 --").arg(meta.getFilePath());
        map = meta.getXmpTagsDataList();
        s_printMetadataMap(map);
    }
    else
    {
        qCWarning(DIGIKAM_TESTS_LOG) << "Exiv2 has no XMP support...";
    }

    return 0;
}

