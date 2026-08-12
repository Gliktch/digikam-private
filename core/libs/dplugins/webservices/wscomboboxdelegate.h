/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-05
 * Description : A combobox delegate to display in Web service image lists.
 *
 * SPDX-FileCopyrightText: 2009 by Pieter Edelman <pieter dot edelman at gmx dot net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractItemDelegate>
#include <QAbstractItemModel>
#include <QMap>
#include <QModelIndex>
#include <QPainter>
#include <QSize>
#include <QString>
#include <QStyleOptionViewItem>
#include <QWidget>

// Local includes

#include "digikam_export.h"
#include "ditemslist.h"

namespace Digikam
{

class DIGIKAM_EXPORT ComboBoxDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:

    explicit ComboBoxDelegate(DItemsList* const, const QMap<int, QString>&);
    ~ComboBoxDelegate() override;

    /**
     * Whenever an element needs to be edited, this method should be called.
     * It's actually a hack to prevent the item text shining through whenever
     * editing occurs.
     */
    void startEditing(QTreeWidgetItem*, int);

    ///@{
    /**
     * Overloaded functions to provide the delegate functionality.
     */
    void     paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&)       const override;
    QSize    sizeHint(const QStyleOptionViewItem&, const QModelIndex&)               const override;
    QWidget* createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const override;
    void     setEditorData(QWidget*, const QModelIndex&)                             const override;
    void     setModelData(QWidget*, QAbstractItemModel*, const QModelIndex&)         const override;
    ///@}

private Q_SLOTS:

    void slotCommitAndCloseEditor(int);
    void slotResetEditedState(QObject*);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
