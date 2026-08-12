/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
 *
 * SPDX-FileCopyrightText: 2008      by Adrian Schroeter <adrian at suse dot de>
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "lensfunfilter.h"
#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT LensFunSettings : public QWidget
{
    Q_OBJECT

public:

    explicit LensFunSettings(QWidget* const parent = nullptr);
    ~LensFunSettings() override;

    void setEnabledCCA(bool b);
    void setEnabledVig(bool b);
    void setEnabledDist(bool b);
    void setEnabledGeom(bool b);

    LensFunContainer defaultSettings() const;
    LensFunContainer settings()        const;
    void resetToDefault();

    void assignFilterSettings(LensFunContainer& prm);
    void setFilterSettings(const LensFunContainer& settings);

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

Q_SIGNALS:

    void signalSettingsChanged();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
