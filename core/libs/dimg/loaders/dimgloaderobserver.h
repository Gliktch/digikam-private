/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-03
 * Description : DImgLoader observer interface
 *
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QtGlobal>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DImg;

class DIGIKAM_EXPORT DImgLoaderObserver
{

public:

    DImgLoaderObserver()          = default;
    virtual ~DImgLoaderObserver() = default;

    /**
     * @brief Posts progress information about image IO
     */
    virtual void progressInfo(float progress)
    {
        Q_UNUSED(progress);
    };

    /**
     * @brief Queries whether the image IO operation shall be continued
     */
    virtual bool continueQuery()
    {
        return true;
    };

    /**
     * @return A relative value which determines the granularity, the frequency
     * with which the DImgLoaderObserver is checked and progress is posted.
     * Standard is 1.0. Values < 1 mean less granularity (fewer checks),
     * values > 1 mean higher granularity (more checks).
     */
    virtual float granularity()
    {
        return 1.0F;
    };

private:

    Q_DISABLE_COPY(DImgLoaderObserver)
};

} // namespace Digikam
