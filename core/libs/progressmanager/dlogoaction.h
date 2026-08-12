/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-27-08
 * Description : a tool bar action object to display animated logo during long operations
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAction>
#include <QToolButton>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DLogoAction : public QAction
{
    Q_OBJECT

public:

    explicit DLogoAction(QObject* const parent);
    ~DLogoAction()                          override;

    void start();
    void stop();
    bool running()                    const;

    /**
     * @brief Method to apply a personalized style to the button hosting the DLogoAction instance in a toolbar.
     * This remove the automatic keyboard shortcut underline in the action text.
     * @arg btn is the button instance used to host the action in the toolbar.
     */
    static void applyStyleForToolBarButton(QToolButton* const btn);

private Q_SLOTS:

    void slotProgressTimerDone();

private:

    /**
     * @brief Generate one animate frame with the lightning.
     */
    QPixmap renderAnimationFrame(int beamPosition);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
