/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : User interface for searches
 *
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractListModel>
#include <QItemDelegate>
#include <QLabel>
#include <QComboBox>

// Local includes

#include "ratingwidget.h"
#include "comboboxutilities.h"

class QTreeView;
class QVBoxLayout;

namespace Digikam
{

class RatingComboBoxModel;
class RatingComboBoxWidget;

class RatingComboBox : public ModelIndexBasedComboBox
{
    Q_OBJECT

public:

    /**
     * An advanced widget for entering a rating,
     * including support for Null and NoRating values
     */
    enum RatingValue
    {
        /**
         * The rating value. All values except Null correspond to
         * the integers used by the database.
         */
        Null     = -2,
        NoRating = -1,
        Rating0  = 0,
        Rating1  = 1,
        Rating2  = 2,
        Rating3  = 3,
        Rating4  = 4,
        Rating5  = 5
    };

public:

    explicit RatingComboBox(QWidget* const parent = nullptr);

    void setRatingValue(RatingValue value);
    RatingValue ratingValue()                                                                   const;

Q_SIGNALS:

    void ratingValueChanged(int value);

protected Q_SLOTS:

    void currentValueChanged(const QModelIndex& current, const QModelIndex& previous);
    void ratingWidgetChanged(int);

protected:

    RatingComboBoxModel*  m_model        = nullptr;
    RatingComboBoxWidget* m_ratingWidget = nullptr;

private:

    bool                  m_syncing      = false;
};

// -- Internal classes ----------------------------------------------------------------

class RatingStarDrawer
{
public:

    RatingStarDrawer() = default;

public:

    QRect drawStarPolygons(QPainter* p, int numberOfStars)                                      const;

protected:

    QPolygon m_starPolygon     = RatingWidget::starPolygon();
    QSize    m_starPolygonSize = QSize(15, 15);
};

// -------------------------------------------------------------------------

class RatingComboBoxWidget : public RatingWidget,
                             public RatingStarDrawer
{
    Q_OBJECT

public:

    /**
     * Internal sub-classing the classic RatingWidget,
     * this provides support for the Null and NoRating states.
     */
    explicit RatingComboBoxWidget(QWidget* const parent = nullptr);

public:

    RatingComboBox::RatingValue ratingValue()                                                   const;
    void setRatingValue(RatingComboBox::RatingValue value);

Q_SIGNALS:

    void ratingValueChanged(int value);

protected Q_SLOTS:

    void slotRatingChanged(int);

protected:

    void paintEvent(QPaintEvent*)                                                                     override;

protected:

    RatingComboBox::RatingValue m_value = RatingComboBox::Null;
};

// -------------------------------------------------------------------------

class RatingComboBoxModel : public QAbstractListModel
{
    Q_OBJECT

public:

    enum CustomRoles
    {
        RatingRole = Qt::UserRole
    };

public:

    explicit RatingComboBoxModel(QObject* const parent = nullptr);

    QModelIndex indexForRatingValue(RatingComboBox::RatingValue value)                          const;

    int rowCount(const QModelIndex& parent)                                                     const override;
    QVariant data(const QModelIndex& index, int role)                                           const override;
    QModelIndex index(int row, int column = 0, const QModelIndex& parent = QModelIndex())       const override;

protected:

    QVariant ratingValueToDisplay(RatingComboBox::RatingValue value)                            const;

protected:

    QList<RatingComboBox::RatingValue> m_entries;
};

// -------------------------------------------------------------------------

class RatingComboBoxDelegate : public QItemDelegate,
                               public RatingStarDrawer
{
    Q_OBJECT

public:

    explicit RatingComboBoxDelegate(QObject* const parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index)                const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

protected:

    void drawRating(QPainter* painter, const QRect& rect, int rating, bool selectable)          const;
};

} // namespace Digikam
