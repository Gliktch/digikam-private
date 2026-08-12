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

#include "textconverterlist.h"

// KDE includes

#include <klocalizedstring.h>

// LibKDcraw includes

#include "drawdecoder.h"
#include "digikam_debug.h"

using namespace Digikam;

namespace DigikamGenericTextConverterPlugin
{

class Q_DECL_HIDDEN TextConverterListViewItem::Private
{
public:

    Private() = default;

public:

    QString destFileName;
    QString recognizedWords;
    QString identity;
    QString status;
};

TextConverterListViewItem::TextConverterListViewItem(DItemsListView* const view, const QUrl& url)
    : DItemsListViewItem(view, url),
      d                 (new Private)
{
}

TextConverterListViewItem::~TextConverterListViewItem()
{
    delete d;
}

void TextConverterListViewItem::setDestFileName(const QString& str)
{
    d->destFileName = str;
    setText(TextConverterList::TARGETFILENAME, d->destFileName);
}

QString TextConverterListViewItem::destFileName() const
{
    return d->destFileName;
}

void TextConverterListViewItem::setRecognizedWords(const QString& str)
{
    d->recognizedWords = str;
    setText(TextConverterList::RECOGNIZEDWORDS, d->recognizedWords);
}

QString TextConverterListViewItem::recognizedWords() const
{
    return d->recognizedWords;
}

void TextConverterListViewItem::setStatus(const QString& str)
{
    d->status = str;
    setText(TextConverterList::STATUS, d->status);
}

// --------------------------------------------------------------------------------------

TextConverterList::TextConverterList(QWidget* const parent)
    : DItemsList(parent)
{
    listView()->setColumn(static_cast<DItemsListView::ColumnType>(RECOGNIZEDWORDS), i18n("Words"),       true);
    listView()->setColumn(static_cast<DItemsListView::ColumnType>(TARGETFILENAME),  i18n("Target File"), true);
    listView()->setColumn(static_cast<DItemsListView::ColumnType>(STATUS),          i18n("Status"),      true);
}

void TextConverterList::slotAddImages(const QList<QUrl>& list)
{
    if (list.count() == 0)
    {
        return;
    }

    QList<QUrl> urls;
    bool raw = false;

    for (const auto& imageUrl : list)
    {
        // Check if the new item already exist in the list.

        bool found = false;

        for (int i = 0 ; i < listView()->topLevelItemCount() ; ++i)
        {
            const TextConverterListViewItem* const currItem = dynamic_cast<TextConverterListViewItem*>(listView()->topLevelItem(i));

            if (currItem && (currItem->url() == imageUrl))
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            if (DRawDecoder::isRawFile(imageUrl))
            {
                raw = true;
                continue;
            }

            new TextConverterListViewItem(listView(), imageUrl);
            urls.append(imageUrl);
        }
    }

    Q_EMIT signalAddItems(urls);
    Q_EMIT signalFoundRAWImages(raw);
    Q_EMIT signalImageListChanged();
}

void TextConverterList::slotRemoveItems()
{
    bool find = false;

    do
    {
        find = false;
        QTreeWidgetItemIterator it(listView());

        while (*it)
        {
            TextConverterListViewItem* const item = dynamic_cast<TextConverterListViewItem*>(*it);

            if (item && item->isSelected())
            {
                delete item;
                find = true;
                break;
            }

            ++it;
        }
    }
    while (find);
}

} // namespace DigikamGenericTextConverterPlugin

#include "moc_textconverterlist.cpp"
