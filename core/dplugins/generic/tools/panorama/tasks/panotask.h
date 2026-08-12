/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-03-15
 * Description : a tool to create panorama by fusion of several images.
 *
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// KDE includes

#include <ThreadWeaver/Job>

// Local includes

#include "panoactions.h"

namespace DigikamGenericPanoramaPlugin
{

class PanoTask : public ThreadWeaver::Job
{
public:

    explicit PanoTask(PanoAction action, const QString& workDirPath);
    ~PanoTask()             override = default;

public:

    bool success() const    override;
    void requestAbort()     override;

public:

    QString          errString;
    const PanoAction action;
    bool             isAbortedFlag;

protected:

    bool             successFlag;
    const QUrl       tmpDir;

private:

    /// @note disabled
    PanoTask() = delete;

    Q_DISABLE_COPY(PanoTask)
};

} // namespace DigikamGenericPanoramaPlugin
