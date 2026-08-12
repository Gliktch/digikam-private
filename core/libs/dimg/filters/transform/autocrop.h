/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-10-18
 * Description : Auto Crop analyzer
 *
 * SPDX-FileCopyrightText: 2013      by Sayantan Datta <sayantan dot knz at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QRect>
#include <QImage>

// Local includes

#include "digikam_export.h"
#include "nrfilter.h"
#include "dimg.h"
#include "dimgthreadedanalyser.h"

namespace Digikam
{

class DIGIKAM_EXPORT AutoCrop : public DImgThreadedAnalyser
{
    Q_OBJECT

public:

    /**
     * Standard constructor with image container to parse
     */
    explicit AutoCrop(DImg* const orgImage, QObject* const parent = nullptr);
    ~AutoCrop() override;

    /**
     * Perform auto-crop analyze to find best inner crop. Use autoInnerCrop()
     * to get computed area.
     */
    void startAnalyse()         override;

    /**
     * Return inner crop area detected by startAnalyse().
     */
    QRect autoInnerCrop() const;

private:

    /**
     * Takes in a binary image and crops it on the basis of black point
     * detection, spirally moving outwards.
     * topCrop can be set to explicitly crop a upper portion of the image
     * bottomCrop can be set to explicitly crop a bottom portion of the image
     */
    QRect spiralClockwiseTraversal(const QImage& source, int topCrop = -1, int bottomCrop = -1);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
