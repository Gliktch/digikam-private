/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-10
 * Description : Film Grain settings view.
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
#include "filmgrainfilter.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT FilmGrainSettings : public QWidget
{
    Q_OBJECT

public:

    explicit FilmGrainSettings(QWidget* const parent);
    ~FilmGrainSettings() override;

    FilmGrainContainer defaultSettings() const;
    void resetToDefault();

    FilmGrainContainer settings()        const;
    void setSettings(const FilmGrainContainer& settings);

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

Q_SIGNALS:

    void signalSettingsChanged();

private Q_SLOTS:

    void slotItemToggled(int, bool);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
