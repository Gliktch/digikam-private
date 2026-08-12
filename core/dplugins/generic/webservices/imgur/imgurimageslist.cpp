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

#include "imgurimageslist.h"

// C++ includes

#include <memory>

// Qt includes

#include <QLabel>
#include <QPointer>
#include <QDesktopServices>
#include <QApplication>
#include <QClipboard>
#include <QMenu>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmetadata.h"
#include "dinfointerface.h"
#include "digikam_debug.h"

namespace DigikamGenericImgUrPlugin
{

ImgurImagesList::ImgurImagesList(QWidget* const parent)
    : DItemsList(parent)
{
    setControlButtonsPlacement(DItemsList::ControlButtonsBelow);
    setAllowDuplicate(false);
    setAllowRAW(false);

    auto* const list = listView();

    list->setColumnLabel(DItemsListView::Thumbnail, i18n("Thumbnail"));

    list->setColumnLabel(static_cast<DItemsListView::ColumnType>(ImgurImagesList::Title),
                         i18n("Submission title"));

    list->setColumnLabel(static_cast<DItemsListView::ColumnType>(ImgurImagesList::Description),
                         i18n("Submission description"));

    list->setColumn(static_cast<DItemsListView::ColumnType>(ImgurImagesList::URL),
                    i18n("Imgur URL"), true);

    list->setColumn(static_cast<DItemsListView::ColumnType>(ImgurImagesList::DeleteURL),
                    i18n("Imgur Delete URL"), true);

    connect(list, &DItemsListView::itemDoubleClicked,
            this, &ImgurImagesList::slotDoubleClick);

    connect(list, &DItemsListView::signalContextMenuRequested,
            this, &ImgurImagesList::slotContextMenuRequested);
}

QList<const ImgurImageListViewItem*> ImgurImagesList::getPendingItems()
{
    QList<const ImgurImageListViewItem*> ret;

    for (int i = 0 ; i < listView()->topLevelItemCount() ; ++i)
    {
        auto* const item = dynamic_cast<const ImgurImageListViewItem*>(listView()->topLevelItem(i));

        if (item && item->ImgurUrl().isEmpty())
        {
            ret << item;
        }
    }

    return ret;
}

void ImgurImagesList::slotAddImages(const QList<QUrl>& list)
{
    /**
     * Replaces the DItemsList::slotAddImages method, so that
     * ImgurImageListViewItems can be added instead of ImagesListViewItems
     */

    QScopedPointer<DMetadata> meta(new DMetadata);

    for (QList<QUrl>::ConstIterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        // Already in the list?

        if (listView()->findItem(*it) == nullptr)
        {
            // Load URLs from meta data, if possible

            if (meta->load((*it).toLocalFile()))
            {
                auto* const item = new ImgurImageListViewItem(listView(), *it);
                item->setImgurUrl(meta->getXmpTagString("Xmp.digiKam.ImgurId"));
                item->setImgurDeleteUrl(meta->getXmpTagString("Xmp.digiKam.ImgurDeleteHash"));
            }
        }
    }

    Q_EMIT signalImageListChanged();
    Q_EMIT signalAddItems(list);
}

void ImgurImagesList::slotSuccess(const ImgurTalkerResult& result)
{
    QUrl imgurl = QUrl::fromLocalFile(result.action->upload.imgpath);

    processed(imgurl, true);

    QScopedPointer<DMetadata> meta(new DMetadata);

    // Save URLs to meta data, if possible

    if (meta->load(imgurl.toLocalFile()))
    {
        meta->setXmpTagString("Xmp.digiKam.ImgurId",
                             result.image.url);
        meta->setXmpTagString("Xmp.digiKam.ImgurDeleteHash",
                             ImgurTalker::urlForDeletehash(result.image.deletehash).toString());
        meta->setMetadataWritingMode((int)DMetadata::WRITE_TO_FILE_ONLY);
        bool saved = meta->applyChanges();

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Metadata"
                                         << (saved ? "Saved" : "Not Saved")
                                         << "to" << imgurl;
    }

    ImgurImageListViewItem* const currItem = dynamic_cast<ImgurImageListViewItem*>(listView()->findItem(imgurl));

    if (!currItem)
    {
        return;
    }

    if (!result.image.url.isEmpty())
    {
        currItem->setImgurUrl(result.image.url);
    }

    if (!result.image.deletehash.isEmpty())
    {
        currItem->setImgurDeleteUrl(ImgurTalker::urlForDeletehash(result.image.deletehash).toString());
    }
}

void ImgurImagesList::slotDoubleClick(QTreeWidgetItem* element, int i)
{
    if ((i == URL) || (i == DeleteURL))
    {
        const QUrl url = QUrl(element->text(i));

        // The delete page asks for confirmation, so we don't need to do that here

        QDesktopServices::openUrl(url);
    }
}

void ImgurImagesList::slotContextMenuRequested()
{
    if (
        listView()->topLevelItemCount()                                    &&
        (
         (listView()->currentIndex().column() == ImgurImagesList::URL)     ||
         (listView()->currentIndex().column() == ImgurImagesList::DeleteURL)
        )
       )
    {
        listView()->blockSignals(true);

        QMenu menu(listView());

        QAction* const action = menu.addAction(i18n("Copy"));

        connect(action, SIGNAL(triggered()),
                this , SLOT(slotCopyImurgURL()));

        menu.exec(QCursor::pos());

        listView()->blockSignals(false);
    }
}

void ImgurImagesList::slotCopyImurgURL()
{
    QClipboard* const clipboard               = QApplication::clipboard();

    QList<QTreeWidgetItem*> selectedItemsList = listView()->selectedItems();
    QString copyURLtext;

    for (QList<QTreeWidgetItem*>::const_iterator it = selectedItemsList.constBegin() ;
         it != selectedItemsList.constEnd() ; ++it)
    {
        const ImgurImageListViewItem* const item = dynamic_cast<ImgurImageListViewItem*>(*it);

        if (item)
        {
            if (
                (listView()->currentIndex().column() == ImgurImagesList::URL) &&
                !item->ImgurUrl().isEmpty()
               )
            {
                copyURLtext.append(QString::fromLatin1("%1").arg(item->ImgurUrl()));
            }

            if (
                (listView()->currentIndex().column() == ImgurImagesList::DeleteURL) &&
                !item->ImgurDeleteUrl().isEmpty()
               )
            {
               copyURLtext.append(QString::fromLatin1("%1").arg(item->ImgurDeleteUrl()));
            }
        }
    }

    clipboard->setText(copyURLtext);
}

// ------------------------------------------------------------------------------------------------

ImgurImageListViewItem::ImgurImageListViewItem(DItemsListView* const view, const QUrl& url)
    : DItemsListViewItem(view, url)
{
    QBrush blue(QColor(50, 50, 255));

    setForeground(ImgurImagesList::URL,       blue);
    setForeground(ImgurImagesList::DeleteURL, blue);
}

void ImgurImageListViewItem::setTitle(const QString& str)
{
    setText(ImgurImagesList::Title, str);
}

QString ImgurImageListViewItem::Title() const
{
    return text(ImgurImagesList::Title);
}

void ImgurImageListViewItem::setDescription(const QString& str)
{
    setText(ImgurImagesList::Description, str);
}

QString ImgurImageListViewItem::Description() const
{
    return text(ImgurImagesList::Description);
}

void ImgurImageListViewItem::setImgurUrl(const QString& str)
{
    setText(ImgurImagesList::URL, str);
}

QString ImgurImageListViewItem::ImgurUrl() const
{
    return text(ImgurImagesList::URL);
}

void ImgurImageListViewItem::setImgurDeleteUrl(const QString& str)
{
    setText(ImgurImagesList::DeleteURL, str);
}

QString ImgurImageListViewItem::ImgurDeleteUrl() const
{
    return text(ImgurImagesList::DeleteURL);
}

} // namespace DigikamGenericImgUrPlugin

#include "moc_imgurimageslist.cpp"
