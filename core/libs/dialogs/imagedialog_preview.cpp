/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-03-13
 * Description : Image files selection dialog - Image Information and preview.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "imagedialog_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN ImageDialogPreview::Private
{
public:

    Private() = default;

public:

    QLabel*              imageLabel      = nullptr;
    QLabel*              infoLabel       = nullptr;

    QUrl                 currentURL;

    ThumbnailLoadThread* thumbLoadThread = nullptr;
};

ImageDialogPreview::ImageDialogPreview(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    d->thumbLoadThread       = ThumbnailLoadThread::defaultThread();

    QVBoxLayout* const vlay  = new QVBoxLayout(this);
    d->imageLabel            = new QLabel(this);
    d->imageLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    d->imageLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    d->infoLabel             = new QLabel(this);
    d->infoLabel->setAlignment(Qt::AlignCenter);

    vlay->setContentsMargins(QMargins());
    vlay->setSpacing(layoutSpacing());
    vlay->addWidget(d->imageLabel);
    vlay->addWidget(d->infoLabel);
    vlay->addStretch();

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnail(LoadingDescription,QPixmap)));
}

ImageDialogPreview::~ImageDialogPreview()
{
    delete d;
}

QSize ImageDialogPreview::sizeHint() const
{
    return QSize(256, 256);
}

void ImageDialogPreview::resizeEvent(QResizeEvent*)
{
    QMetaObject::invokeMethod(this, "showPreview", Qt::QueuedConnection);
}

void ImageDialogPreview::showPreview()
{
    QUrl url(d->currentURL);
    slotClearPreview();
    slotShowPreview(url);
}

void ImageDialogPreview::slotShowPreview(const QUrl& url)
{
    if (!url.isValid())
    {
        slotClearPreview();
        return;
    }

    if (url != d->currentURL)
    {
        slotClearPreview();
        d->currentURL    = url;
        d->thumbLoadThread->find(ThumbnailIdentifier(d->currentURL.toLocalFile()));

        QString identify = identifyItem(d->currentURL);

        if (!identify.isEmpty())
        {
            d->infoLabel->setText(identify);
        }
        else
        {
            d->infoLabel->clear();
        }
    }
}

QString ImageDialogPreview::identifyItem(const QUrl& url, const QImage& preview)
{
    QString identify;
    QScopedPointer<DMetadata> meta(new DMetadata);

    // NOTE: even if metadata loading fail, continue to fill information with empty properties.

    (void)meta->load(url.toLocalFile());

    PhotoInfoContainer info      = meta->getPhotographInformation();
    VideoInfoContainer videoInfo = meta->getVideoInformation();

    DToolTipStyleSheet cnt;
    identify = QLatin1String("<qt><center>");
    QString make, model, dateTime, aperture, focalLength, exposureTime, sensitivity;
    QString aspectRatio, audioBitRate, audioChannelType, audioCodec, duration, frameRate, videoCodec;

    if (info.make.isEmpty())
    {
        make = cnt.unavailable;
    }
    else
    {
        make = info.make;
    }

    if (info.model.isEmpty())
    {
        model = cnt.unavailable;
    }
    else
    {
        model = info.model;
    }

    if (!info.dateTime.isValid())
    {
        dateTime = cnt.unavailable;
    }
    else
    {
        dateTime = QLocale().toString(info.dateTime, QLocale::ShortFormat);
    }

    if (info.aperture.isEmpty())
    {
        aperture = cnt.unavailable;
    }
    else
    {
        aperture = info.aperture;
    }

    if (info.focalLength.isEmpty())
    {
        focalLength = cnt.unavailable;
    }
    else
    {
        focalLength = info.focalLength;
    }

    if (info.exposureTime.isEmpty())
    {
        exposureTime = cnt.unavailable;
    }
    else
    {
        exposureTime = info.exposureTime;
    }

    if (info.sensitivity.isEmpty())
    {
        sensitivity = cnt.unavailable;
    }
    else
    {
        sensitivity = i18n("%1 ISO", info.sensitivity);
    }

    if (videoInfo.aspectRatio.isEmpty())
    {
        aspectRatio = cnt.unavailable;
    }
    else
    {
        aspectRatio = videoInfo.aspectRatio;
    }

    if (videoInfo.audioBitRate.isEmpty())
    {
        audioBitRate = cnt.unavailable;
    }
    else
    {
        audioBitRate = videoInfo.audioBitRate;
    }

    if (videoInfo.audioChannelType.isEmpty())
    {
        audioChannelType = cnt.unavailable;
    }
    else
    {
        audioChannelType = videoInfo.audioChannelType;
    }

    if (videoInfo.audioCodec.isEmpty())
    {
        audioCodec = cnt.unavailable;
    }
    else
    {
        audioCodec = videoInfo.audioCodec;
    }

    if (videoInfo.duration.isEmpty())
    {
        duration = cnt.unavailable;
    }
    else
    {
        duration = videoInfo.duration;
    }

    if (videoInfo.frameRate.isEmpty())
    {
        frameRate = cnt.unavailable;
    }
    else
    {
        frameRate = videoInfo.frameRate;
    }

    if (videoInfo.videoCodec.isEmpty())
    {
        videoCodec = cnt.unavailable;
    }
    else
    {
        videoCodec = videoInfo.videoCodec;
    }

    identify += QLatin1String("<table cellspacing=0 cellpadding=0>");

    if (!preview.isNull())
    {
        QImage img = preview.scaled(QSize(64, 64), Qt::KeepAspectRatio);
        QByteArray byteArray;
        QBuffer    buffer(&byteArray);
        img.save(&buffer, "PNG");
        identify += cnt.cellBeg +
                    i18n("<i>Preview:</i>") +
                    cnt.cellMid +
                    QString::fromLatin1("<img src=\"data:image/png;base64,%1\">").arg(QString::fromLatin1(byteArray.toBase64().data())) +
                    cnt.cellEnd;
    }

    identify += cnt.cellBeg + i18n("<i>Make:</i>")              + cnt.cellMid + make                + cnt.cellEnd;
    identify += cnt.cellBeg + i18n("<i>Model:</i>")             + cnt.cellMid + model               + cnt.cellEnd;
    identify += cnt.cellBeg + i18n("<i>Created:</i>")           + cnt.cellMid + dateTime            + cnt.cellEnd;
    identify += cnt.cellBeg + i18n("<i>Aperture:</i>")          + cnt.cellMid + aperture            + cnt.cellEnd;
    identify += cnt.cellBeg + i18n("<i>Focal:</i>")             + cnt.cellMid + focalLength         + cnt.cellEnd;
    identify += cnt.cellBeg + i18n("<i>Exposure:</i>")          + cnt.cellMid + exposureTime        + cnt.cellEnd;
    identify += cnt.cellBeg + i18n("<i>Sensitivity:</i>")       + cnt.cellMid + sensitivity         + cnt.cellEnd;

    if (
        QMimeDatabase().mimeTypeForFile(url.toLocalFile()).name().startsWith(QLatin1String("video/")) &&
        !videoInfo.isEmpty()
       )
    {
        identify += cnt.cellBeg + i18n("<i>AspectRatio:</i>")       + cnt.cellMid + aspectRatio         + cnt.cellEnd;
        identify += cnt.cellBeg + i18n("<i>AudioBitRate:</i>")      + cnt.cellMid + audioBitRate        + cnt.cellEnd;
        identify += cnt.cellBeg + i18n("<i>AudioChannelType:</i>")  + cnt.cellMid + audioChannelType    + cnt.cellEnd;
        identify += cnt.cellBeg + i18n("<i>AudioCodec:</i>")        + cnt.cellMid + audioCodec          + cnt.cellEnd;
        identify += cnt.cellBeg + i18n("<i>Duration:</i>")          + cnt.cellMid + duration            + cnt.cellEnd;
        identify += cnt.cellBeg + i18n("<i>FrameRate:</i>")         + cnt.cellMid + frameRate           + cnt.cellEnd;
        identify += cnt.cellBeg + i18n("<i>VideoCodec:</i>")        + cnt.cellMid + videoCodec          + cnt.cellEnd;
    }

    identify += QLatin1String("</table></center></qt>");

    return identify;
}

void ImageDialogPreview::slotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    if (QUrl::fromLocalFile(desc.filePath) == d->currentURL)
    {
        QPixmap pixmap;
        QSize   s = d->imageLabel->contentsRect().size();

        if ((s.width() < pix.width()) || (s.height() < pix.height()))
        {
            pixmap = pix.scaled(s, Qt::KeepAspectRatio);
        }
        else
        {
            pixmap = pix;
        }

        d->imageLabel->setPixmap(pixmap);
    }
}

void ImageDialogPreview::slotClearPreview()
{
    d->imageLabel->clear();
    d->infoLabel->clear();
    d->currentURL = QUrl();
}

} // namespace Digikam
