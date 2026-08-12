/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-21
 * Description : a view to show Queue Settings.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QScrollArea>
#include <QList>
#include <QMap>
#include <QTabWidget>

namespace Digikam
{

class AssignedBatchTools;
class QueueSettings;

class QueueSettingsView : public QTabWidget
{
    Q_OBJECT

public:

    explicit QueueSettingsView(QWidget* const parent = nullptr);
    ~QueueSettingsView() override;

    void setBusy(bool b);

Q_SIGNALS:

    void signalSettingsChanged(const QueueSettings&);

public Q_SLOTS:

    void slotQueueSelected(int, const QueueSettings&, const AssignedBatchTools&);

private Q_SLOTS:

    void slotUseOrgAlbum();
    void slotResetSettings();
    void slotSettingsChanged();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
