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

// Qt includes

#include <QPointer>

// Local includes

#include "panotask.h"
#include "dmetadata.h"

using namespace Digikam;

namespace DigikamGenericPanoramaPlugin
{

class CopyFilesTask : public PanoTask
{
public:

    explicit CopyFilesTask(const QString& workDirPath,
                           const QUrl& panoUrl,
                           const QUrl& finalPanoUrl,
                           const QUrl& ptoUrl,
                           const PanoramaItemUrlsMap& urls,
                           bool sPTO,
                           bool GPlusMetadata);
    ~CopyFilesTask() override = default;

protected:

    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;

private:

    const QUrl&                      panoUrl;
    const QUrl                       finalPanoUrl;
    const QUrl&                      ptoUrl;
    const PanoramaItemUrlsMap* const urlList;
    const bool                       savePTO;
    const bool                       addGPlusMetadata;

private:

    /// @note disabled
    CopyFilesTask() = delete;

    Q_DISABLE_COPY(CopyFilesTask)
};

} // namespace DigikamGenericPanoramaPlugin
