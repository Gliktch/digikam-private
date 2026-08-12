/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : A Sharpen threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class DIGIKAM_EXPORT SharpenFilter : public DImgThreadedFilter
{
    Q_OBJECT

private:

    class Q_DECL_HIDDEN Args
    {
    public:

        Args() = default;

    public:

        uint    start           = 0;
        uint    stop            = 0;
        uint    y               = 0;
        long    kernelWidth     = 0;
        double* normal_kernel   = nullptr;
        long    halfKernelWidth = 0;
    };

public:

    explicit SharpenFilter(QObject* const parent = nullptr);
    explicit SharpenFilter(DImg* const orgImage, QObject* const parent = nullptr, double radius=0.0, double sigma=1.0);

    /**
     * Constructor for slave mode: execute immediately in current thread with specified master filter
     */
    SharpenFilter(DImgThreadedFilter* const parentFilter, const DImg& orgImage, const DImg& destImage,
                  int progressBegin = 0, int progressEnd = 100, double radius = 0.0, double sigma = 1.0);

    ~SharpenFilter()                                                          override;

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:SharpenFilter");
    }

    static QString          DisplayableName();

    static QList<int>       SupportedVersions()
    {
        return QList<int>() << 1;
    }

    static int              CurrentVersion()
    {
        return 1;
    }

    QString         filterIdentifier()                                  const override
    {
        return FilterIdentifier();
    }

    FilterAction    filterAction()                                            override;
    void                    readParameters(const FilterAction& action)        override;


private:

    void filterImage()                                                        override;

    void sharpenImage(double radius, double sigma);

    bool convolveImage(const unsigned int order, const double* const kernel);

    void convolveImageMultithreaded(const Args& prm);

    int  getOptimalKernelWidth(double radius, double sigma);

private:

    double m_radius      = 0.0;
    double m_sigma       = 1.0;

    const double SQ2PI   = 2.50662827463100024161235523934010416269302368164062;
    const double Epsilon = 1.0e-12;
};

} // namespace Digikam
