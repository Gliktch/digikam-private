/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-27
 * Description : a view to show Batch Tool Settings.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QStackedWidget>

// Local includes

#include "batchtool.h"
#include "batchtoolutils.h"

namespace Digikam
{

class ToolSettingsView : public QStackedWidget
{
    Q_OBJECT

public:

    explicit ToolSettingsView(QWidget* const parent = nullptr);
    ~ToolSettingsView() override;

    void setBusy(bool b);

Q_SIGNALS:

    void signalSettingsChanged(const BatchToolSet&);

public Q_SLOTS:

    void slotToolSelected(const BatchToolSet&);

private Q_SLOTS:

    void slotSettingsChanged(const BatchToolSettings&);
    void slotThemeChanged();
    void slotAboutPlugin();
    void slotHelpPlugin();

private:

    int  viewMode() const;
    void setViewMode(int mode);
    void setToolSettingsWidget(QWidget* const w);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
