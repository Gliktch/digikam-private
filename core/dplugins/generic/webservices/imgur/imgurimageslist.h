/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-04
 * Description : a tool to export images to Imgur web service
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marius Orcsik <marius at habarnam dot ro>
 * SPDX-FileCopyrightText: 2013-2020 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>

// Local includes

#include "ditemslist.h"
#include "imgurtalker.h"

using namespace Digikam;

namespace DigikamGenericImgUrPlugin
{

class ImgurImageListViewItem;

class ImgurImagesList : public DItemsList
{
    Q_OBJECT

public:

    /// The different columns in a list.
    enum FieldType
    {
        Title           = DItemsListView::User1,
        Description     = DItemsListView::User2,
        URL             = DItemsListView::User3,
        DeleteURL       = DItemsListView::User4
    };

public:

    explicit ImgurImagesList(QWidget* const parent = nullptr);
    ~ImgurImagesList() override = default;

    QList<const ImgurImageListViewItem*> getPendingItems();

public Q_SLOTS:

    void slotAddImages(const QList<QUrl>& list) override;
    void slotSuccess(const ImgurTalkerResult& result);
    void slotDoubleClick(QTreeWidgetItem* element, int i);
    void slotContextMenuRequested();
    void slotCopyImurgURL();
};

// -------------------------------------------------------------------------

class ImgurImageListViewItem : public DItemsListViewItem
{
public:

    explicit ImgurImageListViewItem(DItemsListView* const view, const QUrl& url);
    ~ImgurImageListViewItem() override = default;

    void setTitle(const QString& str);
    QString Title()          const;

    void setDescription(const QString& str);
    QString Description()    const;

    void setImgurUrl(const QString& str);
    QString ImgurUrl()       const;

    void setImgurDeleteUrl(const QString& str);
    QString ImgurDeleteUrl() const;
};

} // namespace DigikamGenericImgUrPlugin
