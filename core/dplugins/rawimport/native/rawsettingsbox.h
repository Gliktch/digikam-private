/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-08-11
 * Description : Raw import settings box
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

#include "editortoolsettings.h"
#include "dimg.h"
#include "curveswidget.h"

using namespace Digikam;

namespace DigikamRawImportNativePlugin
{

class RawSettingsBox : public EditorToolSettings
{
    Q_OBJECT

public:

    explicit RawSettingsBox(const QUrl& url, QWidget* const parent);
    ~RawSettingsBox()                   override;

    void setBusy(bool b)                override;

    CurvesWidget* curvesWidget() const;
    DRawDecoding  settings()     const;

    void writeSettings()                override;
    void readSettings()                 override;

    void setDemosaicedImage(DImg& img);
    void setPostProcessedImage(const DImg& img);

    void resetSettings()                override;

    void enableUpdateBtn(bool b);
    bool updateBtnEnabled() const;

Q_SIGNALS:

    void signalUpdatePreview();
    void signalAbortPreview();
    void signalPostProcessingChanged();

private Q_SLOTS:

    void slotDemosaicingChanged();
    void slotResetCurve();
    void slotFileDialogAboutToOpen();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamRawImportNativePlugin
