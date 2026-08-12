/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-23
 * Description : Autodetect enfuse binary program and version
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "dbinaryiface.h"

using namespace Digikam;

namespace DigikamGenericExpoBlendingPlugin
{

class EnfuseBinary : public DBinaryIface
{
    Q_OBJECT

public:

    EnfuseBinary()
        : DBinaryIface(QLatin1String("enfuse"),
                       QLatin1String("3.2"),
                       QLatin1String("enfuse "),
                       0,
                       QLatin1String("Enblend"),
                       QLatin1String("https://sourceforge.net/projects/enblend/files/latest/download"),
                       QLatin1String("ExpoBlending"),
                       QStringList(QLatin1String("-V"))),
                       versionDouble(0)
    {
        setup();
    }

    ~EnfuseBinary() override = default;

    double getVersion() const;

Q_SIGNALS:

    void signalEnfuseVersion(double version);

protected:

    bool parseHeader(const QString& output) override;

private:

    double versionDouble;

private:

    /// @note disabled
    explicit EnfuseBinary(QObject*) = delete;
};

} // namespace DigikamGenericExpoBlendingPlugin
