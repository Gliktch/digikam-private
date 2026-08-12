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

#include <QObject>
#include <QSharedPointer>
#include <QPinchGesture>

// Local includes

#include "MarbleInputHandler.h"

namespace Marble
{

class MarbleWidget;
class RenderPlugin;
class MarbleWidgetInputHandlerPrivate;

class DIGIKAM_EXPORT MarbleWidgetInputHandler : public MarbleDefaultInputHandler
{
    Q_OBJECT

public:

    MarbleWidgetInputHandler(MarbleAbstractPresenter* marblePresenter, MarbleWidget* marbleWidget);

    void setDebugModeEnabled(bool enabled);

private Q_SLOTS:

    void installPluginEventFilter(RenderPlugin* renderPlugin)   override;
    void showLmbMenu(int x, int y)                              override;
    void showRmbMenu(int x, int y)                              override;
    void openItemToolTip()                                      override;
    void setCursor(const QCursor& cursor)                       override;

private:

    bool handleKeyPress(QKeyEvent* event)                       override;
    bool handleTouch(QTouchEvent* event)                        override;
    bool handleGesture(QGestureEvent* e)                        override;
    AbstractSelectionRubber* selectionRubber()                  override;
    bool layersEventFilter(QObject* o, QEvent* e)               override;

    void handlePinchGesture(QPinchGesture* pinch);

    Q_DISABLE_COPY(MarbleWidgetInputHandler)

private:

    using MarbleWidgetInputHandlerPrivatePtr = QSharedPointer<MarbleWidgetInputHandlerPrivate>;

    MarbleWidgetInputHandlerPrivatePtr d;      // cppcheck-suppress duplInheritedMember

    friend class MarbleWidgetInputHandlerPrivate;
};

} // Namespace Marble
