/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Tools for combining rotation operations.
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QTransform>

// Local includes

#include "digikam_export.h"
#include "metaengine.h"

namespace Digikam
{

class DIGIKAM_EXPORT MetaEngineRotation
{
    Q_GADGET

public:

    /**
     * This describes single transform primitives.
     * Note some of the defined Exif rotation flags combine
     * two of these actions.
     * The enum values correspond to those defined
     * as JXFORM_CODE in the often used the JPEG tool transupp.h.
     */
    enum TransformationAction
    {
        NoTransformation = 0, ///< no transformation
        FlipHorizontal   = 1, ///< horizontal flip
        FlipVertical     = 2, ///< vertical flip
        Rotate90         = 5, ///< 90-degree clockwise rotation
        Rotate180        = 6, ///< 180-degree rotation
        Rotate270        = 7, ///< 270-degree clockwise (or 90 ccw)
        RotateAuto       = 8  ///< auto-rotate based on AutoRotator class
    };
    Q_ENUM(TransformationAction)

public:

    /**
     * Constructs the identity matrix (the matrix describing no transformation)
     */
    MetaEngineRotation();

    /**
     * Returns the matrix corresponding to the given TransformationAction
     */
    explicit MetaEngineRotation(TransformationAction action);

    /**
     * Returns the matrix corresponding to the given TransformationAction
     */
    explicit MetaEngineRotation(MetaEngine::ImageOrientation exifOrientation);

    bool operator==(const MetaEngineRotation& ma)   const;
    bool operator!=(const MetaEngineRotation& ma)   const;

    /**
     * Returns true of this matrix describes no transformation (is the identity matrix)
     */
    bool isNoTransform()                            const;

    MetaEngineRotation& operator*=(const MetaEngineRotation& ma);

    /**
     * Applies the given transform to this matrix
     */
    MetaEngineRotation& operator*=(TransformationAction action);

    /**
     * Applies the given transform actions to this matrix
     */
    MetaEngineRotation& operator*=(const QList<TransformationAction>& actions);

    /**
     * Applies the given Exif orientation flag to this matrix
     */
    MetaEngineRotation& operator*=(MetaEngine::ImageOrientation exifOrientation);

    MetaEngineRotation(int m11, int m12, int m21, int m22);

    /**
     * Returns the actions described by this matrix. The order matters.
     * Not all possible matrices are supported, but all those that can be combined
     * by Exif rotation flags and the transform actions above.
     * If isNoTransform() or the matrix is not supported returns an empty list.
     */
    QList<TransformationAction> transformations()   const;

    /**
     * Returns the Exif orientation flag describing this matrix.
     * Returns ORIENTATION_UNSPECIFIED if no flag matches this matrix.
     */
    MetaEngine::ImageOrientation exifOrientation()  const;

    /**
     * Returns a QTransform representing this matrix
     */
    QTransform toTransform()                        const;

    /**
     * Returns a QTransform for the given Exif orientation
     */
    static QTransform toTransform(MetaEngine::ImageOrientation orientation);

    /**
     * Converts a TransformationAction to a localized QString description.
     * This is useful for debugging and UI display purposes.
     */
    static QString transformationActionToString(TransformationAction action);

protected:

    void set(int m11, int m12, int m21, int m22);

protected:

    int m[2][2];
};

} // namespace Digikam
