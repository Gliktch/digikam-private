/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-19
 * Description : ICC Transform threaded image filter.
 *
 * SPDX-FileCopyrightText: 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dimgloaderobserver.h"
#include "dimgthreadedfilter.h"
#include "icctransform.h"

namespace Digikam
{

class DIGIKAM_EXPORT IccTransformFilter : public DImgThreadedFilter,
                                          public DImgLoaderObserver
{
    Q_OBJECT

public:

    explicit IccTransformFilter(QObject* const parent = nullptr);
    explicit IccTransformFilter(DImg* const orgImage,
                                QObject* const parent,
                                const IccTransform& transform);
    ~IccTransformFilter()                                                                   override;

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:IccTransformFilter");
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

    QString         filterIdentifier()                                                const override
    {
        return FilterIdentifier();
    }

    FilterAction    filterAction()                                                          override;

    void                    readParameters(const FilterAction& action)                      override;
    bool                    parametersSuccessfullyRead()                              const override;
    QString                 readParametersError(const FilterAction& actionThatFailed) const override;

protected:

    void progressInfo(float progress)                                                       override;
    void filterImage()                                                                      override;

private:

    IccTransform m_transform;
};

} // namespace Digikam
