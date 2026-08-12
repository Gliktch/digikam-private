/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-10
 * Description : sharp settings view.
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

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT SharpContainer
{

public:

    enum SharpingMethods
    {
        SimpleSharp = 0,
        UnsharpMask,
        Refocus
    };

public:

    SharpContainer()  = default;
    ~SharpContainer() = default;

public:

    int    method           = SimpleSharp;      ///< Store SharpingMethods value

    /// Simple sharp
    int    ssRadius         = 0;

    /// Unsharp mask
    double umRadius         = 1.0;
    double umAmount         = 1.0;
    double umThreshold      = 0.05;
    bool   umLumaOnly       = false;

    /// Refocus
    double rfRadius         = 1.0;
    double rfCorrelation    = 0.5;
    double rfNoise          = 0.03;
    double rfGauss          = 0.0;
    int    rfMatrix         = 5;
};

// -----------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT SharpSettings : public QWidget
{
    Q_OBJECT

public:

    explicit SharpSettings(QWidget* const parent);
    ~SharpSettings() override;

    SharpContainer defaultSettings() const;
    void resetToDefault();

    SharpContainer settings()        const;
    void setSettings(const SharpContainer& settings);

    void readSettings(const KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void loadSettings();
    void saveAsSettings();

Q_SIGNALS:

    void signalSettingsChanged();

private Q_SLOTS:

    void slotSharpMethodChanged(int);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
