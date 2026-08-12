/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-03-15
 * Description : a tool to create panorama by fusion of several images.
 *
 * SPDX-FileCopyrightText: 2012-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "preprocesstask.h"

// Qt includes

#include <QFileInfo>
#include <QScopedPointer>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "dbinarysearch.h"
#include "digikam_debug.h"
#include "drawdecoder.h"
#include "dimg.h"
#include "dimgloaderobserver.h"
#include "drawdecoderwidget.h"
#include "drawdecoding.h"

namespace DigikamGenericPanoramaPlugin
{

class PanoObserver;

class Q_DECL_HIDDEN PreProcessTask::Private
{
public:

    // cppcheck-suppress [constParameter, constParameterCallback]
    Private(PanoramaPreprocessedUrls& urls, const QUrl& url)
        : fileUrl        (url),
          preProcessedUrl(urls)
    {
    }

    const QUrl                fileUrl;
    PanoramaPreprocessedUrls& preProcessedUrl;
    PanoObserver*             observer          = nullptr;
};

class Q_DECL_HIDDEN PanoObserver : public DImgLoaderObserver
{
public:

    explicit PanoObserver(PreProcessTask* const p)
        : DImgLoaderObserver(),
          parent            (p)
    {
    }

    ~PanoObserver() override = default;

    bool continueQuery() override
    {
        return (!parent->isAbortedFlag);
    }

private:

    PreProcessTask* const parent = nullptr;
};

PreProcessTask::PreProcessTask(const QString& workDirPath,
                               int id,
                               PanoramaPreprocessedUrls& targetUrls,
                               const QUrl& sourceUrl)
    : PanoTask(PANO_PREPROCESS_INPUT, workDirPath),
      id      (id),
      d       (new Private(targetUrls, sourceUrl))
{
    d->observer = new PanoObserver(this);
}

PreProcessTask::~PreProcessTask()
{
    delete d;
}

void PreProcessTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    // check if its a RAW file.

    if (DRawDecoder::isRawFile(d->fileUrl))
    {
        d->preProcessedUrl.preprocessedUrl = tmpDir;

        if (!convertRaw())
        {
            successFlag = false;

            return;
        }
    }
    else
    {
        // NOTE: in this case, preprocessed Url is the original file Url.

        d->preProcessedUrl.preprocessedUrl = d->fileUrl;
    }

    d->preProcessedUrl.previewUrl = tmpDir;

    if (!computePreview(d->preProcessedUrl.preprocessedUrl))
    {
        successFlag = false;

        return;
    }

    successFlag = true;
}

bool PreProcessTask::computePreview(const QUrl& inUrl)
{
    QUrl& outUrl = d->preProcessedUrl.previewUrl;

    QFileInfo fi(QDir::toNativeSeparators(inUrl.toLocalFile()));
    outUrl.setPath(outUrl.path() + fi.completeBaseName().replace(QLatin1Char('.'), QLatin1String("_"))
                                 + QLatin1String("-preview.jpg"));

    DImg img;

    if (img.load(inUrl.toLocalFile()))
    {
        DImg preview = img.smoothScale(1280, 1024, Qt::KeepAspectRatio);
        bool saved   = preview.save(outUrl.toLocalFile(), DImg::JPEG);

        // save exif information also to preview image for auto rotation

        if (saved)
        {
            QScopedPointer<DMetadata> meta(new DMetadata);

            if (meta->load(inUrl.toLocalFile()))
            {
                MetaEngine::ImageOrientation orientation = meta->getItemOrientation();

                if (meta->load(outUrl.toLocalFile()))
                {
                    meta->setItemOrientation(orientation);
                    meta->setItemDimensions(QSize(preview.width(), preview.height()));
                    meta->applyChanges(true);
                }
            }
        }

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Preview Image url: " << outUrl << ", saved: " << saved;

        return saved;
    }
    else
    {
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Error during preview generation of: " << inUrl;
        errString = i18n("Input image cannot be loaded for preview generation.");
    }

    return false;
}

bool PreProcessTask::convertRaw()
{
    const QUrl& inUrl           = d->fileUrl;
    QUrl& outUrl                = d->preProcessedUrl.preprocessedUrl;
    DImg img;

    DRawDecoding settings;
    KSharedConfig::Ptr config   = KSharedConfig::openConfig();
    KConfigGroup group          = config->group(QLatin1String("ImageViewer Settings"));
    DRawDecoderWidget::readSettings(settings.rawPrm, group);

    if (img.load(inUrl.toLocalFile(), d->observer, settings))
    {
        QFileInfo fi(inUrl.toLocalFile());
        QDir outDir(outUrl.toLocalFile());
        Q_UNUSED(outDir.cdUp());
        QString path = outDir.path() + QLatin1Char('/');
        outUrl.setPath(path + fi.completeBaseName().replace(QLatin1Char('.'), QLatin1String("_"))
                            + QLatin1String(".tif"));

        if (!img.save(outUrl.toLocalFile(), QLatin1String("TIF")))
        {
            errString = i18n("Tiff image creation failed.");

            return false;
        }

        QScopedPointer<DMetadata> meta(new DMetadata);

        if (meta->load(outUrl.toLocalFile()))
        {
            DMetadata::MetaDataMap m = meta->getExifTagsDataList(QStringList() << QLatin1String("Photo"));

            if (!m.isEmpty())
            {
                for (DMetadata::MetaDataMap::iterator it = m.begin() ; it != m.end() ; ++it)
                {
                    meta->removeExifTag(it.key().toLatin1().constData());
                }
            }

            meta->setItemDimensions(img.size());
            meta->setExifTagString("Exif.Image.DocumentName", inUrl.fileName());
            meta->setXmpTagString("Xmp.tiff.Make",  meta->getExifTagString("Exif.Image.Make"));
            meta->setXmpTagString("Xmp.tiff.Model", meta->getExifTagString("Exif.Image.Model"));
            meta->setItemOrientation(DMetadata::ORIENTATION_NORMAL);
            meta->applyChanges(true);
        }
    }
    else
    {
        errString = i18n("Raw file conversion failed.");
        return false;
    }

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Convert RAW output url: " << outUrl;

    return true;
}

} // namespace DigikamGenericPanoramaPlugin
