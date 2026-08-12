/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-08-26
 * Description : Tag region formatting
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tagregion.h"

// Qt includes

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

// Local includes

#include "dimg.h"
#include "metaengine.h"

namespace Digikam
{

TagRegion::TagRegion(const QString& descriptor)
    : m_value(descriptor)
{
    QString xmlStartDocument = QLatin1String("<?xml version=\"1.0\"?>");
    QXmlStreamReader reader(xmlStartDocument + descriptor);

    if (reader.readNextStartElement())
    {
        if (reader.name() == QLatin1String("rect"))
        {
            int x = 0;
            int y = 0;
            int w = 0;
            int h = 0;

            if (
                safeToInt(reader.attributes().value(QLatin1String("x")).toString(),      x) &&
                safeToInt(reader.attributes().value(QLatin1String("y")).toString(),      y) &&
                safeToInt(reader.attributes().value(QLatin1String("width")).toString(),  w) &&
                safeToInt(reader.attributes().value(QLatin1String("height")).toString(), h)
               )
            {

                QRect r(x, y, w, h);

                if (r.isValid())
                {
                    m_value = r;
                    m_type  = Rect;
                }
            }
        }
    }
}

TagRegion::TagRegion(const QRect& rect)
    : m_value(rect),
      m_type (Rect)
{
}

TagRegion::Type TagRegion::type() const
{
    return m_type;
}

bool TagRegion::isValid() const
{
    return (m_type != Invalid);
}

bool TagRegion::operator==(const TagRegion& other) const
{
    return (
            (m_type  == other.m_type ) &&
            (m_value == other.m_value)
           );
}

bool TagRegion::operator!=(const TagRegion& other) const
{
    return (!operator==(other));
}

QString TagRegion::toXml() const
{
    if (m_type == Invalid)
    {
        return QString();
    }

    QString output;
    QXmlStreamWriter writer(&output);
    writer.writeStartDocument();
    int lengthOfHeader = output.length();

    if (m_type == Rect)
    {
        QRect rect = m_value.toRect();
        writer.writeStartElement(QLatin1String("rect"));
        writer.writeAttribute(QLatin1String("x"),      QString::number(rect.x()));
        writer.writeAttribute(QLatin1String("y"),      QString::number(rect.y()));
        writer.writeAttribute(QLatin1String("width"),  QString::number(rect.width()));
        writer.writeAttribute(QLatin1String("height"), QString::number(rect.height()));
        writer.writeEndElement();
    }

    // Cut off the <?xml> tag at start of document.

    return output.mid(lengthOfHeader);
}

QRect TagRegion::toRect() const
{
    if (m_type == Rect)
    {
        return m_value.toRect();
    }

    return QRect();
}

QVariant TagRegion::toVariant() const
{
    return m_value;
}

TagRegion TagRegion::fromVariant(const QVariant& var)
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    switch (var.typeId())

#else

    switch (var.type())

#endif

    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        case QMetaType::QRect:

#else

        case QVariant::Rect:

#endif

        {
            return TagRegion(var.toRect());
        }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        case QMetaType::QString:

#else

        case QVariant::String:

#endif

        {
            return TagRegion(var.toString());
        }

        default:
        {
            return TagRegion();
        }
    }
}

bool TagRegion::intersects(const TagRegion& other, double fraction)
{
    if ((m_type == Invalid) || (other.m_type == Invalid))
    {
        return false;
    }

    if (m_type == Rect)
    {
        QRect r = toRect();

        if (other.m_type == Rect)
        {
            QRect r2 = other.toRect();

            if      (fraction == 0)
            {
                return r.intersects(r2);
            }
            else if (fraction == 1)
            {
                return r.contains(r2);
            }
            else
            {
                QRect i = r.intersected(r2);

                return ((double(i.width() * i.height()) / double(r.width() * r.height())) > fraction);
            }
        }
    }

    return false;
}

QRect TagRegion::mapToOriginalSize(const QSize& fullImageSize,
                                   const QSize& reducedImageSize,
                                   const QRect& reducedSizeDetail)
{
    if (fullImageSize == reducedImageSize)
    {
        return reducedSizeDetail;
    }

    double ratioWidth  = double(fullImageSize.width())  / double(reducedImageSize.width());
    double ratioHeight = double(fullImageSize.height()) / double(reducedImageSize.height());

    if (qIsInf(ratioWidth) || qIsInf(ratioHeight))
    {
        return reducedSizeDetail;
    }

    return QRectF(
                  reducedSizeDetail.x()      * ratioWidth,
                  reducedSizeDetail.y()      * ratioHeight,
                  reducedSizeDetail.width()  * ratioWidth,
                  reducedSizeDetail.height() * ratioHeight
                 ).toRect();
}

QRect TagRegion::mapFromOriginalSize(const QSize& fullImageSize, const QSize& reducedImageSize, const QRect& fullSizeDetail)
{
    if (fullImageSize == reducedImageSize)
    {
        return fullSizeDetail;
    }

    double ratioWidth  = double(reducedImageSize.width())  / double(fullImageSize.width());
    double ratioHeight = double(reducedImageSize.height()) / double(fullImageSize.height());

    if (qIsInf(ratioWidth) || qIsInf(ratioHeight))
    {
        return fullSizeDetail;
    }

    return QRectF(
                  fullSizeDetail.x()      * ratioWidth,
                  fullSizeDetail.y()      * ratioHeight,
                  fullSizeDetail.width()  * ratioWidth,
                  fullSizeDetail.height() * ratioHeight
                 ).toRect();
}

QRect TagRegion::mapToOriginalSize(const DImg& reducedSizeImage, const QRect& reducedSizeDetail)
{
    return mapToOriginalSize(reducedSizeImage.originalRatioSize(),
                             reducedSizeImage.size(),
                             reducedSizeDetail);
}

QRect TagRegion::mapFromOriginalSize(const DImg& reducedSizeImage, const QRect& fullSizeDetail)
{
    return mapFromOriginalSize(reducedSizeImage.originalRatioSize(),
                               reducedSizeImage.size(),
                               fullSizeDetail);
}

QRect TagRegion::relativeToAbsolute(const QRectF& region, const QSize& fullSize)
{
    return QRectF(
                  region.x()      * fullSize.width(),
                  region.y()      * fullSize.height(),
                  region.width()  * fullSize.width(),
                  region.height() * fullSize.height()
                 ).toRect();
}

QRect TagRegion::relativeToAbsolute(const QRectF& region, const DImg& reducedSizeImage)
{
    return relativeToAbsolute(region, reducedSizeImage.originalRatioSize());
}

QRectF TagRegion::absoluteToRelative(const QRect& region, const QSize& fullSize)
{
    return QRectF(
                  (qreal)region.x()      / (qreal)fullSize.width(),
                  (qreal)region.y()      / (qreal)fullSize.height(),
                  (qreal)region.width()  / (qreal)fullSize.width(),
                  (qreal)region.height() / (qreal)fullSize.height()
                 );
}

QSize TagRegion::adjustToOrientation(QRect& region, int rotation, const QSize& fullSize)
{
    QSize size = fullSize;

    if      (
             (rotation == MetaEngine::ORIENTATION_ROT_90)       ||
             (rotation == MetaEngine::ORIENTATION_ROT_90_HFLIP) ||
             (rotation == MetaEngine::ORIENTATION_ROT_90_VFLIP)
            )
    {
        region.moveTo(size.height() - region.y() - region.height(), region.x());
        region.setSize(region.size().transposed());
        size.transpose();
    }
    else if (rotation == MetaEngine::ORIENTATION_ROT_180)
    {
        region.moveTo(size.width()  - region.x() - region.width(),
                      size.height() - region.y() - region.height());

    }
    else if (rotation == MetaEngine::ORIENTATION_ROT_270)
    {
        region.moveTo(region.y(), size.width() - region.x() - region.width());
        region.setSize(region.size().transposed());
        size.transpose();
    }

    if      (
             (rotation == MetaEngine::ORIENTATION_HFLIP) ||
             (rotation == MetaEngine::ORIENTATION_ROT_90_HFLIP)
            )
    {
        region.moveTo(size.width() - region.x() - region.width(), region.y());
    }
    else if (
             (rotation == MetaEngine::ORIENTATION_VFLIP) ||
             (rotation == MetaEngine::ORIENTATION_ROT_90_VFLIP)
            )
    {
        region.moveTo(region.x(), size.height() - region.y() - region.height());
    }

    return size;
}

void TagRegion::reverseToOrientation(QRect& region, int orientation, const QSize& fullSize)
{
    QSize size = fullSize;

    switch (orientation)
    {
        case MetaEngine::ORIENTATION_ROT_90_HFLIP:
        {
            TagRegion::adjustToOrientation(region, MetaEngine::ORIENTATION_HFLIP,   size);
            TagRegion::adjustToOrientation(region, MetaEngine::ORIENTATION_ROT_270, size);
            break;
        }

        case MetaEngine::ORIENTATION_ROT_90:
        {
            size.transpose();
            TagRegion::adjustToOrientation(region, MetaEngine::ORIENTATION_ROT_270, size);
            break;
        }

        case MetaEngine::ORIENTATION_ROT_90_VFLIP:
        {
            size.transpose();
            TagRegion::adjustToOrientation(region, MetaEngine::ORIENTATION_VFLIP,   size);
            TagRegion::adjustToOrientation(region, MetaEngine::ORIENTATION_ROT_270, size);
            break;
        }

        case MetaEngine::ORIENTATION_ROT_270:
        {
            size.transpose();
            TagRegion::adjustToOrientation(region, MetaEngine::ORIENTATION_ROT_90,  size);
            break;
        }

        default:
        {
            TagRegion::adjustToOrientation(region, orientation,                     size);
            break;
        }
    }
}

bool TagRegion::safeToInt(const QString& str, int& result)
{
    bool ok;

    result = str.toInt(&ok);

    if (!ok || (result == INT_MIN) || (result == INT_MAX))
    {
        return false;
    }

    return true;
}

QDebug operator<<(QDebug dbg, const TagRegion& r)
{
    QVariant var = r.toVariant();

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    switch (var.typeId())

#else

    switch (var.type())

#endif

    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        case QMetaType::QRect:

#else

        case QVariant::Rect:

#endif

        {
            dbg.nospace() << var.toRect();
            break;
        }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        case QMetaType::QString:

#else

        case QVariant::String:

#endif

        {
            dbg.nospace() << var.toString();
            break;
        }

        default:
        {
            dbg.nospace() << var;
            break;
        }
    }

    return dbg;
}


} // namespace Digikam
