/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Extraction of focus points by exiftool data
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021      by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "focuspoints_extractor.h"

// Local includes

#include "exiftoolparser.h"
#include "digikam_debug.h"

namespace Digikam
{

class Q_DECL_HIDDEN FocusPointsExtractor::Private
{

public:

    Private() = default;

    ListAFPoints                    af_points;                                                  ///< List of AF points extracted from metadata.
    bool                            exifToolAvailable = false;                                  ///< True if ExifTool binary is available.
    ExifToolParser::ExifToolData    metadata;                                                   ///< List of tags parsed by ExifTool.
    bool                            afPointsReadOnly  = true;                                   ///< True if AF points are read-only in metadata.
    QString                         make;                                                       ///< Camera Manufacturer Name
    QString                         model;                                                      ///< Camera Model Name.
    QSize                           originalSize;                                               ///< Original size of image taken by camera
    MetaEngine::ImageOrientation    orientation       = MetaEngine::ORIENTATION_UNSPECIFIED;    ///< Image orientation set by camera.
};

FocusPointsExtractor::FocusPointsExtractor(QObject* const parent,const QString& image_path)
    : QObject(parent),
      d      (new Private)
{
    QScopedPointer<ExifToolParser> const parser(new ExifToolParser(this));

    if (parser->exifToolAvailable())
    {
        parser->load(image_path);
    }

    d->exifToolAvailable = parser->exifToolAvailable();
    d->metadata          = parser->currentData();
    d->make              = findValue(QLatin1String("EXIF.IFD0.Camera.Make")).toString().toUpper();
    d->model             = findValue(QLatin1String("EXIF.IFD0.Camera.Model")).toString().toUpper();

    // NOTE: init image size properties with generic values taken from file by default,
    //       this will be overwritten by delegate with findADPoints().

    QVariant imageWidth  = findValueFirstMatch(QStringList()
                                               << QLatin1String("File.File.Image.ImageWidth")
                                               << QLatin1String("EXIF.IFD0.Image.ImageWidth")
                                               << QLatin1String("EXIF.SubIFD1.Image.ImageWidth")
    );

    QVariant imageHeight = findValueFirstMatch(QStringList()
                                               << QLatin1String("File.File.Image.ImageHeight")
                                               << QLatin1String("EXIF.IFD0.Image.ImageHeight")
                                               << QLatin1String("EXIF.SubIFD1.Image.ImageHeight")
    );

    setOriginalSize(QSize(imageWidth.toInt(), imageHeight.toInt()));

    QVariant direction   = findNumValue(QLatin1String("EXIF.IFD0.Image.Orientation"));
    d->orientation       = direction.isNull() ? MetaEngine::ORIENTATION_UNSPECIFIED
                                              : (MetaEngine::ImageOrientation)direction.toInt();
    qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: orientation:" << d->orientation;

    d->af_points         = findAFPoints();
}

FocusPointsExtractor::~FocusPointsExtractor()
{
    delete d;
}

QVariant FocusPointsExtractor::findValue(const QString& tagName, bool isList) const
{
    QVariantList result = d->metadata.value(tagName);

    if (result.empty())
    {
        return QVariant();
    }

    if (isList)
    {
        QString str = result[0].toString();

        if (str.contains(QLatin1Char(',')))
        {
            return str.split(QLatin1Char(','));
        }
        else
        {
            return str.split(QLatin1Char(' '));
        }
    }
    else
    {
        return result[0];
    }
}

QVariant FocusPointsExtractor::findNumValue(const QString& tagName) const
{
    QVariantList result = d->metadata.value(tagName);

    if (result.empty() || (result.size() < 4))
    {
        return QVariant();
    }

    return result[3];
}

QVariant FocusPointsExtractor::findValue(const QString& tagNameRoot, const QString& key, bool isList) const
{
    return findValue(tagNameRoot + QLatin1String(".") + key, isList);
}

QVariant FocusPointsExtractor::findValueFirstMatch(const QStringList& listTagNames, bool isList) const
{
    for (const QString& tagName : EXIV2_AS_CONST(listTagNames))
    {
        QVariant tmp = findValue(tagName, isList);

        if (!tmp.isNull())
        {
            return tmp;
        }
    }

    return QVariant();
}

QVariant FocusPointsExtractor::findValueFirstMatch(const QString& tagNameRoot, const QStringList& keys, bool isList) const
{
    for (const QString& key : EXIV2_AS_CONST(keys))
    {
        QVariant tmp = findValue(tagNameRoot, key, isList);

        if (!tmp.isNull())
        {
            return tmp;
        }
    }

    return QVariant();
}

FocusPointsExtractor::ListAFPoints FocusPointsExtractor::findAFPoints() const
{
    if (!d->exifToolAvailable)
    {
        return ListAFPoints();
    }

    if (!d->make.isNull())
    {
        if (d->make.contains(QLatin1String("APPLE")))
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: use Apple Exif metadata";

            return getAFPoints_exif();
        }

        if (d->make.contains(QLatin1String("CANON")))
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: use Canon makernotes";

            return getAFPoints_canon();
        }

        if (d->make.contains(QLatin1String("FUJIFILM")))
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: use Fujifilm makernotes";

            return getAFPoints_fujifilm();
        }

        if (d->make.contains(QLatin1String("NIKON")))
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: use Nikon makernotes";

            return getAFPoints_nikon();
        }

        if (
            d->make.contains(QLatin1String("OLYMPUS"))           ||
            d->make.contains(QLatin1String("OM DIGITAL SOLUTIONS"))
           )
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: use Olympus/OMDS makernotes";

            return getAFPoints_olympus();
        }

        if (d->make.contains(QLatin1String("PANASONIC")))
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: use Panasonic makernotes";

            return getAFPoints_panasonic();
        }

        if (d->make.contains(QLatin1String("SONY")))
        {
            qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: use Sony makernotes";

            return getAFPoints_sony();
        }
    }

    qCDebug(DIGIKAM_METAENGINE_LOG) << "FocusPointsExtractor: use Exif or XMP metadata";

    return getAFPoints_exif();
}

FocusPointsExtractor::ListAFPoints FocusPointsExtractor::get_af_points(FocusPoint::TypePoint type)
{
    ListAFPoints points;

    for (const auto& point : EXIV2_AS_CONST(d->af_points))
    {
        if (type == FocusPoint::TypePoint::Inactive)
        {
            if (point.getType() == type)
            {
                points.push_back(point);
            }
        }
        else
        {
            if ((point.getType() & type) == type)
            {
                points.push_back(point);
            }
        }
    }

    return points;
}

FocusPointsExtractor::ListAFPoints FocusPointsExtractor::get_af_points()
{
    return d->af_points;
}

bool FocusPointsExtractor::isAFPointsReadOnly() const
{
    findAFPoints();

    return d->afPointsReadOnly;
}

void FocusPointsExtractor::setAFPointsReadOnly(bool readOnly) const
{
    d->afPointsReadOnly = readOnly;
}

void FocusPointsExtractor::setOriginalSize(const QSize& size) const
{
    d->originalSize = size;
}

QSize FocusPointsExtractor::originalSize() const
{
    return d->originalSize;
}

QString FocusPointsExtractor::make() const
{
    return d->make;
}

QString FocusPointsExtractor::model() const
{
    return d->model;
}

MetaEngine::ImageOrientation FocusPointsExtractor::orientation() const
{
    return d->orientation;
}

} // namespace Digikam

#include "moc_focuspoints_extractor.cpp"
