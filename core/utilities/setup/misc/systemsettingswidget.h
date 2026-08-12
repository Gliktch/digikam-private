/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-27-07
 * Description : system settings widget
 *
 * SPDX-FileCopyrightText: 2025      by Michael Miller <michael underscore miller at msn dot com>
 * SPDX-FileCopyrightText: 2025      by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2020-2025 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT SystemSettingsWidget : public QWidget
{
    Q_OBJECT

public:

    explicit SystemSettingsWidget(QWidget* const parent);
    ~SystemSettingsWidget() override;

public:

    void readSettings();
    void saveSettings();

private Q_SLOTS:

    void slotBinaryDownload();
    void slotOpenCLDNNTest();
    void slotOpenCLDNNTestFinished();
    void slotOpenLocalStorage();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
