/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2010-09-02
 * Description : A convenience class for a standalone face detector
 *
 * SPDX-FileCopyrightText: 2010      by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2024-2025 by Michael Miller <michael underscore miller at msn dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QExplicitlySharedDataPointer>
#include <QImage>
#include <QVariant>

// Local includes

#include "digikam_export.h"
#include "dimg.h"
#include "facescansettings.h"

namespace Digikam
{

class DIGIKAM_EXPORT FaceDetector
{
public:

    /**
     * @brief Provides face detection, that means the process of selecting
     * those regions of a full image which contain face.
     *
     * This class provides shallow copying
     * The class is fully reentrant (a single object and its copies are not thread-safe).
     * Deferred creation is guaranteed, that means creation of a FaceDetector
     * object is cheap, the expensive creation of the detection backend
     * is performed when detectFaces is called for the first time.
     */
    FaceDetector();
    FaceDetector(const FaceDetector& other);
    ~FaceDetector();

    QString backendIdentifier()                                     const;

    FaceDetector& operator=(const FaceDetector& other);

    /**
     * @brief Scan an image for faces. Return a list with regions possibly
     * containing faces.
     * If the image has been downscaled anywhere in the process,
     * provide the original size of the image as this may be of importance in the detection process.
     *
     * @return Found faces are returned in relative coordinates.
     */
    QList<QRectF> detectFaces(const QImage& image, const QSize& originalSize = QSize());

    /**
     * @brief Scan an image for faces. Return a list with regions possibly
     * containing faces.
     * If the image has been downscaled anywhere in the process,
     * provide the original size of the image as this may be of importance in the detection process.
     *
     * @return Found faces are returned in relative coordinates.
     */
    QList<QRectF> detectFaces(const DImg& image, const QSize& originalSize = QSize());

    QList<QRectF> detectFaces(const QString& imagePath);

    /**
     * @brief Tunes backend parameters.
     * Available parameters:
     *
     * "speed"       vs. "accuracy",    0..1, float
     * "sensitivity" vs. "specificity", 0..1, float.
     *
     * For both pairs: a = 1-b, you can set either.
     * The first pair changes the ROC curve in a trade for computing time.
     * The second pair moves on a given ROC curve towards more false positives, or more missed faces.
     */
    void        setParameter(const QString& parameter, const QVariant& value);
    void        setParameters(const QVariantMap& parameters);
    QVariantMap parameters()                                        const;

    /**
     * @return the recommended size if you want to scale images for detection.
     * Larger images can be passed, but may be downscaled.
     */
    int recommendedImageSize(const QSize& availableSize = QSize())  const;

    static QRectF        toRelativeRect(const QRect& absoluteRect, const QSize& size);
    static QRect         toAbsoluteRect(const QRectF& relativeRect, const QSize& size);
    static QList<QRectF> toRelativeRects(const QList<QRect>& absoluteRects, const QSize& size);
    static QList<QRect>  toAbsoluteRects(const QList<QRectF>& relativeRects, const QSize& size);

private:

    class Private;
    QExplicitlySharedDataPointer<Private> d;
};

} // namespace Digikam
