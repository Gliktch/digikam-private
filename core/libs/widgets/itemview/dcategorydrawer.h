/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-01-16
 * Description : drawing item view based on DCategorizedView
 *
 * SPDX-FileCopyrightText: 2007      by Rafael Fernández López <ereslibre at kde dot org>
 * SPDX-FileCopyrightText: 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QMouseEvent>

// Local includes

#include "digikam_export.h"

class QPainter;
class QModelIndex;
class QStyleOption;

namespace Digikam
{

class DCategorizedView;

/**
 * The category drawing is performed by this class. It also gives information about the category
 * height and margins.
 */
class DIGIKAM_EXPORT DCategoryDrawer : public QObject       // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT

    friend class DCategorizedView;

public:

    /**
     * @brief Construct a category drawer for a given view
     */
    explicit DCategoryDrawer(DCategorizedView* const view);
    ~DCategoryDrawer()                                            override;

    /**
     * @return The view this category drawer is associated with.
     */
    DCategorizedView* view()                                const;

    /**
     * @brief This method purpose is to draw a category represented by the given
     *
     * @param index    The index with the given
     * @param sortRole The sorting role
     * @param option   The painter style options
     * @param painter  The painter instance
     *
     * @note This method will be called one time per category, always with the
     *       first element in that category
     */
    virtual void drawCategory(const QModelIndex& index,
                              int sortRole,
                              const QStyleOption& option,
                              QPainter* painter)            const;

    /**
     * @return The category height for the category represented by index @p index with
     *         style options @p option.
     */
    virtual int categoryHeight(const QModelIndex& index,
                               const QStyleOption& option)  const;

    /**
     * @note 0 by default
     */
    virtual int leftMargin()                                const;

    /**
     * @note 0 by default
     */
    virtual int rightMargin()                               const;

Q_SIGNALS:

    /**
     * @brief This signal becomes emitted when collapse or expand has been clicked.
     */
    void collapseOrExpandClicked(const QModelIndex& index);

    /**
     * @brief Emit this signal on your subclass implementation to notify that something happened. Usually
     * this will be triggered when you have received an event, and its position matched some "hot spot".
     *
     * You give this action the integer you want, and having connected this signal to your code,
     * the connected slot can perform the needed changes (view, model, selection model, delegate...)
     */
    void actionRequested(int action, const QModelIndex& index);

protected:

    /**
     * @brief Method called when the mouse button has been pressed.
     *
     * @param index     The representative index of the block of items.
     * @param blockRect The rect occupied by the block of items.
     * @param event     The mouse event.
     *
     * @warning You explicitly have to determine whether the event has been accepted or not. You
     *          have to call event->accept() or event->ignore() at all possible case branches in
     *          your code.
     */
    virtual void mouseButtonPressed(const QModelIndex& index, const QRect& blockRect, QMouseEvent* event);

    /**
     * @brief Method called when the mouse button has been released.
     *
     * @param index     The representative index of the block of items.
     * @param blockRect The rect occupied by the block of items.
     * @param event     The mouse event.
     *
     * @warning You explicitly have to determine whether the event has been accepted or not. You
     *          have to call event->accept() or event->ignore() at all possible case branches in
     *          your code.
     */
    virtual void mouseButtonReleased(const QModelIndex& index, const QRect& blockRect, QMouseEvent* event);

    /**
     * @brief Method called when the mouse has been moved.
     *
     * @param index     The representative index of the block of items.
     * @param blockRect The rect occupied by the block of items.
     * @param event     The mouse event.
     */
    virtual void mouseMoved(const QModelIndex& index, const QRect& blockRect, QMouseEvent* event);

    /**
     * @brief Method called when the mouse button has been double clicked.
     *
     * @param index     The representative index of the block of items.
     * @param blockRect The rect occupied by the block of items.
     * @param event     The mouse event.
     *
     * @warning You explicitly have to determine whether the event has been accepted or not. You
     *          have to call event->accept() or event->ignore() at all possible case branches in
     *          your code.
     */
    virtual void mouseButtonDoubleClicked(const QModelIndex& index, const QRect& blockRect, QMouseEvent* event);

    /**
     * @brief Method called when the mouse button has left this block.
     *
     * @param index     The representative index of the block of items.
     * @param blockRect The rect occupied by the block of items.
     */
    virtual void mouseLeft(const QModelIndex& index, const QRect& blockRect);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
