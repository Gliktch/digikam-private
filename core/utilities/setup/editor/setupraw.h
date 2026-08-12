/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : Editor RAW decoding settings.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"

class QTabWidget;

namespace Digikam
{

class DIGIKAM_EXPORT SetupRaw : public QObject
{
    Q_OBJECT

public:

    explicit SetupRaw(QTabWidget* const tab);
    ~SetupRaw() override;

    void applySettings();
    void readSettings();

private Q_SLOTS:

    void slotSixteenBitsImageToggled(bool);
    void slotBehaviorChanged();
    void slotAboutRawImportPlugin();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
