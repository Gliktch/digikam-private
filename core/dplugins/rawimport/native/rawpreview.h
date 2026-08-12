/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-08-04
 * Description : RAW preview widget.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QImage>
#include <QPixmap>
#include <QResizeEvent>
#include <QUrl>

// Local includes

#include "dimg.h"
#include "graphicsdimgview.h"
#include "loadingdescription.h"


using namespace Digikam;

namespace DigikamRawImportNativePlugin
{

class  RawPreview : public GraphicsDImgView
{
    Q_OBJECT

public:

    explicit RawPreview(const QUrl& url, QWidget* const parent);
    ~RawPreview()                    override;

    DImg& demosaicedImage()    const;
    DImg  postProcessedImage() const;

    void setDecodingSettings(const DRawDecoding& settings);
    void setPostProcessedImage(const DImg& image);

    void ICCSettingsChanged();
    void exposureSettingsChanged();

    void cancelLoading();

Q_SIGNALS:

    void signalLoadingStarted();
    void signalLoadingProgress(float);
    void signalLoadingFailed();
    void signalDemosaicedImage();
    void signalPostProcessedImage();

private Q_SLOTS:

    void slotLoadingProgress(const LoadingDescription& description, float progress);
    void slotImageLoaded(const LoadingDescription& description, const DImg& image);

private:

    void   setdemosaicedImg(const DImg& image);
    void   postProcessing(const DRawDecoding& settings);
    int    previewWidth()    const;
    int    previewHeight()   const;
    bool   previewIsNull()   const;
    QImage previewToQImage() const;
    void   resetPreview();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamRawImportNativePlugin
