/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Writer of focus points to exiftool data
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "focuspoints_writer.h"

// Qt includes

#include <QScopedPointer>

// Local includes

#include "exiftoolparser.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN FocusPointsWriter::Private
{
public:

    Private() = default;

    ExifToolParser* exifTool = nullptr;
    QString         path;
};

FocusPointsWriter::FocusPointsWriter(QObject* const parent, const QString& path)
    : QObject(parent),
      d      (new Private)
{
    d->exifTool = new ExifToolParser(this);
    d->exifTool->load(path);
    d->path     = path;
}

FocusPointsWriter::~FocusPointsWriter()
{
    delete d;
}

void FocusPointsWriter::writeFocusPoint(const FocusPoint& point)
{
    ExifToolParser::ExifToolData newTags;

    QPointF pos = point.getCenterPosition();
    QSizeF size = point.getSize();

    newTags.insert(QLatin1String("xmp:RegionAreaX"),
                   QVariantList() << QString::number(pos.x())
                                  << QString()
                                  << QString::fromUtf8("Region Area X"));

    newTags.insert(QLatin1String("xmp:RegionAreaY"),
                   QVariantList() << QString::number(pos.y())
                                  << QString()
                                  << QString::fromUtf8("Region Area Y"));

    newTags.insert(QLatin1String("xmp:RegionAreaH"),
                   QVariantList() << QString::number(size.height())
                                  << QString()
                                  << QString::fromUtf8("Region Area Height"));

    newTags.insert(QLatin1String("xmp:RegionAreaW"),
                   QVariantList() << QString::number(size.width())
                                  << QString()
                                  << QString::fromUtf8("Region Area Width"));

    newTags.insert(QLatin1String("xmp:RegionDescription"),
                   QVariantList() << QString::fromUtf8("digikam customized auto focus point")
                                  << QString()
                                  << QString::fromUtf8("Region description"));

    d->exifTool->applyChanges(d->path, newTags);
}

void FocusPointsWriter::writeFocusPoint(const QRectF& rectF)
{
    FocusPoint point
    {
        static_cast<float> (rectF.topLeft().x() + 0.5 * rectF.width()),
        static_cast<float> (rectF.topLeft().y() + 0.5 * rectF.height()),
        static_cast<float> (rectF.width()), static_cast<float>(rectF.height())
    };

    writeFocusPoint(point);
}

} // namespace Digikam

#include "moc_focuspoints_writer.cpp"
