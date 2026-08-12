/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-15
 * Description : Anti vignetting settings view.
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include <QWidget>

// Local includes

#include "digikam_export.h"
#include "antivignettingfilter.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT AntiVignettingSettings : public QWidget
{
    Q_OBJECT

public:

    explicit AntiVignettingSettings(QWidget* parent);
    ~AntiVignettingSettings() override;

    AntiVignettingContainer defaultSettings() const;
    void resetToDefault();

    AntiVignettingContainer settings()        const;
    void setSettings(const AntiVignettingContainer& settings);

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void setMaskPreviewPixmap(const QPixmap& pix);

Q_SIGNALS:

    void signalSettingsChanged();

private Q_SLOTS:

    void slotSettingsChanged();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
