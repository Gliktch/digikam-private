/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-21
 * Description : a bar to indicate pending metadata
 *
 * SPDX-FileCopyrightText: 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "metadatahubmngr.h"
#include "progressmanager.h"

namespace Digikam
{

class MetadataStatusBar : public QWidget
{
    Q_OBJECT

public:

    explicit MetadataStatusBar(QWidget* const parent);
    ~MetadataStatusBar() override;

public Q_SLOTS:

    void slotSettingsChanged();
    void slotSetPendingItems(int number);
    void slotAddedProgressItem(ProgressItem* item);
    void slotCompletedProgressItem(ProgressItem* item);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
