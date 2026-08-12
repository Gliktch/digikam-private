/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-08-26
 * Description : file list view and items
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2022      by Quoc Hung Tran <quochungtran1999 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Local includes

#include "ditemslist.h"

using namespace Digikam;

namespace DigikamGenericTextConverterPlugin
{

class TextConverterListViewItem : public DItemsListViewItem
{

public:

    explicit TextConverterListViewItem(DItemsListView* const view, const QUrl& url);
    ~TextConverterListViewItem()         override;

    void setDestFileName(const QString& str);
    QString destFileName()         const;

    void setRecognizedWords(const QString& str);
    QString recognizedWords()      const;

    void setStatus(const QString& str);

private:

    class Private;
    Private* const d = nullptr;
};

// -------------------------------------------------------------------------

class TextConverterList : public DItemsList
{
    Q_OBJECT

public:

    /**
     * List the different columns in a list.
     */
    enum FieldType
    {
        RECOGNIZEDWORDS = DItemsListView::User1,
        TARGETFILENAME  = DItemsListView::User2,
        STATUS          = DItemsListView::User3,
    };

public:

    explicit TextConverterList(QWidget* const parent = nullptr);
    ~TextConverterList()                           override = default;

public Q_SLOTS:

    void slotAddImages(const QList<QUrl>& list)    override;

protected Q_SLOTS:

    void slotRemoveItems()                         override;
};

} // namespace DigikamGenericTextConverterPlugin
