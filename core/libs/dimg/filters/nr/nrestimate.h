/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-10-18
 * Description : Wavelets YCrCb Noise Reduction settings estimation by image content analys.
 *
 * SPDX-FileCopyrightText: 2012-2013 by Sayantan Datta <sayantan dot knz at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"
#include "nrfilter.h"
#include "dimg.h"
#include "dimgthreadedanalyser.h"

namespace Digikam
{

class DIGIKAM_EXPORT NREstimate : public DImgThreadedAnalyser
{
    Q_OBJECT

public:

    /**
     * Standard constructor with image container to parse
     */
    explicit NREstimate(DImg* const img, QObject* const parent = nullptr);
    ~NREstimate()                 override;

    /**
     * Perform estimate noise.
     */
    void startAnalyse()           override;

    /**
     * Return all Wavelets noise reduction settings computed by image analys.
     */
    NRContainer settings()  const;

    /**
     * To set image path where log files will be created to host computation algorithm results, for hacking purpose.
     * If path is not set, no log files will be created.
     */
    void setLogFilesPath(const QString& path);

private:

    /**
     * Internal method dedicated to convert DImg pixels from integer values to float values.
     * These ones will by used internally by estimateNoise through OpenCV API.
     */
    void readImage()        const;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
