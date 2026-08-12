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
#include "metaengine.h"

using namespace Digikam;

namespace DigikamGenericPanoramaPlugin
{

class CreatePtoTask : public PanoTask
{
public:

    explicit CreatePtoTask(const QString& workDirPath,
                           PanoramaFileType fileType,
                           QUrl& ptoUrl,
                           const QList<QUrl>& inputFiles,
                           const PanoramaItemUrlsMap& preProcessedMap,
                           bool addGPlusMetadata,
                           const QString& huginVersion);
    ~CreatePtoTask() override = default;

protected:

    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;

private:

    QUrl&                            ptoUrl;
    const PanoramaItemUrlsMap* const preProcessedMap;
    const PanoramaFileType           fileType;
    const QList<QUrl>&               inputFiles;
    const bool                       addGPlusMetadata;
    const QString&                   huginVersion;
    MetaEngine                       m_meta;

private:

    /// @note disabled
    CreatePtoTask() = delete;

    Q_DISABLE_COPY(CreatePtoTask)
};

} // namespace DigikamGenericPanoramaPlugin
