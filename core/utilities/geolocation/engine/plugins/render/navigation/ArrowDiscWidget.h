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

#include <QWidget>
#include <QPixmap>
#include <QTimer>

namespace Marble
{

class MarbleWidget;

class ArrowDiscWidget : public QWidget
{
    Q_OBJECT

public:

    explicit ArrowDiscWidget(QWidget* parent = nullptr);
    ~ArrowDiscWidget() override;

    void setMarbleWidget(MarbleWidget* marbleWidget);

Q_SIGNALS:

    void repaintNeeded();

protected:

    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent* mouseEvent) override;
    void mousePressEvent(QMouseEvent* mouseEvent) override;
    void mouseReleaseEvent(QMouseEvent* mouseEvent) override;
    void leaveEvent(QEvent* event) override;
    void repaint();

private Q_SLOTS:

    void startPressRepeat();
    void repeatPress();

private:

    static QPixmap pixmap(const QString& id);
    Qt::ArrowType arrowUnderMouse(const QPoint& position) const;

private:

    QTimer          m_initialPressTimer;
    QTimer          m_repeatPressTimer;
    Qt::ArrowType   m_arrowPressed;
    int             m_repetitions;

    MarbleWidget*   m_marbleWidget    = nullptr;
    QString         m_imagePath;
};

} // namespace Marble
