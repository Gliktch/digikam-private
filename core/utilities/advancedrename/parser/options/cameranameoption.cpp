/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to provide camera information to the parser
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "cameranameoption.h"

// Qt includes

#include <QScopedPointer>
#include <QRegularExpression>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "parser.h"
#include "dmetadata.h"

namespace Digikam
{

CameraNameOption::CameraNameOption()
    : Option(i18n("Camera"),
             i18n("Add the camera name"),
             QLatin1String("camera-photo"))
{
    QString token(QLatin1String("[cam]"));
    addToken(token, i18n("Camera name"));

    QRegularExpression reg(escapeToken(token));
    reg.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    setRegExp(reg);
}

QString CameraNameOption::parseOperation(ParseSettings& settings, const QRegularExpressionMatch& /*match*/)
{
    QString result;

    ItemInfo info = ItemInfo::fromUrl(settings.fileUrl);

    if (!info.isNull())
    {
        result = info.photoInfoContainer().make + QLatin1Char(' ') + info.photoInfoContainer().model;
    }
    else
    {
        // If ItemInfo is not available, read the information from the EXIF data

        QString make;
        QString model;

        QScopedPointer<DMetadata> meta(new DMetadata(settings.fileUrl.toLocalFile()));

        if (!meta->isEmpty())
        {
            MetaEngine::MetaDataMap dataMap;
            dataMap         = meta->getExifTagsDataList(QStringList(), true);
            const auto keys = dataMap.keys();

            for (const QString& key : keys)
            {
                if      (key.toLower().contains(QLatin1String("exif.image.model")))
                {
                    model = dataMap[key];
                }
                else if (key.toLower().contains(QLatin1String("exif.image.make")))
                {
                    make = dataMap[key];
                }
            }
        }

        result = make + QLatin1Char(' ') + model;
    }

    result.replace(QLatin1Char('/'), QLatin1Char('-'));

    return result.simplified();
}

} // namespace Digikam

#include "moc_cameranameoption.cpp"
