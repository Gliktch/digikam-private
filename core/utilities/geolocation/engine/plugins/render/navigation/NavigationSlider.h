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

#include <QAbstractSlider>
#include <QPixmap>

namespace Marble
{

class NavigationSlider : public QAbstractSlider
{
    Q_OBJECT

public:

    explicit NavigationSlider(QWidget* parent = nullptr);
    ~NavigationSlider() override;

Q_SIGNALS:

    void repaintNeeded();

protected:

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    void enterEvent(QEnterEvent*) override;

#else

    void enterEvent(QEvent*) override;

#endif

    void mouseMoveEvent(QMouseEvent* mouseEvent) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void leaveEvent(QEvent*) override;
    void paintEvent(QPaintEvent*) override;
    void repaint();

private:

    static QPixmap pixmap(const QString& id);

private:

    QString m_handleImagePath;
};

} // namespace Marble
