/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-26
 * Description : a progress bar with information dispatched to progress manager
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QProgressBar>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DProgressWdg : public QProgressBar
{
    Q_OBJECT

public:

    explicit DProgressWdg(QWidget* const parent);
    ~DProgressWdg() override;

    /**
     * Call this method to start a new instance of progress notification into progress manager
     * You can pass title string to name progress item, and set it as cancelable. In this case,
     * signalProgressCanceled() is fired when user press cancel button from progress manager.
     * This item can also accept a thumbnail that you can change through progresssThumbnailChanged().
     */
    void progressScheduled(const QString& title, bool canBeCanceled, bool hasThumb);

    /**
     * Change thumbnail in progress manager
     */
    void progressThumbnailChanged(const QPixmap& thumb);

    /**
     * Change status string in progress manager
     */
    void progressStatusChanged(const QString& status);

    /**
     * Call this method to query progress manager that process is done.
     */
    void progressCompleted();

Q_SIGNALS:

    /**
     * Fired when user press cancel button from progress manager.
     */
    void signalProgressCanceled();

private Q_SLOTS:

    void slotValueChanged(int);
    void slotProgressCanceled(const QString& id);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
