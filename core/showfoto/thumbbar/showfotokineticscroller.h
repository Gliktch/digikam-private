/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-02-01
 * Description : Kinetic Scroller for Thumbnail Bar
 *               based on Razvan Petru implementation.
 *
 * SPDX-FileCopyrightText: 2014 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QScopedPointer>
#include <QAbstractScrollArea>
#include <QListView>

namespace ShowFoto
{

/**
 * Vertical kinetic scroller implementation without overshoot and bouncing.
 * A temporary solution to get kinetic-like scrolling on Symbian.
 */
class ShowfotoKineticScroller: public QObject
{
   Q_OBJECT

public:

    explicit ShowfotoKineticScroller(QObject* const parent = nullptr);
    ~ShowfotoKineticScroller()                       override;

    /**
     * NOTE: enabled for one widget only, new calls remove previous association
     */
    void enableKineticScrollFor(QAbstractScrollArea* const scrollArea);
    void setScrollFlow(QListView::Flow flow);

protected:

    bool eventFilter(QObject* object, QEvent* event) override;

private Q_SLOTS:

    void onKineticTimerElapsed();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace ShowFoto
