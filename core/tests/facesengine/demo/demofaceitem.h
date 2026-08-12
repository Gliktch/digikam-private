/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : Demo test program for FacesEngine
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Alex Jironkin <alexjironkin at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QtGlobal>
#include <QObject>
#include <QGraphicsObject>
#include <QGraphicsRectItem>
#include <QBrush>
#include <QFont>

// Local includes

#include "demobutton.h"

namespace FaceEngineDemo
{

class FaceItem : public QGraphicsObject
{
    Q_OBJECT

public:

    explicit FaceItem(QGraphicsItem* const parent = nullptr,
                      QGraphicsScene* const scene = nullptr,
                      const QRect& rect = QRect(0, 0, 0, 0),
                      double scale = 1,
                      const QString& name = QString(),
                      double originalscale = 1);
    ~FaceItem() override;

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void setText(const QString& newName);
    QString text() const;

    QRect originalRect()   const;
    double originalScale() const ;

    void setVisible(bool);
    void setControlsVisible(bool);

Q_SIGNALS:

    void acceptButtonClicked(const QString&, const QRect&);
    void rejectButtonClicked(const QString&, const QRect&);

    void suggestionAcceptButtonClicked(const QString&, const QRect&);
    void suggestionRejectButtonClicked(const QString&, const QRect&);

public Q_SLOTS:

    void update();
    void clearText();
    void clearAndHide();
    void accepted();
    void reject();

    void suggest(const QString& name);
    void switchToEditMode();
    void switchToSuggestionMode();

    void slotSuggestionAccepted();
    void slotSuggestionRejected();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace FaceEngineDemo
