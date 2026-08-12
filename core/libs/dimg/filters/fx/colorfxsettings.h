/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-11-08
 * Description : Color effects settings view.
 *
 * SPDX-FileCopyrightText: 2012 by Alexander Dymo <adymo at kdevelop dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include <QWidget>

// Local includes

#include "digikam_export.h"
#include "colorfxfilter.h"

class  KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT ColorFXSettings : public QWidget
{
    Q_OBJECT

public:

    explicit ColorFXSettings(QWidget* const parent, bool useGenericImg = true);
    ~ColorFXSettings() override;

    ColorFXContainer defaultSettings()          const;
    void resetToDefault();

    ColorFXContainer settings()                 const;
    void setSettings(const ColorFXContainer& settings);

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void startPreviewFilters();

Q_SIGNALS:

    void signalSettingsChanged();

private:

    void findLuts();
    QString translateLuts(const QString& name)  const;

private Q_SLOTS:

    void slotEffectTypeChanged(int type);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
