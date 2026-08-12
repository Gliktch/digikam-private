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
#include "ptotype.h"
#include "metaengine.h"

using namespace Digikam;

namespace DigikamGenericPanoramaPlugin
{

class CreatePreviewTask : public PanoTask
{
public:

    explicit CreatePreviewTask(const QString& workDirPath,
                               QSharedPointer<const PTOType> inputPTO,
                               QUrl& previewPtoUrl,
                               const PanoramaItemUrlsMap& preProcessedUrlsMap);
    ~CreatePreviewTask() override = default;

protected:

    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;

private:

    QUrl&                         previewPtoUrl;
    QSharedPointer<const PTOType> ptoData;
    const PanoramaItemUrlsMap     preProcessedUrlsMap;
    MetaEngine                    m_meta;

private:

    /// @note disabled
    CreatePreviewTask() = delete;

    Q_DISABLE_COPY(CreatePreviewTask)
};

} // namespace DigikamGenericPanoramaPlugin
