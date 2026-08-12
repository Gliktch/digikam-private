/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-12
 * Description : a tool to export images to IPFS web service
 *
 * SPDX-FileCopyrightText: 2018      by Amar Lakshya <amar dot lakshya at xaviers dot edu dot in>
 * SPDX-FileCopyrightText: 2018-2020 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "ditemslist.h"
#include "ipfstalker.h"

using namespace Digikam;

namespace DigikamGenericIpfsPlugin
{

class IpfsImagesListViewItem;

class IpfsImagesList : public DItemsList
{
    Q_OBJECT

public:

    /* The different columns in a list.
     */
    enum FieldType
    {
        Title       = DItemsListView::User1,
        Description = DItemsListView::User2,
        Url         = DItemsListView::User3,
    };

public:

    explicit IpfsImagesList(QWidget* const parent = nullptr);
    ~IpfsImagesList()                                            override = default;

    QList<const IpfsImagesListViewItem*> getPendingItems() const;

public Q_SLOTS:

    void slotAddImages(const QList<QUrl>& list) override;
    void slotSuccess(const IpfsTalkerResult& result);
    void slotDoubleClick(QTreeWidgetItem* element, int i);
};

// -------------------------------------------------------------------------

class IpfsImagesListViewItem : public DItemsListViewItem
{
public:

    explicit IpfsImagesListViewItem(DItemsListView* const view, const QUrl& url);
    ~IpfsImagesListViewItem()         override = default;

    void setTitle(const QString& str);
    QString Title()             const;

    void setDescription(const QString& str);
    QString Description()       const;

    void setIpfsUrl(const QString& str);
    QString IpfsUrl()           const;
};

} // namespace DigikamGenericIpfsPlugin
