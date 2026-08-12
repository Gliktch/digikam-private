/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-08-04
 * Description : RAW postProcessedImg widget.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "rawpreview.h"

// Qt includes

#include <QStyle>
#include <QString>
#include <QPainter>
#include <QFileInfo>
#include <QFontMetrics>
#include <QApplication>
#include <QGridLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "managedloadsavethread.h"
#include "editorcore.h"
#include "singlephotopreviewlayout.h"
#include "imagepreviewitem.h"
#include "paniconwidget.h"

namespace DigikamRawImportNativePlugin
{

class Q_DECL_HIDDEN RawPreview::Private
{
public:

    Private() = default;

public:

    double                 currentFitWindowZoom = 0.0;

    QUrl                   url;

    DImg                   demosaicedImg;

    DRawDecoding           settings;
    ManagedLoadSaveThread* thread               = nullptr;
    LoadingDescription     loadingDesc;
    ImagePreviewItem*      item                 = nullptr;
};

RawPreview::RawPreview(const QUrl& url, QWidget* const parent)
    : GraphicsDImgView(parent),
      d               (new Private)
{
    d->item   = new ImagePreviewItem();
    setItem(d->item);

    d->url    = url;
    d->thread = new ManagedLoadSaveThread;
    d->thread->setLoadingPolicy(ManagedLoadSaveThread::LoadingPolicyFirstRemovePrevious);

    // ------------------------------------------------------------

    // set default zoom

    layout()->fitToWindow();

    // ---

    PanIconWidget* const pan = installPanIcon();

    connect(d->item, &ImagePreviewItem::imageChanged,
            this, [this]()
        {
            updatePanIconWidget();
        }
    );

    // ---

    QGridLayout* const grid = new QGridLayout(this);
    grid->addWidget(pan, 2, 2, 1, 1);
    grid->setContentsMargins(QMargins(0, 0,
                                      QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent),
                                      QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent)));
    grid->setSpacing(layoutSpacing());
    grid->setRowStretch(1, 1);
    grid->setColumnStretch(1, 1);

    // ---

    setMinimumWidth(500);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ------------------------------------------------------------

    connect(d->thread, SIGNAL(signalImageLoaded(LoadingDescription,DImg)),
            this, SLOT(slotImageLoaded(LoadingDescription,DImg)));

    connect(d->thread, SIGNAL(signalLoadingProgress(LoadingDescription,float)),
            this, SLOT(slotLoadingProgress(LoadingDescription,float)));
}

RawPreview::~RawPreview()
{
    delete d->item;
    delete d;
}

void RawPreview::setPostProcessedImage(const DImg& image)
{
    d->item->setImage(image);
}

DImg RawPreview::postProcessedImage() const
{
    return d->item->image();
}

DImg& RawPreview::demosaicedImage() const
{
    return d->demosaicedImg;
}

void RawPreview::setDecodingSettings(const DRawDecoding& settings)
{
    if ((d->settings == settings) && d->thread->isRunning())
    {
        return;
    }

    // Save post processing settings.

    d->settings                     = settings;

    // All post processing settings will be used after demosaicing.

    DRawDecoding demosaisedSettings = settings;
    demosaisedSettings.resetPostProcessingSettings();

    d->loadingDesc                  = LoadingDescription(d->url.toLocalFile(), demosaisedSettings);
    d->thread->load(d->loadingDesc, ManagedLoadSaveThread::LoadingPolicyFirstRemovePrevious);

    Q_EMIT signalLoadingStarted();
}

void RawPreview::exposureSettingsChanged()
{
    viewport()->update();
}

void RawPreview::ICCSettingsChanged()
{
    viewport()->update();
}

void RawPreview::cancelLoading()
{
    d->thread->stopLoading(d->loadingDesc);
}

void RawPreview::slotLoadingProgress(const LoadingDescription& description, float progress)
{
    if (description.filePath != d->loadingDesc.filePath)
    {
        return;
    }

    Q_EMIT signalLoadingProgress(progress);
}

void RawPreview::slotImageLoaded(const LoadingDescription& description, const DImg& image)
{
    if (description.filePath != d->loadingDesc.filePath)
    {
        return;
    }

    if (image.isNull())
    {
        QString msg    = i18n("Cannot decode RAW image\n\"%1\"",
                              QFileInfo(d->loadingDesc.filePath).fileName());
        QFontMetrics fontMt(font());
        QRect fontRect = fontMt.boundingRect(0, 0, width(), height(), 0, msg);
        QPixmap pix(fontRect.size());
        pix.fill(qApp->palette().color(QPalette::Base));
        QPainter p(&pix);
        p.setPen(QPen(qApp->palette().color(QPalette::Text)));
        p.drawText(0, 0, pix.width(), pix.height(), Qt::AlignCenter | Qt::TextWordWrap, msg);
        p.end();

        // three copies - but the image is small

        setPostProcessedImage(DImg(pix.toImage()));
        Q_EMIT signalLoadingFailed();
    }
    else
    {
        d->demosaicedImg = image;

        Q_EMIT signalDemosaicedImage();

        // NOTE: we will apply all Raw post processing corrections in RawImport class.
    }
}

int RawPreview::previewWidth() const
{
    return d->item->image().width();
}

int RawPreview::previewHeight() const
{
    return d->item->image().height();
}

bool RawPreview::previewIsNull() const
{
    return d->item->image().isNull();
}

void RawPreview::resetPreview()
{
    d->item->setImage(DImg());
    d->loadingDesc = LoadingDescription();
    update();
}

QImage RawPreview::previewToQImage() const
{
    return d->item->image().copyQImage();
}

} // namespace DigikamRawImportNativePlugin

#include "moc_rawpreview.cpp"
