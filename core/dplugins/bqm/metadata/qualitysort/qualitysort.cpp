/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-09-22
 * Description : assign pick label metadata by image quality batch tool.
 *
 * SPDX-FileCopyrightText: 2021-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "qualitysort.h"

// Qt includes

#include <QApplication>
#include <QGridLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QFile>
#include <QMenu>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dimg.h"
#include "dinfointerface.h"
#include "dmetadata.h"
#include "dpluginbqm.h"
#include "imagequalitywidget.h"
#include "imagequalitysettings.h"
#include "imagequalityparser.h"
#include "dlayoutbox.h"
#include "previewloadthread.h"
#include "dfileoperations.h"

namespace DigikamBqmQualitySortPlugin
{

class Q_DECL_HIDDEN QualitySort::Private
{
public:

    Private() = default;

public:

    ImageQualityWidget* qualityWidget  = nullptr;
    ImageQualityParser* imgqsort       = nullptr;

    bool                changeSettings = true;
};

QualitySort::QualitySort(QObject* const parent)
    : BatchTool(QLatin1String("QualitySort"), MetadataTool, parent),
      d        (new Private)
{
}

QualitySort::~QualitySort()
{
    delete d;
}

BatchTool* QualitySort::clone(QObject* const parent) const
{
    return new QualitySort(parent);
}

void QualitySort::registerSettingsWidget()
{
    DVBox* const vbox = new DVBox;
    d->qualityWidget  = new ImageQualityWidget(ImageQualityWidget::BQM, vbox);

    m_settingsWidget  = vbox;

    connect(d->qualityWidget, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings QualitySort::defaultSettings()
{
    BatchToolSettings settings;
    ImageQualitySettings prm;

    settings.insert(QLatin1String("CustomSettingsDetectBlur"),          prm.detectBlur);
    settings.insert(QLatin1String("CustomSettingsDetectNoise"),         prm.detectNoise);
    settings.insert(QLatin1String("CustomSettingsDetectCompression"),   prm.detectCompression);
    settings.insert(QLatin1String("CustomSettingsDetectExposure"),      prm.detectExposure);
    settings.insert(QLatin1String("CustomSettingsDetectAesthetic"),     prm.detectAesthetic);
    settings.insert(QLatin1String("CustomSettingsLowQRejected"),        prm.lowQRejected);
    settings.insert(QLatin1String("CustomSettingsMediumQPending"),      prm.mediumQPending);
    settings.insert(QLatin1String("CustomSettingsHighQAccepted"),       prm.highQAccepted);
    settings.insert(QLatin1String("CustomSettingsRejectedThreshold"),   prm.rejectedThreshold);
    settings.insert(QLatin1String("CustomSettingsPendingThreshold"),    prm.pendingThreshold);
    settings.insert(QLatin1String("CustomSettingsAcceptedThreshold"),   prm.acceptedThreshold);
    settings.insert(QLatin1String("CustomSettingsBlurWeight"),          prm.blurWeight);
    settings.insert(QLatin1String("CustomSettingsNoiseWeight"),         prm.noiseWeight);
    settings.insert(QLatin1String("CustomSettingsCompressionWeight"),   prm.compressionWeight);
    settings.insert(QLatin1String("CustomSettingsExposureWeight"),      prm.exposureWeight);

    return settings;
}

void QualitySort::slotAssignSettings2Widget()
{
    d->changeSettings = false;

    ImageQualitySettings prm;

    prm.detectBlur          = settings().value(QLatin1String("CustomSettingsDetectBlur")).toBool();
    prm.detectNoise         = settings().value(QLatin1String("CustomSettingsDetectNoise")).toBool();
    prm.detectCompression   = settings().value(QLatin1String("CustomSettingsDetectCompression")).toBool();
    prm.detectExposure      = settings().value(QLatin1String("CustomSettingsDetectExposure")).toBool();
    prm.detectAesthetic     = settings().value(QLatin1String("CustomSettingsDetectAesthetic")).toBool();
    prm.lowQRejected        = settings().value(QLatin1String("CustomSettingsLowQRejected")).toBool();
    prm.mediumQPending      = settings().value(QLatin1String("CustomSettingsMediumQPending")).toBool();
    prm.highQAccepted       = settings().value(QLatin1String("CustomSettingsHighQAccepted")).toBool();
    prm.rejectedThreshold   = settings().value(QLatin1String("CustomSettingsRejectedThreshold")).toInt();
    prm.pendingThreshold    = settings().value(QLatin1String("CustomSettingsPendingThreshold")).toInt();
    prm.acceptedThreshold   = settings().value(QLatin1String("CustomSettingsAcceptedThreshold")).toInt();
    prm.blurWeight          = settings().value(QLatin1String("CustomSettingsBlurWeight")).toInt();
    prm.noiseWeight         = settings().value(QLatin1String("CustomSettingsNoiseWeight")).toInt();
    prm.compressionWeight   = settings().value(QLatin1String("CustomSettingsCompressionWeight")).toInt();
    prm.exposureWeight      = settings().value(QLatin1String("CustomSettingsExposureWeight")).toInt();

    d->qualityWidget->setSettings(prm);

    d->changeSettings = true;
}

void QualitySort::slotSettingsChanged()
{
    if (d->changeSettings)
    {
        BatchToolSettings settings;
        ImageQualitySettings prm = d->qualityWidget->settings();

        settings.insert(QLatin1String("CustomSettingsDetectBlur"),          prm.detectBlur);
        settings.insert(QLatin1String("CustomSettingsDetectNoise"),         prm.detectNoise);
        settings.insert(QLatin1String("CustomSettingsDetectCompression"),   prm.detectCompression);
        settings.insert(QLatin1String("CustomSettingsDetectExposure"),      prm.detectExposure);
        settings.insert(QLatin1String("CustomSettingsDetectAesthetic"),     prm.detectAesthetic);
        settings.insert(QLatin1String("CustomSettingsLowQRejected"),        prm.lowQRejected);
        settings.insert(QLatin1String("CustomSettingsMediumQPending"),      prm.mediumQPending);
        settings.insert(QLatin1String("CustomSettingsHighQAccepted"),       prm.highQAccepted);
        settings.insert(QLatin1String("CustomSettingsRejectedThreshold"),   prm.rejectedThreshold);
        settings.insert(QLatin1String("CustomSettingsPendingThreshold"),    prm.pendingThreshold);
        settings.insert(QLatin1String("CustomSettingsAcceptedThreshold"),   prm.acceptedThreshold);
        settings.insert(QLatin1String("CustomSettingsBlurWeight"),          prm.blurWeight);
        settings.insert(QLatin1String("CustomSettingsNoiseWeight"),         prm.noiseWeight);
        settings.insert(QLatin1String("CustomSettingsCompressionWeight"),   prm.compressionWeight);
        settings.insert(QLatin1String("CustomSettingsExposureWeight"),      prm.exposureWeight);

        BatchTool::slotSettingsChanged(settings);
    }
}

bool QualitySort::toolOperations()
{
    bool ret = true;
    QScopedPointer<DMetadata> meta(new DMetadata);
    DImg dimg;

    if (image().isNull())
    {
        if (!meta->load(inputUrl().toLocalFile()))
        {
            return false;
        }

        dimg = PreviewLoadThread::loadFastSynchronously(inputUrl().toLocalFile(), 1024);
    }
    else
    {
        meta->setData(image().getMetadata());
        QSize scaledSize = image().size();
        scaledSize.scale(1024, 1024, Qt::KeepAspectRatio);
        dimg             = image().smoothScale(scaledSize.width(), scaledSize.height());
    }

    ImageQualitySettings prm;

    prm.detectBlur          = settings().value(QLatin1String("CustomSettingsDetectBlur")).toBool();
    prm.detectNoise         = settings().value(QLatin1String("CustomSettingsDetectNoise")).toBool();
    prm.detectCompression   = settings().value(QLatin1String("CustomSettingsDetectCompression")).toBool();
    prm.detectExposure      = settings().value(QLatin1String("CustomSettingsDetectExposure")).toBool();
    prm.detectAesthetic     = settings().value(QLatin1String("CustomSettingsDetectAesthetic")).toBool();
    prm.lowQRejected        = settings().value(QLatin1String("CustomSettingsLowQRejected")).toBool();
    prm.mediumQPending      = settings().value(QLatin1String("CustomSettingsMediumQPending")).toBool();
    prm.highQAccepted       = settings().value(QLatin1String("CustomSettingsHighQAccepted")).toBool();
    prm.rejectedThreshold   = settings().value(QLatin1String("CustomSettingsRejectedThreshold")).toInt();
    prm.pendingThreshold    = settings().value(QLatin1String("CustomSettingsPendingThreshold")).toInt();
    prm.acceptedThreshold   = settings().value(QLatin1String("CustomSettingsAcceptedThreshold")).toInt();
    prm.blurWeight          = settings().value(QLatin1String("CustomSettingsBlurWeight")).toInt();
    prm.noiseWeight         = settings().value(QLatin1String("CustomSettingsNoiseWeight")).toInt();
    prm.compressionWeight   = settings().value(QLatin1String("CustomSettingsCompressionWeight")).toInt();
    prm.exposureWeight      = settings().value(QLatin1String("CustomSettingsExposureWeight")).toInt();

    PickLabel pick;
    d->imgqsort = new ImageQualityParser(dimg, prm, &pick);
    d->imgqsort->startAnalyse();

    meta->setItemPickLabel(pick);
    qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Assign Picklabel:" << pick;

    delete d->imgqsort;
    d->imgqsort = nullptr;

    if (image().isNull())
    {
        QFile::remove(outputUrl().toLocalFile());
        ret &= DFileOperations::copyFile(inputUrl().toLocalFile(), outputUrl().toLocalFile());

        if (ret)
        {
            ret &= meta->save(outputUrl().toLocalFile());
            qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Save metadata to file:" << ret;
        }
    }
    else
    {
        qCDebug(DIGIKAM_DPLUGIN_BQM_LOG) << "Save metadata to image";
        image().setMetadata(meta->data());
        ret &= savefromDImg();
    }

    return ret;
}

void QualitySort::cancel()
{
    if (d->imgqsort)
    {
        d->imgqsort->cancelAnalyse();
    }

    BatchTool::cancel();
}

} // namespace DigikamBqmQualitySortPlugin

#include "moc_qualitysort.cpp"
