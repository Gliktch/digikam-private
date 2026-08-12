/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QUrl>
#include <QWidget>

// Locale includes

#include "dsavesettingswidget.h"

class KConfigGroup;

using namespace Digikam;

namespace DigikamGenericExpoBlendingPlugin
{

class EnfuseSettings
{
public:

    EnfuseSettings()  = default;
    ~EnfuseSettings() = default;

    QString asCommentString() const;

    QString inputImagesList() const;

public:

    bool                              autoLevels    = true;
    bool                              hardMask      = false;
    bool                              ciecam02      = false;

    int                               levels        = 20;

    double                            exposure      = 1.0;
    double                            saturation    = 0.2;
    double                            contrast      = 0.0;

    QString                           targetFileName;

    QList<QUrl>                       inputUrls;
    QUrl                              previewUrl;

    DSaveSettingsWidget::OutputFormat outputFormat  = DSaveSettingsWidget::OUTPUT_PNG;
};

// ------------------------------------------------------------------------

class EnfuseSettingsWidget : public QWidget
{
    Q_OBJECT

public:

    explicit EnfuseSettingsWidget(QWidget* const parent);
    ~EnfuseSettingsWidget() override;

    void           setSettings(const EnfuseSettings& settings);
    EnfuseSettings settings() const;

    void           resetToDefault();

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericExpoBlendingPlugin
