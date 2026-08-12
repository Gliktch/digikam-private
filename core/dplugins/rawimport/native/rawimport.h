/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : Raw import tool
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>

// Local includes

#include "editortool.h"
#include "dimg.h"

using namespace Digikam;

namespace DigikamRawImportNativePlugin
{

class DRawDecoderSettings;

class RawImport : public EditorToolThreaded
{
    Q_OBJECT

public:

    explicit RawImport(const QUrl& url, QObject* const parent);
    ~RawImport()                                    override;

    DRawDecoding rawDecodingSettings()      const;
    DImg         postProcessedImage()       const;
    bool         hasPostProcessedImage()    const;
    bool         demosaicingSettingsDirty() const;

private:

    void setBusy(bool busy)                         override;
    void preparePreview()                           override;
    void setPreviewImage()                          override;
    void setBackgroundColor(const QColor& bg)       override;
    void ICCSettingsChanged()                       override;
    void exposureSettingsChanged()                  override;

private Q_SLOTS:

    void slotInit() override;

    void slotLoadingStarted();
    void slotDemosaicedImage();
    void slotLoadingFailed();
    void slotLoadingProgress(float);
    void slotScaleChanged()                         override;

    void slotUpdatePreview();
    void slotAbort()                                override;

    void slotOk()                                   override;
    void slotCancel()                               override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamRawImportNativePlugin
