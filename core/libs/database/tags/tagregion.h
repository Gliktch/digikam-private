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

#pragma once

// Qt includes

#include <QRect>
#include <QString>
#include <QVariant>
#include <QDebug>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DImg;

/**
 * Use this small class to convert between the formatted
 * textual representation of a tag region in the database
 * and the corresponding object.
 */
class DIGIKAM_EXPORT TagRegion
{

public:

    enum Type
    {
        Invalid,
        Rect
    };

public:

    /**
     * Construct an invalid region.
     */
    TagRegion() = default;

    /**
     * Construct with the textual descriptor.
     */
    explicit TagRegion(const QString& descriptor);

    /**
     * Construct with the region.
     */
    explicit TagRegion(const QRect& rect);

    Type type()                             const;
    bool isValid()                          const;

    bool operator==(const TagRegion& other) const;

    bool operator!=(const TagRegion& other) const;

    /**
     * Returns an XML textual representation of this region.
     */
    QString toXml()                         const;

    /**
     * If type is Rect, returns the contained rectangle.
     */
    QRect toRect()                          const;

    /**
     * Stores in / loads from a variant. Will only use native QVariant types.
     */
    QVariant toVariant()                    const;
    static TagRegion fromVariant(const QVariant& var);

    /**
     * Returns true if this and the other region intersect.
     * fraction describes the relative overlap area needed to return true:
     * If fraction is 0, returns true if the regions intersect at all.
     * If fraction is 1, returns true only if other is completely contained in this region.
     * If fraction is x, 0 < x < 1, returns true if the area of this region
     * covered by the other is greater than x.
     * Invalid areas never intersect.
     */
    bool intersects(const TagRegion& other, double fraction = 0);

    /**
     * Converts detail rectangles taken from a reduced size image to the original size, and vice versa.
     */
    static QRect mapToOriginalSize(const QSize& fullImageSize, const QSize& reducedImageSize, const QRect& reducedSizeDetail);
    static QRect mapFromOriginalSize(const QSize& fullImageSize, const QSize& reducedImageSize, const QRect& fullSizeDetail);

    /**
     * Takes the original and reduced size from the DImg.
     */
    static QRect mapToOriginalSize(const DImg& reducedSizeImage, const QRect& reducedSizeDetail);
    static QRect mapFromOriginalSize(const DImg& reducedSizeImage, const QRect& fullSizeDetail);

    /**
     * Takes a relative region and a full size and returns the absolute region.
     */
    static QRect relativeToAbsolute(const QRectF& region, const QSize& fullSize);

    /**
     * Takes the original and reduced size from the DImg, maps to original size.
     */
    static QRect relativeToAbsolute(const QRectF& region, const DImg& reducedSizeImage);

    /**
     * Takes absolute region and full size to return the original relative region.
     * Used to write back rectangles into image's XMP. see MetadataHub::write.
     */
    static QRectF absoluteToRelative(const QRect& region, const QSize& fullSize);

    /**
     * Rotate and flip region to MetaEngine::ImageOrientation.
     * The value region are calculated for the new image orientation.
     */
    static QSize adjustToOrientation(QRect& region, int orientation, const QSize& fullSize);

    /**
     * Reverse rotate and flip region to MetaEngine::ImageOrientation.
     * The value region are calculated for the new image orientation.
     */
    static void reverseToOrientation(QRect& region, int orientation, const QSize& fullSize);

protected:

    QVariant m_value;
    Type     m_type = Invalid;

private:

    /**
     * Helper method for a safe conversion of XML rectangle properties to int values.
     */
    bool safeToInt(const QString& str, int& result);
};

QDebug DIGIKAM_EXPORT operator<<(QDebug dbg, const TagRegion& r);

} // namespace Digikam
