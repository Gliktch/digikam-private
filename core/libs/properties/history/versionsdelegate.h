/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-15
 * Description : Item delegate for image versions list view
 *
 * SPDX-FileCopyrightText: 2010-2011 by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QStyledItemDelegate>

// Local includes

#include "digikam_export.h"
#include "itemdelegateoverlay.h"

namespace Digikam
{

class VersionsDelegate : public QStyledItemDelegate,
                         public ItemDelegateOverlayContainer
{
    Q_OBJECT
    Q_PROPERTY(int animationState READ animationState
                                  WRITE setAnimationState
                                  NOTIFY animationStateChanged)

public:

    explicit VersionsDelegate(QObject* const parent);
    ~VersionsDelegate()                                                                               override;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index)                const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void beginPainting();
    void finishPainting();

    int animationState()                                                                        const;
    void setAnimationState(int animationState);

    void setThumbnailSize(int size)                                                             const;
    int  thumbnailSize()                                                                        const;

Q_SIGNALS:

    void animationStateChanged();
    void visualChange(); // for ItemDelegateOverlayContainer

    /// @note for ItemDelegateOverlayContainer, unimplemented:
    void requestNotification(const QModelIndex& index, const QString& message);
    void hideNotification();

protected Q_SLOTS:

    void overlayDestroyed(QObject* o)                                                                 override
    {
        ItemDelegateOverlayContainer::overlayDestroyed(o);
    }

protected:

    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index)                const override;

    /// Returns the delegate, typically, the derived class
    QAbstractItemDelegate* asDelegate() override
    {
        return this;
    }

private:

    VersionsDelegate(); /// @note disabled default constructor.

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
