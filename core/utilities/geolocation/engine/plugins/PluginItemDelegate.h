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

#include <QAbstractItemDelegate>
#include <QModelIndex>
#include <QIcon>

class QPainter;
class QRect;
class QStyleOptionButton;
class QStyleOptionViewItem;

namespace Marble
{

class PluginItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:

    explicit PluginItemDelegate(QAbstractItemView* view, QObject* parent = nullptr);
    ~PluginItemDelegate() override;

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void setAboutIcon(const QIcon& icon);
    void setConfigIcon(const QIcon& icon);

Q_SIGNALS:

    /**
     * This signal is emitted if the user clicks on a "about"-button of an item in the view
     * passed to the constructor.
     */
    void aboutPluginClicked(const QModelIndex& index);

    /**
     * This signal is emitted if the user clicks on a "configure"-button of an item in the view
     * passed to the constructor.
     */
    void configPluginClicked(const QModelIndex& index);

protected:

    bool editorEvent(QEvent* event,
                     QAbstractItemModel* model,
                     const QStyleOptionViewItem& option,
                     const QModelIndex& index) override;

private:

    enum ButtonType
    {
        About,
        Configure
    };

private:

    static QStyleOptionButton checkboxOption(const QStyleOptionViewItem& option,
                                             const QModelIndex& index,
                                             int position = 0,
                                             Qt::AlignmentFlag alignment = Qt::AlignLeft);
    QStyleOptionButton buttonOption(const QStyleOptionViewItem& option,
                                    const QModelIndex& index,
                                    PluginItemDelegate::ButtonType type,
                                    int position = 0,
                                    Qt::AlignmentFlag alignment = Qt::AlignLeft) const;
    static QSize nameSize(const QModelIndex& index);

    static QRect alignRect(const QRect& object, const QRect& frame, int position, Qt::AlignmentFlag alignment);

private:

    QModelIndex m_configPressedIndex;
    QModelIndex m_aboutPressedIndex;

    QIcon       m_aboutIcon;
    QIcon       m_configIcon;
};

} // namespace Marble
