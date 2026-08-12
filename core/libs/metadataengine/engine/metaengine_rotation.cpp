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

// Local includes

#include "metaengine_rotation.h"

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

/**
 * If the picture is displayed according to the exif orientation tag,
 * the user will request rotating operations relative to what he sees,
 * and that is the picture rotated according to the EXIF tag.
 * So the operation requested and the given EXIF angle must be combined.
 * E.g. if orientation is "6" (rotate 90 clockwiseto show correctly)
 * and the user selects 180 clockwise, the operation is 270.
 * If the user selected 270, the operation would be None (and clearing the exif tag).
 *
 * This requires to describe the transformations in a model which
 * cares for both composing (180+90=270) and eliminating (180+180=no action),
 * as well as the non-commutative nature of the operations (vflip+90 is not 90+vflip)
 *
 * All 2D transformations can be described by a 2x3 matrix, see QWMetaEngineRotation.
 * All transformations needed here - rotate 90, 180, 270, flipV, flipH -
 * can be described in a 2x2 matrix with the values 0,1,-1
 * (because flipping is expressed by changing the sign only,
 *  and sine and cosine of 90, 180 and 270 are either 0,1 or -1).
 *
 *  x' = m11 x + m12 y
 *  y' = m21 x + m22 y
 *
 * Moreover, all combinations of these rotate/flip operations result in one of the eight
 * matrices defined below. This did not proof that mathematically, but empirically.
 */

namespace Matrix
{

static const MetaEngineRotation identity               ( 1,  0,  0,  1);
static const MetaEngineRotation rotate90               ( 0,  1, -1,  0);
static const MetaEngineRotation rotate180              (-1,  0,  0, -1);
static const MetaEngineRotation rotate270              ( 0, -1,  1,  0);
static const MetaEngineRotation flipHorizontal         (-1,  0,  0,  1);
static const MetaEngineRotation flipVertical           ( 1,  0,  0, -1);
static const MetaEngineRotation rotate90flipHorizontal ( 0,  1,  1,  0);    ///< first rotate, then flip
static const MetaEngineRotation rotate90flipVertical   ( 0, -1, -1,  0);    ///< first rotate, then flip

MetaEngineRotation matrix(MetaEngineRotation::TransformationAction action)
{
    switch (action)
    {
        case MetaEngineRotation::NoTransformation:
        {
            return identity;
        }

        case MetaEngineRotation::FlipHorizontal:
        {
            return flipHorizontal;
        }

        case MetaEngineRotation::FlipVertical:
        {
            return flipVertical;
        }

        case MetaEngineRotation::Rotate90:
        {
            return rotate90;
        }

        case MetaEngineRotation::Rotate180:
        {
            return rotate180;
        }

        case MetaEngineRotation::Rotate270:
        {
            return rotate270;
        }

        default:
        {
            break;
        }
    }

    return identity;
}

MetaEngineRotation matrix(MetaEngine::ImageOrientation exifOrientation)
{
    switch (exifOrientation)
    {
        case MetaEngine::ORIENTATION_NORMAL:
        {
            return identity;
        }

        case MetaEngine::ORIENTATION_HFLIP:
        {
            return flipHorizontal;
        }

        case MetaEngine::ORIENTATION_ROT_180:
        {
            return rotate180;
        }

        case MetaEngine::ORIENTATION_VFLIP:
        {
            return flipVertical;
        }

        case MetaEngine::ORIENTATION_ROT_90_HFLIP:
        {
            return rotate90flipHorizontal;
        }

        case MetaEngine::ORIENTATION_ROT_90:
        {
            return rotate90;
        }

        case MetaEngine::ORIENTATION_ROT_90_VFLIP:
        {
            return rotate90flipVertical;
        }

        case MetaEngine::ORIENTATION_ROT_270:
        {
            return rotate270;
        }

        case MetaEngine::ORIENTATION_UNSPECIFIED:
        {
            return identity;
        }
    }

    return identity;
}

} // namespace Matrix

MetaEngineRotation::MetaEngineRotation()
{
    set(1, 0, 0, 1);
}

MetaEngineRotation::MetaEngineRotation(TransformationAction action)
{
    *this = Matrix::matrix(action);
}

MetaEngineRotation::MetaEngineRotation(MetaEngine::ImageOrientation exifOrientation)
{
    *this = Matrix::matrix(exifOrientation);
}

MetaEngineRotation::MetaEngineRotation(int m11, int m12, int m21, int m22)
{
    set(m11, m12, m21, m22);
}

void MetaEngineRotation::set(int m11, int m12, int m21, int m22)
{
    m[0][0] = m11;
    m[0][1] = m12;
    m[1][0] = m21;
    m[1][1] = m22;
}

bool MetaEngineRotation::isNoTransform() const
{
    return (*this == Matrix::identity);
}

MetaEngineRotation& MetaEngineRotation::operator*=(const MetaEngineRotation& ma)
{
    set(
        ma.m[0][0]*m[0][0] + ma.m[0][1]*m[1][0],  ma.m[0][0]*m[0][1] + ma.m[0][1]*m[1][1],
        ma.m[1][0]*m[0][0] + ma.m[1][1]*m[1][0],  ma.m[1][0]*m[0][1] + ma.m[1][1]*m[1][1]
       );

    return *this;
}

bool MetaEngineRotation::operator==(const MetaEngineRotation& ma) const
{
    return (
            (m[0][0] == ma.m[0][0]) &&
            (m[0][1] == ma.m[0][1]) &&
            (m[1][0] == ma.m[1][0]) &&
            (m[1][1] == ma.m[1][1])
           );
}

bool MetaEngineRotation::operator!=(const MetaEngineRotation& ma) const
{
    return !(*this==ma);
}

MetaEngineRotation& MetaEngineRotation::operator*=(TransformationAction action)
{
    return (*this *= Matrix::matrix(action));
}

MetaEngineRotation& MetaEngineRotation::operator*=(const QList<TransformationAction>& actions)
{
    for (const TransformationAction& action : EXIV2_AS_CONST(actions))
    {
        *this *= Matrix::matrix(action);
    }

    return *this;
}

MetaEngineRotation& MetaEngineRotation::operator*=(MetaEngine::ImageOrientation exifOrientation)
{
    return (*this *= Matrix::matrix(exifOrientation));
}

/**
 * Converts the mathematically correct description
 * into the primitive operations that can be carried out losslessly.
 */
QList<MetaEngineRotation::TransformationAction> MetaEngineRotation::transformations() const
{
    QList<TransformationAction> transforms;

    if      (*this == Matrix::rotate90)
    {
        transforms << Rotate90;
    }
    else if (*this == Matrix::rotate180)
    {
        transforms << Rotate180;
    }
    else if (*this == Matrix::rotate270)
    {
        transforms << Rotate270;
    }
    else if (*this == Matrix::flipHorizontal)
    {
        transforms << FlipHorizontal;
    }
    else if (*this == Matrix::flipVertical)
    {
        transforms << FlipVertical;
    }
    else if (*this == Matrix::rotate90flipHorizontal)
    {
        // first rotate, then flip!

        transforms << Rotate90;
        transforms << FlipHorizontal;
    }
    else if (*this == Matrix::rotate90flipVertical)
    {
        // first rotate, then flip!

        transforms << Rotate90;
        transforms << FlipVertical;
    }

    return transforms;
}

MetaEngine::ImageOrientation MetaEngineRotation::exifOrientation() const
{
    if (*this == Matrix::identity)
    {
        return MetaEngine::ORIENTATION_NORMAL;
    }

    if      (*this == Matrix::rotate90)
    {
        return MetaEngine::ORIENTATION_ROT_90;
    }
    else if (*this == Matrix::rotate180)
    {
        return MetaEngine::ORIENTATION_ROT_180;
    }
    else if (*this == Matrix::rotate270)
    {
        return MetaEngine::ORIENTATION_ROT_270;
    }
    else if (*this == Matrix::flipHorizontal)
    {
        return MetaEngine::ORIENTATION_HFLIP;
    }
    else if (*this == Matrix::flipVertical)
    {
        return MetaEngine::ORIENTATION_VFLIP;
    }
    else if (*this == Matrix::rotate90flipHorizontal)
    {
        return MetaEngine::ORIENTATION_ROT_90_HFLIP;
    }
    else if (*this == Matrix::rotate90flipVertical)
    {
        return MetaEngine::ORIENTATION_ROT_90_VFLIP;
    }

    return MetaEngine::ORIENTATION_UNSPECIFIED;
}

QTransform MetaEngineRotation::toTransform() const
{
    return toTransform(exifOrientation());
}

QTransform MetaEngineRotation::toTransform(MetaEngine::ImageOrientation orientation)
{
    QTransform matrix;

    switch (orientation)
    {
        case MetaEngine::ORIENTATION_NORMAL:
        case MetaEngine::ORIENTATION_UNSPECIFIED:
        {
            break;
        }

        case MetaEngine::ORIENTATION_HFLIP:
        {
            matrix.scale(-1, 1);
            break;
        }

        case MetaEngine::ORIENTATION_ROT_180:
        {
            matrix.rotate(180);
            break;
        }

        case MetaEngine::ORIENTATION_VFLIP:
        {
            matrix.scale(1, -1);
            break;
        }

        case MetaEngine::ORIENTATION_ROT_90_HFLIP:
        {
            matrix.scale(-1, 1);
            matrix.rotate(90);
            break;
        }

        case MetaEngine::ORIENTATION_ROT_90:
        {
            matrix.rotate(90);
            break;
        }

        case MetaEngine::ORIENTATION_ROT_90_VFLIP:
        {
            matrix.scale(1, -1);
            matrix.rotate(90);
            break;
        }

        case MetaEngine::ORIENTATION_ROT_270:
        {
            matrix.rotate(270);
            break;
        }
    }

    return matrix;
}

/**
 * Converts a MetaEngineRotation::TransformationAction to a QString description.
 */
QString MetaEngineRotation::transformationActionToString(TransformationAction action)
{
    switch (action)
    {
        case NoTransformation:
        {
            return i18n("No Transformation");
        }

        case FlipHorizontal:
        {
            return i18n("Flip Horizontal");
        }

        case FlipVertical:
        {
            return i18n("Flip Vertical");
        }

        case Rotate90:
        {
            return i18n("Rotate 90°");
        }

        case Rotate180:
        {
            return i18n("Rotate 180°");
        }

        case Rotate270:
        {
            return i18n("Rotate 270°");
        }

        case RotateAuto:
        {
            return i18n("Auto Rotation");
        }

        default:
        {
            return i18n("Unknown Transformation");
        }
    }
}

} // namespace Digikam
