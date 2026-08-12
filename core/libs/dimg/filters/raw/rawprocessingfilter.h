/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-14
 * Description : Filter to manage and help with raw loading
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "drawdecoding.h"
#include "iccprofile.h"

namespace Digikam
{

class DImgLoaderObserver;
class FilterAction;

/**
 * This is a special filter.
 * It implements RAW post processing.
 * Additionally, it provides some facilities for use from the DImg Raw loader.
 *
 * The original image shall come from RawEngine without further modification.
 */
class DIGIKAM_EXPORT RawProcessingFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    /**
     * Default constructor. You need to call setSettings() and setOriginalImage()
     * before starting the filter.
     */
    explicit RawProcessingFilter(QObject* const parent = nullptr);

    /**
     * Traditional constructor
     */
    RawProcessingFilter(DImg* const orgImage,
                        QObject* const parent,
                        const DRawDecoding& settings,
                        const QString& name = QString());

    /**
     * For use with a master filter. Computation is started immediately.
     */
    RawProcessingFilter(const DRawDecoding& settings,
                        DImgThreadedFilter* const master,
                        const DImg& orgImage,
                        const DImg& destImage,
                        int progressBegin = 0,
                        int progressEnd = 100,
                        const QString& name = QString());

    ~RawProcessingFilter()                                            override;

    /**
     * Set the raw decoding settings. The post processing is carried out here,
     * the libraw settings are needed to construct the FilterAction.
     */
    void setSettings(const DRawDecoding& settings);
    DRawDecoding settings()                                     const;

    /**
     * As additional and first post-processing step, convert the image's
     * color space to the specified profile.
     */
    void setOutputProfile(const IccProfile& profile);

    /**
     * Normally, filters post progress and are cancelled by DynamicThread facilities.
     * Here, as an alternative, a DImgLoaderObserver is set. It's continueQuery is called
     * and progress is posted in the given interval.
     */
    void setObserver(DImgLoaderObserver* const observer, int progressBegin, int progressEnd);

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:RawConverter");
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

    void                    readParameters(const FilterAction& action) override;

    QString         filterIdentifier()                   const override
    {
        return FilterIdentifier();
    }

    FilterAction    filterAction()                             override;

protected:

    void postProgress(int)                                     override;
    bool continueQuery()                                 const;

    void filterImage()                                         override;

protected:

    DRawDecoding        m_settings;
    IccProfile          m_customOutputProfile;
    DImgLoaderObserver* m_observer  = nullptr;
};

} // namespace Digikam
