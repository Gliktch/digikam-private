/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-23-03
 * Description : A tool tip widget which follows cursor movements.
 *               Tool tip content is displayed without delay.
 *
 * SPDX-FileCopyrightText: 2007-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QEvent>
#include <QString>
#include <QLabel>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/**
 * This class implements a window which looks like a tool tip. It will follow the cursor
 * when it's over a specified widget.
 */
class DIGIKAM_EXPORT DCursorTracker : public QLabel
{
    Q_OBJECT

public:

    explicit DCursorTracker(const QString& txt, QWidget* const parent, Qt::Alignment align = Qt::AlignCenter);
    ~DCursorTracker()                   override;

    void setText(const QString& txt);
    void setEnable(bool b);
    void setKeepOpen(bool b);
    void setTrackerAlignment(Qt::Alignment alignment);

    void triggerAutoShow(int timeout = 2000);
    void refresh();

protected:

    bool eventFilter(QObject*, QEvent*) override;
    void paintEvent(QPaintEvent*)       override;

private Q_SLOTS:

    void slotAutoHide();

private:

    void moveToParent(QWidget* const parent);
    bool canBeDisplayed();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
