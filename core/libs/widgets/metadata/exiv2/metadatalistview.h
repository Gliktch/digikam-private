/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-21
 * Description : a generic list view widget to
 *               display metadata
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QString>
#include <QMap>
#include <QResizeEvent>
#include <QTreeWidget>

// Local includes

#include "searchtextbar.h"
#include "metadatawidget.h"
#include "digikam_export.h"

namespace Digikam
{

class MdKeyListViewItem;

class DIGIKAM_EXPORT MetadataListView : public QTreeWidget
{
    Q_OBJECT

public:

    explicit MetadataListView(QWidget* const parent);
    ~MetadataListView() override = default;

    QString getCurrentItemKey() const;
    void    setCurrentItemByKey(const QString& itemKey);

    void    setIfdList(const DMetadata::MetaDataMap& ifds, const QStringList& tagsFilter);
    void    setIfdList(const DMetadata::MetaDataMap& ifds, const QStringList& keysFilter,
                       const QStringList& tagsFilter);

Q_SIGNALS:

    void signalTextFilterMatch(bool);

public Q_SLOTS:

    void slotSearchTextChanged(const SearchTextSettings&);

private Q_SLOTS:

    void slotSelectionChanged(QTreeWidgetItem*, int);

private:

    MdKeyListViewItem* findMdKeyItem(const QString& key);
    void cleanUpMdKeyItem();

private:

    QString         m_selectedItemKey;

    MetadataWidget* m_parent = nullptr;
};

} // namespace Digikam
