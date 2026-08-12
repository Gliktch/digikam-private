/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Wavelets Noise Reduction threaded image filter.
 *               This filter work in YCrCb color space.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QMetaType>
#include <QDebug>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT NRContainer
{

public:

    NRContainer()  = default;
    ~NRContainer() = default;

public:

    /**
     * Separated values per channel
     */
    double thresholds[3] = { 1.2 };    ///< Y, Cb, Cr thresholds.
    double softness[3]   = { 0.9 };    ///< Y, Cb, Cr softness.
};

//! qDebug() stream operator. Writes property @a inf to the debug output in a nicely formatted way.
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const NRContainer& inf);

// --------------------------------------------------------------------------

class DIGIKAM_EXPORT NRFilter : public DImgThreadedFilter
{
    Q_OBJECT

private:

    class Q_DECL_HIDDEN Args
    {
    public:

        Args() = default;

        uint    start       = 0;
        uint    stop        = 0;
        float*  thold       = nullptr;
        uint*   lpass       = nullptr;
        uint*   hpass       = nullptr;
        double* stdev       = nullptr;
        uint*   samples     = nullptr;
        float** fimg        = nullptr;
        float   threshold   = 0.0F;
        double  softness    = 0.0;
    };

public:

    explicit NRFilter(QObject* const parent = nullptr);
    NRFilter(DImg* const orgImage, QObject* const parent, const NRContainer& settings);
    ~NRFilter()                                           override;

    void readParameters(const FilterAction& action)       override;

    FilterAction filterAction()                           override;
    QString filterIdentifier()                      const override;

    static QString FilterIdentifier();
    static QString DisplayableName();
    static QList<int> SupportedVersions();
    static int CurrentVersion();

    static void srgb2ycbcr(float** const fimg, uint size);

private:

    void filterImage()                                    override;

    void waveletDenoise(float* fimg[3], unsigned int width, unsigned int height,
                        float threshold, double softness);
    inline void hatTransform(float* const temp, float* const base, int st, int size, int sc);

    void ycbcr2srgb(float** const fimg, uint size);

    void calculteStdevMultithreaded(const Args& prm);
    void thresholdingMultithreaded(const Args& prm);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
