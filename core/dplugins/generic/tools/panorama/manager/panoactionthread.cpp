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

#include "panoactionthread.h"

// Qt includes

#include <QSharedPointer>
#include <QTemporaryDir>
#include <QString>

// KDE includes

#include <ThreadWeaver/Queue>
#include <ThreadWeaver/QObjectDecorator>
#include <threadweaver/debuggingaids.h>

// Local includes

#include "digikam_debug.h"
#include "panotasks.h"

using namespace ThreadWeaver;

namespace DigikamGenericPanoramaPlugin
{

class Q_DECL_HIDDEN PanoActionThread::Private
{
public:

    explicit Private(QObject* const parent = nullptr)
        : threadQueue(new Queue(parent))
    {
        ThreadWeaver::setDebugLevel(true, 10);
    }

    ~Private()
    {
        threadQueue->dequeue();
        threadQueue->requestAbort();
        threadQueue->finish();
    }

public:

    QSharedPointer<QTemporaryDir> preprocessingTmpDir;
    QString                       preprocessingTmpPath;
    QSharedPointer<Queue>         threadQueue;
};

PanoActionThread::PanoActionThread(QObject* const parent)
    : QObject(parent),
      d      (new Private(this))
{
    // PanoActionThread init

    qRegisterMetaType<PanoActionData>("PanoActionData");

    d->threadQueue->setMaximumNumberOfThreads(qMax(QThread::idealThreadCount(), 1));
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Starting Main Thread";
}

PanoActionThread::~PanoActionThread()
{
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Calling action thread destructor";

    delete d;
}

void PanoActionThread::cancel()
{
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Cancel (PanoAction Thread)";
    d->threadQueue->dequeue();
    d->threadQueue->requestAbort();
}

void PanoActionThread::finish()
{
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Finish (PanoAction Thread)";

    // Wait for all queued jobs to finish

    d->threadQueue->finish();
    d->threadQueue->resume();
}

void PanoActionThread::preProcessFiles(const QList<QUrl>& urlList,
                                       PanoramaItemUrlsMap& preProcessedMap,
                                       QUrl& baseUrl,
                                       QUrl& cpFindPtoUrl,
                                       QUrl& cpCleanPtoUrl,
                                       bool celeste,
                                       PanoramaFileType fileType,
                                       bool gPano,
                                       const QString& huginVersion,
                                       const QString& cpCleanPath,
                                       const QString& cpFindPath)
{
    QString prefix          = QDir::tempPath() + QLatin1Char('/') +
                              QLatin1String("digiKam-panorama-tmp-XXXXXX");

    d->preprocessingTmpDir  = QSharedPointer<QTemporaryDir>(new QTemporaryDir(prefix));
    QDir().mkpath(d->preprocessingTmpDir->path() + QLatin1String("/work"));
    d->preprocessingTmpPath = d->preprocessingTmpDir->path() + QLatin1String("/work");

    QSharedPointer<Sequence> jobSeq(new Sequence());

    QSharedPointer<Collection> preprocessJobs(new Collection());

    int id                  = 0;

    for (const QUrl& file : urlList)
    {
        preProcessedMap.insert(file, PanoramaPreprocessedUrls());

        QObjectDecorator* const t = new QObjectDecorator(new PreProcessTask(d->preprocessingTmpPath,
                                                         id++,
                                                         preProcessedMap[file],
                                                         file));

        connect(t, SIGNAL(started(ThreadWeaver::JobPointer)),
                this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

        connect(t, SIGNAL(done(ThreadWeaver::JobPointer)),
                this, SLOT(slotStepDone(ThreadWeaver::JobPointer)));

        (*preprocessJobs) << JobPointer(t);
    }

    (*jobSeq) << preprocessJobs;

    QObjectDecorator* const pto = new QObjectDecorator(new CreatePtoTask(d->preprocessingTmpPath,
                                                       fileType,
                                                       baseUrl,
                                                       urlList,
                                                       preProcessedMap,
                                                       gPano,
                                                       huginVersion));

    connect(pto, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(pto, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotStepDone(ThreadWeaver::JobPointer)));

    (*jobSeq) << JobPointer(pto);

    QObjectDecorator* const cpFind = new QObjectDecorator(new CpFindTask(d->preprocessingTmpPath,
                                                          baseUrl,
                                                          cpFindPtoUrl,
                                                          celeste,
                                                          cpFindPath));

    connect(cpFind, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(cpFind, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotStepDone(ThreadWeaver::JobPointer)));

    (*jobSeq) << JobPointer(cpFind);

    QObjectDecorator* const cpClean = new QObjectDecorator(new CpCleanTask(d->preprocessingTmpPath,
                                                           cpFindPtoUrl,
                                                           cpCleanPtoUrl,
                                                           cpCleanPath));

    connect(cpClean, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(cpClean, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotDone(ThreadWeaver::JobPointer)));

    (*jobSeq) << JobPointer(cpClean);

    d->threadQueue->enqueue(jobSeq);
}

void PanoActionThread::optimizeProject(const QUrl& ptoUrl,
                                       QUrl& optimizePtoUrl,
                                       QUrl& viewCropPtoUrl,
                                       bool levelHorizon,
                                       bool buildGPano,
                                       const QString& autooptimiserPath,
                                       const QString& panoModifyPath)
{
    QSharedPointer<Sequence> jobs(new Sequence());

    QObjectDecorator* const opt = new QObjectDecorator(new OptimisationTask(d->preprocessingTmpPath,
                                                       ptoUrl,
                                                       optimizePtoUrl,
                                                       levelHorizon,
                                                       buildGPano,
                                                       autooptimiserPath));

    connect(opt, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(opt, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotStepDone(ThreadWeaver::JobPointer)));

    (*jobs) << opt;

    QObjectDecorator* const act = new QObjectDecorator(new AutoCropTask(d->preprocessingTmpPath,
                                                       optimizePtoUrl,
                                                       viewCropPtoUrl,
                                                       buildGPano,
                                                       panoModifyPath));

    connect(act, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(act, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotDone(ThreadWeaver::JobPointer)));

    (*jobs) << act;

    d->threadQueue->enqueue(jobs);
}

void PanoActionThread::generatePanoramaPreview(QSharedPointer<const PTOType> ptoData,
                                               QUrl& previewPtoUrl,
                                               QUrl& previewUrl,
                                               const PanoramaItemUrlsMap& preProcessedUrlsMap,
                                               const QString& huginExecutorPath)
{
    QSharedPointer<Sequence> jobs(new Sequence());

    QObjectDecorator* const ptoTask = new QObjectDecorator(new CreatePreviewTask(d->preprocessingTmpPath,
                                                           ptoData,
                                                           previewPtoUrl,
                                                           preProcessedUrlsMap));

    connect(ptoTask, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(ptoTask, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotStepDone(ThreadWeaver::JobPointer)));

    (*jobs) << ptoTask;

    QObjectDecorator* const huginExecutorTask = new QObjectDecorator(new HuginExecutorTask(d->preprocessingTmpPath,
                                                                     previewPtoUrl,
                                                                     previewUrl,
                                                                     JPEG,
                                                                     huginExecutorPath,
                                                                     true));

    connect(huginExecutorTask, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(huginExecutorTask, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotStepDone(ThreadWeaver::JobPointer)));

    (*jobs) << huginExecutorTask;

    d->threadQueue->enqueue(jobs);
}

void PanoActionThread::compileProject(QSharedPointer<const PTOType> basePtoData,
                                      QUrl& panoPtoUrl,
                                      QUrl& panoUrl,
                                      PanoramaFileType fileType,
                                      const QRect& crop,
                                      const QString& huginExecutorPath)
{
    QSharedPointer<Sequence> jobs(new Sequence());

    QObjectDecorator* const ptoTask = new QObjectDecorator(new CreateFinalPtoTask(d->preprocessingTmpPath,
                                                           basePtoData,
                                                           panoPtoUrl,
                                                           crop));

    connect(ptoTask, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(ptoTask, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotStepDone(ThreadWeaver::JobPointer)));

    (*jobs) << ptoTask;

    QObjectDecorator* const huginExecutorTask = new QObjectDecorator(new HuginExecutorTask(d->preprocessingTmpPath,
                                                                     panoPtoUrl,
                                                                     panoUrl,
                                                                     fileType,
                                                                     huginExecutorPath,
                                                                     false));

    connect(huginExecutorTask, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(huginExecutorTask, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotStepDone(ThreadWeaver::JobPointer)));

    (*jobs) << huginExecutorTask;

    d->threadQueue->enqueue(jobs);
}

void PanoActionThread::copyFiles(const QUrl& ptoUrl,
                                 const QUrl& panoUrl,
                                 const QUrl& finalPanoUrl,
                                 const PanoramaItemUrlsMap& preProcessedUrlsMap,
                                 bool savePTO,
                                 bool addGPlusMetadata)
{
    QObjectDecorator* const t = new QObjectDecorator(new CopyFilesTask(d->preprocessingTmpPath,
                                                     panoUrl,
                                                     finalPanoUrl,
                                                     ptoUrl,
                                                     preProcessedUrlsMap,
                                                     savePTO,
                                                     addGPlusMetadata));

    connect(t, SIGNAL(started(ThreadWeaver::JobPointer)),
            this, SLOT(slotStarting(ThreadWeaver::JobPointer)));

    connect(t, SIGNAL(done(ThreadWeaver::JobPointer)),
            this, SLOT(slotDone(ThreadWeaver::JobPointer)));

    d->threadQueue->enqueue(JobPointer(t));
}

void PanoActionThread::slotStarting(JobPointer j)
{
    QSharedPointer<QObjectDecorator> dec = j.staticCast<QObjectDecorator>();
    PanoTask* const t                    = static_cast<PanoTask*>(dec->job());

    PanoActionData ad;
    ad.starting     = true;
    ad.action       = t->action;
    ad.id           = -1;

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Starting (PanoAction Thread) (action):" << ad.action;

    if (t->action == PANO_PREPROCESS_INPUT)
    {
        const PreProcessTask* const p = static_cast<PreProcessTask*>(t);
        ad.id                         = p->id;
    }

    Q_EMIT starting(ad);
}

void PanoActionThread::slotStepDone(JobPointer j)
{
    QSharedPointer<QObjectDecorator> dec = j.staticCast<QObjectDecorator>();
    PanoTask* const t                    = static_cast<PanoTask*>(dec->job());

    PanoActionData ad;
    ad.starting     = false;
    ad.action       = t->action;
    ad.id           = -1;
    ad.success      = t->success();
    ad.message      = t->errString;

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Step done (PanoAction Thread) (action, success):"
                                         << ad.action << ad.success;

    if (t->action == PANO_PREPROCESS_INPUT)
    {
        const PreProcessTask* const p = static_cast<PreProcessTask*>(t);
        ad.id                         = p->id;
    }

    if (!ad.success)
    {
        d->threadQueue->dequeue();
    }

    Q_EMIT stepFinished(ad);
}

void PanoActionThread::slotDone(JobPointer j)
{
    QSharedPointer<QObjectDecorator> dec = j.staticCast<QObjectDecorator>();
    PanoTask* const t                    = static_cast<PanoTask*>(dec->job());

    PanoActionData ad;
    ad.starting     = false;
    ad.action       = t->action;
    ad.id           = -1;
    ad.success      = t->success();
    ad.message      = t->errString;

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Done (PanoAction Thread) (action, success):"
                                         << ad.action << ad.success;

    if (t->action == PANO_PREPROCESS_INPUT)
    {
        const PreProcessTask* const p = static_cast<PreProcessTask*>(t);
        ad.id                         = p->id;
    }

    Q_EMIT jobCollectionFinished(ad);
}

} // namespace DigikamGenericPanoramaPlugin

#include "moc_panoactionthread.cpp"
