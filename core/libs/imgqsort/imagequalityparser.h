/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 25/08/2013
 * Description : Image Quality Parser
 *
 * SPDX-FileCopyrightText: 2013-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dimg.h"
#include "digikam_export.h"
#include "digikam_globals.h"
#include "imagequalitysettings.h"
#include "digikam_opencv.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT ImageQualityParser : public QObject
{
    Q_OBJECT

public:

    /**
     * Standard constructor with picklabel container to fill at end of analyze.
     */
    explicit ImageQualityParser(const DImg& image,
                                const ImageQualitySettings& settings,
                                PickLabel* const label);
    ~ImageQualityParser();

    /**
     * Perform quality estimation and fill Pick Label value accordingly.
     */
    void startAnalyse();
    void cancelAnalyse();

private:

    /// @note disabled
    ImageQualityParser(const ImageQualityParser&)            = delete;
    ImageQualityParser& operator=(const ImageQualityParser&) = delete;
    explicit ImageQualityParser(QObject*);

public:

    static void unloadDLModel();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
