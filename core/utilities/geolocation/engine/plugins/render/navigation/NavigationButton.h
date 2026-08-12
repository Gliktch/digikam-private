/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractButton>

namespace Marble
{

class NavigationButton : public QAbstractButton
{
    Q_OBJECT

public:

    explicit NavigationButton(QWidget* parent = nullptr);

Q_SIGNALS:

    void repaintNeeded();

protected:

    void mousePressEvent(QMouseEvent* mouseEvent) override;
    void mouseReleaseEvent(QMouseEvent* mouseEvent) override;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    void enterEvent(QEnterEvent* e) override;

#else

    void enterEvent(QEvent* e) override;

#endif

    void leaveEvent(QEvent* e) override;
    void changeEvent(QEvent* e) override;
    void paintEvent(QPaintEvent*) override;

private:

    QIcon::Mode m_iconMode;
};

} // namespace Marble
