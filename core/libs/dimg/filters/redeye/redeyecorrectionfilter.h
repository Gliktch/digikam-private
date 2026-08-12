/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 16/08/2016
 * Description : A Red-Eye automatic detection and correction filter.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2016      by Omar Amin <Omar dot moh dot amin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"
#include "digikam_opencv.h"
#include "redeyecorrectioncontainer.h"

namespace Digikam
{

class DIGIKAM_EXPORT RedEyeCorrectionFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    explicit RedEyeCorrectionFilter(QObject* const parent = nullptr);
    explicit RedEyeCorrectionFilter(DImg* const orgImage,
                                    QObject* const parent = nullptr,
                                    const RedEyeCorrectionContainer& settings = RedEyeCorrectionContainer());

    /**
     * Constructor for slave mode: execute immediately in current thread with specified master filter
     */
    explicit RedEyeCorrectionFilter(const RedEyeCorrectionContainer& settings,
                                    DImgThreadedFilter* const parentFilter,
                                    const DImg& orgImage,
                                    const DImg& destImage,
                                    int progressBegin = 0,
                                    int progressEnd = 100);

    ~RedEyeCorrectionFilter()                     override;

    static QString FilterIdentifier()
    {
        return QLatin1String("digikam:RedEyeCorrectionFilter");
    }

    static QString DisplayableName();

    static QList<int> SupportedVersions()
    {
        return QList<int>() << 1;
    }

    static int CurrentVersion()
    {
        return 1;
    }

    QString filterIdentifier()              const override
    {
        return FilterIdentifier();
    }

    FilterAction filterAction()                   override;

private:

    void filterImage()                            override;
    void readParameters(const FilterAction&)      override;

    void correctRedEye(uchar* data,
                       int type,
                       const cv::Rect& eyerect,
                       const cv::Rect& imgRect);
    void QRectFtocvRect(const QList<QRect>& faces,
                        std::vector<cv::Rect>& result);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
