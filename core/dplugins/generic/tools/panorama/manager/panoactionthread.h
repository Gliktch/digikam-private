/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 *
 * SPDX-FileCopyrightText: 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QThread>
#include <QUrl>

// KDE includes

#include <ThreadWeaver/Sequence>

// Local includes

#include "panoactions.h"
#include "ptotype.h"

using namespace Digikam;

namespace DigikamGenericPanoramaPlugin
{

struct PanoActionData;

class PanoActionThread : public QObject
{
    Q_OBJECT

public:

    explicit PanoActionThread(QObject* const parent);
    ~PanoActionThread() override;

public:

    void preProcessFiles(const QList<QUrl>& urlList,
                         PanoramaItemUrlsMap& preProcessedMap,
                         QUrl& baseUrl,
                         QUrl& cpFindPtoUrl,
                         QUrl& cpCleanPtoUrl,
                         bool celeste,
                         PanoramaFileType fileType,
                         bool gPano,
                         const QString& huginVersion,
                         const QString& cpCleanPath,
                         const QString& cpFindPath);

    void optimizeProject(const QUrl& ptoUrl,
                         QUrl& optimizePtoUrl,
                         QUrl& viewCropPtoUrl,
                         bool levelHorizon,
                         bool buildGPano,
                         const QString& autooptimiserPath,
                         const QString& panoModifyPath);

    void generatePanoramaPreview(QSharedPointer<const PTOType> ptoData,
                                 QUrl& previewPtoUrl,
                                 QUrl& previewUrl,
                                 const PanoramaItemUrlsMap& preProcessedUrlsMap,
                                 const QString& huginExecutorPath);

    void compileProject(QSharedPointer<const PTOType> basePtoData,
                        QUrl& panoPtoUrl,
                        QUrl& panoUrl,
                        PanoramaFileType fileType,
                        const QRect& crop,
                        const QString& huginExecutorPath);

    void copyFiles(const QUrl& ptoUrl,
                   const QUrl& panoUrl,
                   const QUrl& finalPanoUrl,
                   const PanoramaItemUrlsMap& preProcessedUrlsMap,
                   bool savePTO,
                   bool addGPlusMetadata);

    void cancel();
    void finish();

Q_SIGNALS:

    void starting(const DigikamGenericPanoramaPlugin::PanoActionData& ad);
    void stepFinished(const DigikamGenericPanoramaPlugin::PanoActionData& ad);
    void jobCollectionFinished(const DigikamGenericPanoramaPlugin::PanoActionData& ad);

    void cpFindPtoReady(const QUrl& cpFindPtoUrl);
    void cpCleanPtoReady(const QUrl& cpCleanPtoUrl);
    void optimizePtoReady(const QUrl& optimizePtoUrl);
    void previewFileReady(const QUrl& previewFileUrl);
    void panoFileReady(const QUrl& panoFileUrl);

private Q_SLOTS:

    void slotDone(ThreadWeaver::JobPointer j);
    void slotStepDone(ThreadWeaver::JobPointer j);
    void slotStarting(ThreadWeaver::JobPointer j);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPanoramaPlugin
