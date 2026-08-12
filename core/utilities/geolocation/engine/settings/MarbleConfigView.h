/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-12-21
 * Description : Marble Settings View.
 *
 * SPDX-FileCopyrightText: 2009      by Bastian Holst <bastianholst at gmx dot de>
 * SPDX-FileCopyrightText: 2022-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTabWidget>

// Local includes

#include "MarbleGlobal.h"
#include "MarbleLocale.h"
#include "digikam_export.h"

namespace Marble
{

class MarbleWidget;

class DIGIKAM_EXPORT MarbleConfigView : public QTabWidget
{
    Q_OBJECT

public:

    explicit MarbleConfigView(MarbleWidget* const marbleWidget,
                              QWidget* const parent = nullptr);
    ~MarbleConfigView() override;

    void readSettings();
    void applySettings();
    void cancel();

private:

    Q_DISABLE_COPY(MarbleConfigView)

    class Private;
    Private* const d = nullptr;
};

} // namespace Marble
