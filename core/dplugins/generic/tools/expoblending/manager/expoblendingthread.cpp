/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "expoblendingthread.h"

// C++ includes

#include <cmath>

// Under Win32, log2f is not defined...

#ifdef Q_OS_WIN32       // krazy:exclude=cpp
#   define log2f(x) (logf(x)*1.4426950408889634f)
#endif

#ifdef Q_OS_FREEBSD
#include <osreldate.h>
#    if __FreeBSD_version < 802502
#        define log2f(x) (logf(x)*1.4426950408889634f)
#    endif
#endif

// Qt includes

#include <QPair>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QDateTime>
#include <QFileInfo>
#include <QPointer>
#include <QFuture>
#include <QtConcurrentRun>
#include <QTemporaryDir>
#include <QProcess>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "drawdecoder.h"
#include "dimg.h"
#include "dimgloaderobserver.h"
#include "drawdecoderwidget.h"
#include "drawdecoding.h"
#include "actionthreadbase.h"

namespace DigikamGenericExpoBlendingPlugin
{

class RawObserver;

class Q_DECL_HIDDEN ExpoBlendingThread::Private
{
public:

    class Q_DECL_HIDDEN Task
    {
    public:

        bool                        align           = false;
        QList<QUrl>                 urls;
        QUrl                        outputUrl;
        QString                     binaryPath;
        ExpoBlendingAction          action          = EXPOBLENDING_NONE;
        EnfuseSettings              enfuseSettings;
    };

public:

    Private() = default;

public:

    volatile bool                   cancel          = false;
    bool                            align           = false;
    bool                            enfuseVersion4x = true;

    QMutex                          mutex;
    QMutex                          lock;

    QWaitCondition                  condVar;

    QList<Task*>                    todo;

    QSharedPointer<QTemporaryDir>   preprocessingTmpDir;
    QSharedPointer<QProcess>        enfuseProcess;
    QSharedPointer<QProcess>        alignProcess;

    RawObserver*                    rawObserver     = nullptr;

    /**
     * List of results files produced by enfuse that may need cleaning.
     * Only access this through the provided mutex.
     */
    QList<QUrl>                     enfuseTmpUrls;
    QMutex                          enfuseTmpUrlsMutex;

    /// Preprocessing
    QList<QUrl>                     mixedUrls;     ///< Original non-RAW + Raw converted urls to align.
    ExpoBlendingItemUrlsMap         preProcessedUrlsMap;

    MetaEngine                      meta;
};

// -------------------------------------------------------------------------

class Q_DECL_HIDDEN RawObserver : public DImgLoaderObserver
{
public:

    explicit RawObserver(ExpoBlendingThread::Private* const priv)
        : DImgLoaderObserver(),
          d                 (priv)
    {
    }

    ~RawObserver() override = default;

    bool continueQuery() override
    {
        return (!d->cancel);
    }

private:

    ExpoBlendingThread::Private* const d = nullptr;
};

// -------------------------------------------------------------------------

ExpoBlendingThread::ExpoBlendingThread(QObject* const parent)
    : QThread(parent),
      d      (new Private)
{
    d->rawObserver = new RawObserver(d);
    qRegisterMetaType<ExpoBlendingActionData>("ExpoBlendingActionData");
}

ExpoBlendingThread::~ExpoBlendingThread()
{
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "ExpoBlendingThread shutting down."
                                         << "Canceling all actions and waiting for them";

    // cancel the thread

    cancel();

    // wait for the thread to finish

    wait();

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Thread finished";

    cleanUpResultFiles();

    delete d;
}

void ExpoBlendingThread::setEnfuseVersion(const double version)
{
    d->enfuseVersion4x = (version >= 4.0);
}

void ExpoBlendingThread::cleanUpResultFiles()
{
    // Cleanup all tmp files created by Enfuse process.

    QMutexLocker locker(&d->enfuseTmpUrlsMutex);

    for (const QUrl& url : std::as_const(d->enfuseTmpUrls))
    {
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Removing temp file" << url.toLocalFile();
        QFile(url.toLocalFile()).remove();
    }

    d->enfuseTmpUrls.clear();
}

void ExpoBlendingThread::setPreProcessingSettings(bool align)
{
    d->align = align;
}

void ExpoBlendingThread::identifyFiles(const QList<QUrl>& urlList)
{
    for (const QUrl& url : std::as_const(urlList))
    {
        Private::Task* const t = new Private::Task;
        t->action              = EXPOBLENDING_IDENTIFY;
        t->urls.append(url);

        QMutexLocker lock(&d->mutex);
        d->todo << t;
        d->condVar.wakeAll();
    }
}

void ExpoBlendingThread::loadProcessed(const QUrl& url)
{
    Private::Task* const t = new Private::Task;
    t->action              = EXPOBLENDING_LOAD;
    t->urls.append(url);

    QMutexLocker lock(&d->mutex);
    d->todo << t;
    d->condVar.wakeAll();
}

void ExpoBlendingThread::preProcessFiles(const QList<QUrl>& urlList, const QString& alignPath)
{
    Private::Task* const t = new Private::Task;
    t->action              = EXPOBLENDING_PREPROCESSING;
    t->urls                = urlList;
    t->align               = d->align;
    t->binaryPath          = alignPath;

    QMutexLocker lock(&d->mutex);
    d->todo << t;
    d->condVar.wakeAll();
}

void ExpoBlendingThread::enfusePreview(const QList<QUrl>& alignedUrls, const QUrl& outputUrl,
                                       const EnfuseSettings& settings, const QString& enfusePath)
{
    Private::Task* const t = new Private::Task;
    t->action              = EXPOBLENDING_ENFUSEPREVIEW;
    t->urls                = alignedUrls;
    t->outputUrl           = outputUrl;
    t->enfuseSettings      = settings;
    t->binaryPath          = enfusePath;

    QMutexLocker lock(&d->mutex);
    d->todo << t;
    d->condVar.wakeAll();
}

void ExpoBlendingThread::enfuseFinal(const QList<QUrl>& alignedUrls, const QUrl& outputUrl,
                                     const EnfuseSettings& settings, const QString& enfusePath)
{
    Private::Task* const t = new Private::Task;
    t->action              = EXPOBLENDING_ENFUSEFINAL;
    t->urls                = alignedUrls;
    t->outputUrl           = outputUrl;
    t->enfuseSettings      = settings;
    t->binaryPath          = enfusePath;

    QMutexLocker lock(&d->mutex);
    d->todo << t;
    d->condVar.wakeAll();
}

void ExpoBlendingThread::cancel()
{
    QMutexLocker lock(&d->mutex);
    d->todo.clear();
    d->cancel = true;

    if (d->enfuseProcess)
    {
        d->enfuseProcess->kill();
    }

    if (d->alignProcess)
    {
        d->alignProcess->kill();
    }

    d->condVar.wakeAll();
}

void ExpoBlendingThread::run()
{
    ActionThreadBase::setCurrentThreadName(QLatin1String("ExpoBlendingThread"));       // To customize thread name

    d->cancel = false;

    while (!d->cancel)
    {
        Private::Task* t = nullptr;
        {
            QMutexLocker lock(&d->mutex);

            if (!d->todo.isEmpty())
            {

                t = d->todo.takeFirst();
            }
            else
            {
                d->condVar.wait(&d->mutex);
            }
        }

        if (t)
        {
            switch (t->action)
            {
                case EXPOBLENDING_IDENTIFY:
                {
                    // Identify Exposure.

                    QString avLum;

                    if (!t->urls.isEmpty())
                    {
                        float val = getAverageSceneLuminance(t->urls[0]);

                        if (val != -1)
                        {
                            avLum.setNum(log2f(val), 'g', 2);
                        }
                    }

                    ExpoBlendingActionData ad;
                    ad.action  = t->action;
                    ad.inUrls  = t->urls;
                    ad.message = avLum.isEmpty() ? i18nc("average scene luminance value unknown", "unknown") : avLum;
                    ad.success = avLum.isEmpty();

                    Q_EMIT signalFinished(ad);

                    break;
                }

                case EXPOBLENDING_PREPROCESSING:
                {
                    ExpoBlendingActionData ad1;
                    ad1.action   = EXPOBLENDING_PREPROCESSING;
                    ad1.inUrls   = t->urls;
                    ad1.starting = true;

                    Q_EMIT signalStarting(ad1);

                    QString errors;

                    bool result  = startPreProcessing(t->urls, t->align, t->binaryPath, errors);

                    ExpoBlendingActionData ad2;
                    ad2.action              = EXPOBLENDING_PREPROCESSING;
                    ad2.inUrls              = t->urls;
                    ad2.preProcessedUrlsMap = d->preProcessedUrlsMap;
                    ad2.success             = result;
                    ad2.message             = errors;

                    Q_EMIT signalFinished(ad2);

                    break;
                }

                case EXPOBLENDING_LOAD:
                {
                    ExpoBlendingActionData ad1;
                    ad1.action   = EXPOBLENDING_LOAD;
                    ad1.inUrls   = t->urls;
                    ad1.starting = true;

                    Q_EMIT signalStarting(ad1);

                    QImage image;
                    bool result  = image.load(t->urls[0].toLocalFile());

                    // rotate image

                    if (result)
                    {
                        if (d->meta.load(t->urls[0].toLocalFile()))
                        {
                            d->meta.rotateExifQImage(image, d->meta.getItemOrientation());
                        }
                    }

                    ExpoBlendingActionData ad2;
                    ad2.action         = EXPOBLENDING_LOAD;
                    ad2.inUrls         = t->urls;
                    ad2.success        = result;
                    ad2.image          = image;

                    Q_EMIT signalFinished(ad2);

                    break;
                }

                case EXPOBLENDING_ENFUSEPREVIEW:
                {
                    ExpoBlendingActionData ad1;
                    ad1.action         = EXPOBLENDING_ENFUSEPREVIEW;
                    ad1.inUrls         = t->urls;
                    ad1.starting       = true;
                    ad1.enfuseSettings = t->enfuseSettings;

                    Q_EMIT signalStarting(ad1);

                    QString errors;
                    QUrl    destUrl         = t->outputUrl;
                    EnfuseSettings settings = t->enfuseSettings;
                    settings.outputFormat   = DSaveSettingsWidget::OUTPUT_JPEG;    // JPEG for preview: fast and small.
                    bool result             = startEnfuse(t->urls, destUrl, settings, t->binaryPath, errors);

                    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Preview result was:" << result;

                    // preserve exif information for auto rotation

                    if (result)
                    {
                        if (d->meta.load(t->urls[0].toLocalFile()))
                        {
                            MetaEngine::ImageOrientation orientation = d->meta.getItemOrientation();

                            if (d->meta.load(destUrl.toLocalFile()))
                            {
                                d->meta.setItemOrientation(orientation);
                                d->meta.applyChanges(true);
                            }
                        }
                    }

                    // To be cleaned in destructor.

                    QMutexLocker locker(&d->enfuseTmpUrlsMutex);
                    d->enfuseTmpUrls << destUrl;

                    ExpoBlendingActionData ad2;
                    ad2.action         = EXPOBLENDING_ENFUSEPREVIEW;
                    ad2.inUrls         = t->urls;
                    ad2.outUrls        = QList<QUrl>() << destUrl;
                    ad2.success        = result;
                    ad2.message        = errors;
                    ad2.enfuseSettings = t->enfuseSettings;

                    Q_EMIT signalFinished(ad2);

                    break;
                }

                case EXPOBLENDING_ENFUSEFINAL:
                {
                    ExpoBlendingActionData ad1;
                    ad1.action         = EXPOBLENDING_ENFUSEFINAL;
                    ad1.inUrls         = t->urls;
                    ad1.starting       = true;
                    ad1.enfuseSettings = t->enfuseSettings;

                    Q_EMIT signalStarting(ad1);

                    QString errors;
                    QUrl destUrl = t->outputUrl;
                    bool result  = startEnfuse(t->urls, destUrl, t->enfuseSettings, t->binaryPath, errors);

                    // We will take first image metadata from stack to restore Exif, Iptc, and Xmp.

                    if (d->meta.load(t->urls[0].toLocalFile()))
                    {
                        result = result & d->meta.setXmpTagString("Xmp.digiKam.EnfuseInputFiles",
                                                                  t->enfuseSettings.inputImagesList());

                        result = result & d->meta.setXmpTagString("Xmp.digiKam.EnfuseSettings",
                                                                  t->enfuseSettings.asCommentString().
                                                                  replace(QLatin1Char('\n'),
                                                                  QLatin1String(" ; ")));

                        d->meta.setImageDateTime(QDateTime::currentDateTime());

                        if (t->enfuseSettings.outputFormat != DSaveSettingsWidget::OUTPUT_JPEG)
                        {
                            QImage img;

                            if (img.load(destUrl.toLocalFile()))
                            {
                                d->meta.setItemPreview(img.scaled(1280, 1024, Qt::KeepAspectRatio));
                            }
                        }

                        d->meta.save(destUrl.toLocalFile(), true);
                    }

                    // To be cleaned in destructor.

                    QMutexLocker locker(&d->enfuseTmpUrlsMutex);
                    d->enfuseTmpUrls << destUrl;

                    ExpoBlendingActionData ad2;
                    ad2.action         = EXPOBLENDING_ENFUSEFINAL;
                    ad2.inUrls         = t->urls;
                    ad2.outUrls        = QList<QUrl>() << destUrl;
                    ad2.success        = result;
                    ad2.message        = errors;
                    ad2.enfuseSettings = t->enfuseSettings;

                    Q_EMIT signalFinished(ad2);

                    break;
                }

                default:
                {
                    qCritical(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Unknown action specified" << Qt::endl;
                    break;
                }
            }
        }

        delete t;
    }
}

bool ExpoBlendingThread::preProcessingMultithreaded(const QUrl& url)
{
    ActionThreadBase::setCurrentThreadName(QLatin1String(__FUNCTION__));       // To customize thread name

    bool error  = false;

    // check if we have RAW or HEIF file -> use preview image then

    QString ext = QFileInfo(url.toLocalFile()).suffix().toUpper();

    if (
        DRawDecoder::isRawFile(url)    ||
        (ext == QLatin1String("HIF"))  ||
        (ext == QLatin1String("HEIC")) ||
        (ext == QLatin1String("HEIF"))
       )
    {
        QUrl preprocessedUrl, previewUrl;

        if (!convertRaw(url, preprocessedUrl))
        {
            error = true;
            return error;
        }

        if (!computePreview(preprocessedUrl, previewUrl))
        {
            error = true;
            return error;
        }

        d->lock.lock();
        d->mixedUrls.append(preprocessedUrl);

        // In case of alignment is not performed.

        d->preProcessedUrlsMap.insert(url, ExpoBlendingItemPreprocessedUrls(preprocessedUrl, previewUrl));
        d->lock.unlock();
    }
    else
    {
        // NOTE: in this case, preprocessed Url is original file Url.

        QUrl previewUrl;

        if (!computePreview(url, previewUrl))
        {
            error = true;
            return error;
        }

        d->lock.lock();
        d->mixedUrls.append(url);

        // In case of alignment is not performed.

        d->preProcessedUrlsMap.insert(url, ExpoBlendingItemPreprocessedUrls(url, previewUrl));
        d->lock.unlock();
    }

    return error;
}

bool ExpoBlendingThread::startPreProcessing(const QList<QUrl>& inUrls,
                                            bool align,
                                            const QString& alignPath, QString& errors)
{
    QString prefix = QDir::tempPath() + QLatin1Char('/') +
                     QLatin1String("digiKam-expoblending-tmp-XXXXXX");

    d->preprocessingTmpDir = QSharedPointer<QTemporaryDir>(new QTemporaryDir(prefix));

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Expoblending temp dir:"
                                         << d->preprocessingTmpDir->path();

    // Parallelized pre-process RAW files if necessary.

    d->mixedUrls.clear();
    d->preProcessedUrlsMap.clear();

    bool error = false;

    QList <QFuture<bool> > tasks;

    for (int i = 0 ; i < inUrls.size() ; ++i)
    {
        QUrl url = inUrls.at(i);

        tasks.append(QtConcurrent::run(

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

                                       &ExpoBlendingThread::preProcessingMultithreaded, this,

#else

                                       this, &ExpoBlendingThread::preProcessingMultithreaded,

#endif

                                       url
                                      )
        );
    }

    for (QFuture<bool>& t: tasks)
    {
        t.waitForFinished();

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        error |= t.takeResult();

#else

        error |= t.result();

#endif

    }

    if (error)
    {
        return false;
    }

    if (align)
    {
        // Re-align images

        d->alignProcess.reset(new QProcess());
        d->alignProcess->setProcessChannelMode(QProcess::MergedChannels);
        d->alignProcess->setWorkingDirectory(d->preprocessingTmpDir->path());

        QProcessEnvironment env = adjustedEnvironmentForAppImage();
        env.insert(QLatin1String("OMP_NUM_THREADS"),
                   QString::number(QThread::idealThreadCount()));
        d->alignProcess->setProcessEnvironment(env);

        QStringList args;
        args << QLatin1String("-v");
        args << QLatin1String("-c");
        args << QLatin1String("16");
        args << QLatin1String("-a");
        args << QLatin1String("aligned");

        for (const QUrl& url : std::as_const(d->mixedUrls))
        {
            args << url.toLocalFile();
        }

        d->alignProcess->setProgram(alignPath);
        d->alignProcess->setArguments(args);

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Align command line:" << d->alignProcess->program();

        d->alignProcess->start();

        if (!d->alignProcess->waitForFinished(-1))
        {
            errors = getProcessError(*(d->alignProcess));
            qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "align_image_stack error:" << errors;

            return false;
        }

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Align exit status: " << d->alignProcess->exitStatus();
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Align exit code:   " << d->alignProcess->exitCode();

        if (d->alignProcess->exitStatus() != QProcess::NormalExit)
        {
            return false;
        }

        if (d->alignProcess->exitCode() != 0)
        {
            errors = getProcessError(*(d->alignProcess));

            return false;
        }

        uint    i = 0;
        d->preProcessedUrlsMap.clear();

        for (const QUrl& url : std::as_const(inUrls))
        {
            QUrl previewUrl;
            QUrl alignedUrl = QUrl::fromLocalFile(d->preprocessingTmpDir->path()                         +
                                                  QLatin1Char('/')                                       +
                                                  QLatin1String("aligned")                               +
                                                  QString::number(i).rightJustified(4, QLatin1Char('0')) +
                                                  QLatin1String(".tif"));

            if (!computePreview(alignedUrl, previewUrl))
            {
                return false;
            }

            d->preProcessedUrlsMap.insert(url, ExpoBlendingItemPreprocessedUrls(alignedUrl, previewUrl));
            ++i;
        }

        const auto urls = d->preProcessedUrlsMap.keys();

        for (const QUrl& inputUrl : urls)
        {
            qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Pre-processed output urls map:"
                                                 << inputUrl << "=>"
                                                 << d->preProcessedUrlsMap[inputUrl].preprocessedUrl << ","
                                                 << d->preProcessedUrlsMap[inputUrl].previewUrl;
        }

        return true;
    }
    else
    {
        const auto urls = d->preProcessedUrlsMap.keys();

        for (const QUrl& inputUrl : urls)
        {
            qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Pre-processed output urls map:"
                                                 << inputUrl << "=>"
                                                 << d->preProcessedUrlsMap[inputUrl].preprocessedUrl << ","
                                                 << d->preProcessedUrlsMap[inputUrl].previewUrl;
        }

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Alignment not performed.";

        return true;
    }
}

bool ExpoBlendingThread::computePreview(const QUrl& inUrl, QUrl& outUrl)
{
    outUrl = QUrl::fromLocalFile(d->preprocessingTmpDir->path()                               +
                                 QLatin1Char('/')                                             +
                                 QLatin1Char('.')                                             +
                                 inUrl.fileName().replace(QLatin1Char('.'), QLatin1Char('_')) +
                                 QLatin1String("-preview.jpg"));

    DImg img;

    if (img.load(inUrl.toLocalFile()))
    {
        DImg preview = img.smoothScale(1280, 1024, Qt::KeepAspectRatio);
        bool saved   = preview.save(outUrl.toLocalFile(), QLatin1String("JPG"));

        // save exif information also to preview image for auto rotation

        if (saved)
        {
            if (d->meta.load(inUrl.toLocalFile()))
            {
                MetaEngine::ImageOrientation orientation = d->meta.getItemOrientation();

                if (d->meta.load(outUrl.toLocalFile()))
                {
                    d->meta.setItemOrientation(orientation);
                    d->meta.applyChanges(true);
                }
            }
        }

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Preview Image url:" << outUrl << ", saved:" << saved;

        return saved;
    }

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Input image not loaded:" << inUrl;

    return false;
}

bool ExpoBlendingThread::convertRaw(const QUrl& inUrl, QUrl& outUrl)
{
    DImg img;

    DRawDecoding settings;
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("ImageViewer Settings"));
    DRawDecoderWidget::readSettings(settings.rawPrm, group);

    if (img.load(inUrl.toLocalFile(), d->rawObserver, settings))
    {
        QFileInfo fi(inUrl.toLocalFile());
        outUrl = QUrl::fromLocalFile(d->preprocessingTmpDir->path()                                    +
                                     QLatin1Char('/')                                                  +
                                     QLatin1Char('.')                                                  +
                                     fi.completeBaseName().replace(QLatin1Char('.'), QLatin1Char('_')) +
                                     QLatin1String(".tif"));

        if (!img.save(outUrl.toLocalFile(), QLatin1String("TIF")))
        {
            return false;
        }

        if (d->meta.load(outUrl.toLocalFile()))
        {
            d->meta.setItemDimensions(img.size());
            d->meta.setExifTagString("Exif.Image.DocumentName", inUrl.fileName());
            d->meta.setXmpTagString("Xmp.tiff.Make",  d->meta.getExifTagString("Exif.Image.Make"));
            d->meta.setXmpTagString("Xmp.tiff.Model", d->meta.getExifTagString("Exif.Image.Model"));
            d->meta.setItemOrientation(MetaEngine::ORIENTATION_NORMAL);
            d->meta.applyChanges(true);
        }
    }
    else
    {
        return false;
    }

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Convert RAW output url:" << outUrl;

    return true;
}

bool ExpoBlendingThread::startEnfuse(const QList<QUrl>& inUrls, QUrl& outUrl,
                                     const EnfuseSettings& settings,
                                     const QString& enfusePath, QString& errors)
{
    QString comp;
    QString ext = DSaveSettingsWidget::extensionForFormat(settings.outputFormat);

    if (ext == QLatin1String(".tif"))
    {
        comp = QLatin1String("--compression=DEFLATE");
    }

    outUrl.setPath(outUrl.adjusted(QUrl::RemoveFilename).path()                +
                                   QLatin1String(".digiKam-expoblending-tmp-") +
                                   QString::number(QDateTime::currentSecsSinceEpoch()) + ext);

    d->enfuseProcess.reset(new QProcess());
    d->enfuseProcess->setProcessChannelMode(QProcess::MergedChannels);
    d->enfuseProcess->setWorkingDirectory(d->preprocessingTmpDir->path());

    QProcessEnvironment env = adjustedEnvironmentForAppImage();
    env.insert(QLatin1String("OMP_NUM_THREADS"),
               QString::number(QThread::idealThreadCount()));
    d->enfuseProcess->setProcessEnvironment(env);

    QStringList args;

    if (!settings.autoLevels)
    {
        args << QLatin1String("-l");
        args << QString::number(settings.levels);
    }

    if (settings.ciecam02)
    {
        args << QLatin1String("-c");
    }

    if (!comp.isEmpty())
    {
        args << comp;
    }

    if (settings.hardMask)
    {
        if (d->enfuseVersion4x)
        {
            args << QLatin1String("--hard-mask");
        }
        else
        {
            args << QLatin1String("--HardMask");
        }
    }

    if (d->enfuseVersion4x)
    {
        args << QString::fromUtf8("--exposure-weight=%1").arg(settings.exposure);
        args << QString::fromUtf8("--saturation-weight=%1").arg(settings.saturation);
        args << QString::fromUtf8("--contrast-weight=%1").arg(settings.contrast);
    }
    else
    {
        args << QString::fromUtf8("--wExposure=%1").arg(settings.exposure);
        args << QString::fromUtf8("--wSaturation=%1").arg(settings.saturation);
        args << QString::fromUtf8("--wContrast=%1").arg(settings.contrast);
    }

    args << QLatin1String("-v");
    args << QLatin1String("-o");
    args << outUrl.toLocalFile();

    for (const QUrl& url : std::as_const(inUrls))
    {
        args << url.toLocalFile();
    }

    d->enfuseProcess->setProgram(enfusePath);
    d->enfuseProcess->setArguments(args);

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Enfuse command line:" << d->enfuseProcess->program();

    d->enfuseProcess->start();

    if (!d->enfuseProcess->waitForFinished(-1))
    {
        errors = getProcessError(*(d->enfuseProcess));
        return false;
    }

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Enfuse output url: " << outUrl;
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Enfuse exit status:" << d->enfuseProcess->exitStatus();
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Enfuse exit code:  " << d->enfuseProcess->exitCode();

    if (d->enfuseProcess->exitStatus() != QProcess::NormalExit)
    {
        return false;
    }

    if (d->enfuseProcess->exitCode() == 0)
    {
        // Process finished successfully !

        return true;
    }

    errors = getProcessError(*(d->enfuseProcess));

    return false;
}

QString ExpoBlendingThread::getProcessError(QProcess& proc) const
{
    QString std = QString::fromLocal8Bit(proc.readAll());

    return (i18n("Cannot run %1:\n\n %2", proc.program(), std));
}

/**
 * This function obtains the "average scene luminance" (cd/m^2) from an image file.
 * "average scene luminance" is the L (aka B) value mentioned in [1]
 * You have to take a log2f of the returned value to get an EV value.
 *
 * We are using K=12.07488f and the exif-implied value of N=1/3.125 (see [1]).
 * K=12.07488f is the 1.0592f * 11.4f value in pfscalibration's pfshdrcalibrate.cpp file.
 * Based on [3] we can say that the value can also be 12.5 or even 14.
 * Another reference for APEX is [4] where N is 0.3, closer to the APEX specification of 2^(-7/4)=0.2973.
 *
 * [1] en.wikipedia.org/wiki/APEX_system
 * [2] en.wikipedia.org/wiki/Exposure_value
 * [3] en.wikipedia.org/wiki/Light_meter
 * [4] doug.kerr.home.att.net/pumpkin/#APEX
 *
 * This function tries first to obtain the shutter speed from either of
 * two exif tags (there is no standard between camera manufacturers):
 * ExposureTime or ShutterSpeedValue.
 * Same thing for f-number: it can be found in FNumber or in ApertureValue.
 *
 * F-number and shutter speed are mandatory in exif data for EV calculation, iso is not.
 */
float ExpoBlendingThread::getAverageSceneLuminance(const QUrl& url)
{
    if (!d->meta.load(url.toLocalFile()))
    {
        return -1;
    }

    if (!d->meta.hasExif())
    {
        return -1;
    }

    long num = 1;
    long den = 1;

    // default not valid values

    float    expo = -1.0;
    float    iso  = -1.0;
    float    fnum = -1.0;

    if      (d->meta.getExifTagRational("Exif.Photo.ExposureTime", num, den))
    {
        if (den)
        {
            expo = (float)(num) / (float)(den);
        }
    }
    else if (getXmpRational("Xmp.exif.ExposureTime", num, den, &d->meta))
    {
        if (den)
        {
            expo = (float)(num) / (float)(den);
        }
    }
    else if (d->meta.getExifTagRational("Exif.Photo.ShutterSpeedValue", num, den))
    {
        long   nmr = 1;
        long   div = 1;
        double tmp = 0.0;

        if (den)
        {
            tmp = exp(log(2.0) * (float)(num) / (float)(den));
        }

        if (tmp > 1.0)
        {
            div = (long)(tmp + 0.5);
        }
        else
        {
            if ((tmp + 0.5) != 0.0)
            {
                nmr = (long)(1 / (tmp + 0.5));
            }
        }

        if (div)
        {
            expo = (float)(nmr) / (float)(div);
        }
    }
    else if (getXmpRational("Xmp.exif.ShutterSpeedValue", num, den, &d->meta))
    {
        long   nmr = 1;
        long   div = 1;
        double tmp = 0.0;

        if (den)
        {
            tmp = exp(log(2.0) * (float)(num) / (float)(den));
        }

        if (tmp > 1.0)
        {
            div = (long)(tmp + 0.5);
        }
        else
        {
            if ((tmp + 0.5) != 0.0)
            {
                nmr = (long)(1 / (tmp + 0.5));
            }
        }

        if (div)
        {
            expo = (float)(nmr) / (float)(div);
        }
    }

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << url.fileName() << ": expo =" << expo;

    if      (d->meta.getExifTagRational("Exif.Photo.FNumber", num, den))
    {
        if (den)
        {
            fnum = (float)(num) / (float)(den);
        }

    }
    else if (getXmpRational("Xmp.exif.FNumber", num, den, &d->meta))
    {
        if (den)
        {
            fnum = (float)(num) / (float)(den);
        }
    }
    else if (d->meta.getExifTagRational("Exif.Photo.ApertureValue", num, den))
    {
        if (den)
        {
            fnum = (float)(exp(log(2.0) * (float)(num) / (float)(den) / 2.0));
        }
    }
    else if (getXmpRational("Xmp.exif.ApertureValue", num, den, &d->meta))
    {
        if (den)
        {
            fnum = (float)(exp(log(2.0) * (float)(num) / (float)(den) / 2.0));
        }
    }

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << url.fileName() << ": fnum =" << fnum;

    // Some cameras/lens DO print the fnum but with value 0, and this is not allowed for ev computation purposes.

    if (fnum == 0.0)
    {
        return -1.0;
    }

    // If iso is found use that value, otherwise assume a value of iso=100. (again, some cameras do not print iso in exif).

    if      (d->meta.getExifTagRational("Exif.Photo.ISOSpeedRatings", num, den))
    {
        if (den)
        {
            iso = (float)(num) / (float)(den);
        }
    }
    else if (getXmpRational("Xmp.exif.ISOSpeedRatings", num, den, &d->meta))
    {
        if (den)
        {
            iso = (float)(num) / (float)(den);
        }
    }
    else
    {
        iso = 100.0;
    }

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << url.fileName() << ": iso =" << iso;

    // At this point the three variables have to be != -1

    if ((expo != -1.0) && (iso != -1.0) && (fnum != -1.0))
    {
        float asl = (expo * iso) / (fnum * fnum * 12.07488f);
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << url.fileName() << ": ASL ==>" << asl;

        return asl;
    }

    return -1.0;
}

bool ExpoBlendingThread::getXmpRational(const char* xmpTagName, long& num, long& den, MetaEngine* const meta)
{
    QVariant rationals = meta->getXmpTagVariant(xmpTagName);

    if (!rationals.isNull())
    {
        QVariantList list = rationals.toList();

        if (list.size() == 2)
        {
            num = list[0].toInt();
            den = list[1].toInt();

            return true;
        }
    }

    return false;
}

} // namespace DigikamGenericExpoBlendingPlugin

#include "moc_expoblendingthread.cpp"
