/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : line edit for addition of tags on mouse hover
 *
 * SPDX-FileCopyrightText: 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractItemView>

// Local includes

#include "itemdelegateoverlay.h"
#include "itemviewdelegate.h"

namespace Digikam
{

class AddTagsLineEdit;

class TagsLineEditOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewDelegate)

public:

    explicit TagsLineEditOverlay(QObject* const parent);

    AddTagsLineEdit* addTagsLineEdit() const;

Q_SIGNALS:

    void signalTagEditedById(const QModelIndex& index, int id);
    void signalTagEditedByKeyword(const QModelIndex& index, const QString& keyword);

protected Q_SLOTS:

    void slotTagChanged(int);
    void slotTagChanged(const QString&);
    void slotDataChanged(const QModelIndex&, const QModelIndex&);

protected:

    QWidget* createWidget()                    override;
    void setActive(bool)                       override;
    void visualChange()                        override;
    void slotEntered(const QModelIndex& index) override;
    void hide()                                override;

    void updatePosition();
    void updateTag();

protected:

    QPersistentModelIndex m_index;
};

} // namespace Digikam
