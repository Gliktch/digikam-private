/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : Qt item view mouse hover button
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractButton>
#include <QAbstractItemView>

// Local includes

#include "digikam_export.h"

class QTimeLine;

namespace Digikam
{

class DIGIKAM_EXPORT ItemViewHoverButton : public QAbstractButton
{
    Q_OBJECT

public:

    explicit ItemViewHoverButton(QAbstractItemView* const parentView);

    void initIcon();
    void reset();
    void setIndex(const QModelIndex& index);
    QModelIndex index() const;
    void setVisible(bool visible) override;

    /**
     * @brief Reimplement to match the size of your icon
     */
    QSize sizeHint() const override = 0;

protected:

    /**
     * @brief to call in children class constructors to init signal/slot connections.
     */
    void setup();

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    void enterEvent(QEnterEvent* event);

#else

    void enterEvent(QEvent* event);

#endif

    void leaveEvent(QEvent* event);
    void paintEvent(QPaintEvent* event);

    /**
     * @return your icon here. Will be queried again on toggle.
     */
    virtual QIcon icon() = 0;

    /**
     * @brief Optionally update tooltip here. Will be called again on state change.
     */
    virtual void updateToolTip();

protected Q_SLOTS:

    void setFadingValue(int value);
    void refreshIcon();
    void startFading();
    void stopFading();

protected:

    QPersistentModelIndex m_index;
    bool                  m_isHovered       = false;
    int                   m_fadingValue     = 0;
    QIcon                 m_icon;
    QTimeLine*            m_fadingTimeLine  = nullptr;
};

} // namespace Digikam
