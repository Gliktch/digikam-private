/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-24
 * Description : file list view and items.
 *
 * SPDX-FileCopyrightText: 2008-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Veaceslav Munteanu <slavuttici at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dngconverterlist.h"

// Qt includes

#include <QFileInfo>
#include <QDir>

// KDE includes

#include <klocalizedstring.h>

// LibKDcraw includes

#include "drawdecoder.h"

namespace DigikamGenericDNGConverterPlugin
{

DNGConverterList::DNGConverterList(QWidget* const parent)
    : DItemsList(parent)
{
    listView()->setColumnLabel(DItemsListView::Filename, i18n("Raw File"));
    listView()->setColumn(static_cast<DItemsListView::ColumnType>(TARGETFILENAME), i18n("Target File"), true);
    listView()->setColumn(static_cast<DItemsListView::ColumnType>(IDENTIFICATION), i18n("Camera"),      true);
    listView()->setColumn(static_cast<DItemsListView::ColumnType>(STATUS),         i18n("Status"),      true);
}

void DNGConverterList::slotAddImages(const QList<QUrl>& list)
{
    /**
     * Replaces the DItemsList::slotAddImages method, so that
     * DNGConverterListViewItems can be added instead of ImagesListViewItems
     */

    // Figure out which of the supplied Url's should actually be added and which
    // of them already exist.

    bool found = false;

    for (QList<QUrl>::const_iterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        QUrl imageUrl = *it;
        found         = false;

        for (int i = 0 ; i < listView()->topLevelItemCount() ; ++i)
        {
            const DNGConverterListViewItem* const currItem = dynamic_cast<DNGConverterListViewItem*>(listView()->topLevelItem(i));

            if (currItem && (currItem->url() == imageUrl))
            {
                found = true;
                break;
            }
        }

        if (
            !found                                                                       &&
            (DRawDecoder::isRawFile(imageUrl))                                           &&
            (QFileInfo(imageUrl.toLocalFile()).suffix().toUpper() != QLatin1String("DNG"))
           )
        {
            new DNGConverterListViewItem(listView(), imageUrl);
        }
    }

    // Duplicate the signalImageListChanged of the ImageWindow, to enable the upload button again.

    Q_EMIT signalImageListChanged();
}

void DNGConverterList::slotRemoveItems()
{
    bool find = false;

    do
    {
        find = false;
        QTreeWidgetItemIterator it(listView());

        while (*it)
        {
            DNGConverterListViewItem* const item = dynamic_cast<DNGConverterListViewItem*>(*it);

            if (item && item->isSelected())
            {
                delete item;
                find = true;
                break;
            }

            ++it;
        }
    }
    while(find);
}

// ------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN DNGConverterListViewItem::Private
{
public:

    Private() = default;

public:

    QString destFileName;
    QString identity;
    QString status;
};

DNGConverterListViewItem::DNGConverterListViewItem(DItemsListView* const view, const QUrl& url)
    : DItemsListViewItem(view, url),
      d                 (new Private)
{
}

DNGConverterListViewItem::~DNGConverterListViewItem()
{
    delete d;
}

void DNGConverterListViewItem::setDestFileName(const QString& str)
{
    d->destFileName = str;
    setText(DNGConverterList::TARGETFILENAME, d->destFileName);
}

QString DNGConverterListViewItem::destFileName() const
{
    return d->destFileName;
}

void DNGConverterListViewItem::setIdentity(const QString& str)
{
    d->identity = str;
    setText(DNGConverterList::IDENTIFICATION, d->identity);
}

QString DNGConverterListViewItem::identity() const
{
    return d->identity;
}

void DNGConverterListViewItem::setStatus(const QString& str)
{
    d->status = str;
    setText(DNGConverterList::STATUS, d->status);
}

QString DNGConverterListViewItem::destPath() const
{
    return (QDir::fromNativeSeparators(QFileInfo(url().toLocalFile()).path() + QLatin1String("/") + destFileName()));
}

} // namespace DigikamGenericDNGConverterPlugin

#include "moc_dngconverterlist.cpp"
