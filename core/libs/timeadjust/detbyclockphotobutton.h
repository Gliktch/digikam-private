/* ============================================================
 *
 * This file is a part of the digikam project
 * https://www.digikam.org
 *
 * Date        : 2017-01-01
 * Description : button for choosing time difference photo which accepts drag & drop
 *
 * SPDX-FileCopyrightText: 2017 by Markus Leuthold <kusi at forum dot titlis dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPushButton>
#include <QDragEnterEvent>

namespace Digikam
{

class DetByClockPhotoButton : public QPushButton
{
    Q_OBJECT

public:

    explicit DetByClockPhotoButton(const QString& text);

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event)   override;
    void dropEvent(QDropEvent* event)           override;

Q_SIGNALS:

    void signalClockPhotoDropped(const QUrl&);

private:

    /// @note disabled
    DetByClockPhotoButton(QWidget*);
};

} // namespace Digikam
