/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-13
 * Description : progress manager
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2004      by Till Adam <adam at kde dot org>
 * SPDX-FileCopyrightText: 2004      by David Faure <faure at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QFrame>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class ProgressItem;
class ProgressView;

class DIGIKAM_EXPORT StatusbarProgressWidget : public QFrame
{
    Q_OBJECT

public:

    StatusbarProgressWidget(ProgressView* const progressView, QWidget* const parent, bool button = true);
    ~StatusbarProgressWidget() override;

public Q_SLOTS:

    void slotClean();

    void slotProgressItemAdded(ProgressItem* i);
    void slotProgressItemCompleted(ProgressItem* i);
    void slotProgressItemProgress(ProgressItem* i, unsigned int value);

protected Q_SLOTS:

    void slotProgressViewVisible(bool);
    void slotShowItemDelayed();
    void slotBusyIndicator();
    void updateBusyMode();

protected:

    void setMode();
    void connectSingleItem();
    void activateSingleItemMode();

    bool eventFilter(QObject*, QEvent*) override;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
