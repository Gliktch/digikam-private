/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-21
 * Description : widget to display a list of items
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2010 by Andi Clemens <andi dot clemens at googlemail dot com>
 * SPDX-FileCopyrightText: 2009-2010 by Luka Renko <lure at kubuntu dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ditemslist_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN DItemsList::Private
{
public:

    Private() = default;

public:

    bool                        allowRAW                = true;
    bool                        allowDuplicate          = false;
    bool                        controlButtonsEnabled   = true;
    int                         iconSize                = 48;

    CtrlButton*                 addButton               = nullptr;
    CtrlButton*                 removeButton            = nullptr;
    CtrlButton*                 moveUpButton            = nullptr;
    CtrlButton*                 moveDownButton          = nullptr;
    CtrlButton*                 clearButton             = nullptr;
    CtrlButton*                 loadButton              = nullptr;
    CtrlButton*                 saveButton              = nullptr;
    QWidget*                    extraWidget             = nullptr;   ///< Extra widget append to the end of control buttons layout.

    QList<QUrl>                 processItems;
    DWorkingPixmap*             progressPix             = nullptr;
    int                         progressCount           = 0;
    QTimer*                     progressTimer           = nullptr;

    DItemsListView*             listView                = nullptr;
    ThumbnailLoadThread*        thumbLoadThread         = ThumbnailLoadThread::defaultThread();

    DInfoInterface*             iface                   = nullptr;
    DItemsListIsLessThanHandler isLessThan;
};

DItemsList::DItemsList(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    d->progressPix    = new DWorkingPixmap(this);
    d->listView       = new DItemsListView(this);
    d->listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // --------------------------------------------------------

    d->addButton      = new CtrlButton(QIcon::fromTheme(QLatin1String("list-add")).pixmap(16, 16),      this);
    d->removeButton   = new CtrlButton(QIcon::fromTheme(QLatin1String("list-remove")).pixmap(16, 16),   this);
    d->moveUpButton   = new CtrlButton(QIcon::fromTheme(QLatin1String("go-up")).pixmap(16, 16),         this);
    d->moveDownButton = new CtrlButton(QIcon::fromTheme(QLatin1String("go-down")).pixmap(16, 16),       this);
    d->clearButton    = new CtrlButton(QIcon::fromTheme(QLatin1String("edit-clear")).pixmap(16, 16),    this);
    d->loadButton     = new CtrlButton(QIcon::fromTheme(QLatin1String("document-open")).pixmap(16, 16), this);
    d->saveButton     = new CtrlButton(QIcon::fromTheme(QLatin1String("document-save")).pixmap(16, 16), this);

    d->addButton->setToolTip(i18nc("@info", "Add new images to the list"));
    d->removeButton->setToolTip(i18nc("@info", "Remove selected images from the list"));
    d->moveUpButton->setToolTip(i18nc("@info", "Move current selected image up in the list"));
    d->moveDownButton->setToolTip(i18nc("@info", "Move current selected image down in the list"));
    d->clearButton->setToolTip(i18nc("@info", "Clear the list."));
    d->loadButton->setToolTip(i18nc("@info", "Load a saved list."));
    d->saveButton->setToolTip(i18nc("@info", "Save the list."));

    d->progressTimer  = new QTimer(this);

    // --------------------------------------------------------

    setIconSize(d->iconSize);
    setControlButtons(Add | Remove | MoveUp | MoveDown | Clear | Save | Load ); // add all buttons      (default)
    setControlButtonsPlacement(ControlButtonsBelow);                            // buttons on the bottom (default)
    enableDragAndDrop(true);                                                    // enable drag and drop (default)

    // --------------------------------------------------------

    connect(d->listView, &DItemsListView::signalAddedDropedItems,
            this, &DItemsList::slotAddImages);

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnail(LoadingDescription,QPixmap)));

    connect(d->listView, &DItemsListView::signalItemClicked,
            this, &DItemsList::signalItemClicked);

    connect(d->listView, &DItemsListView::signalContextMenuRequested,
            this, &DItemsList::signalContextMenuRequested);

    // queue this connection because itemSelectionChanged is emitted
    // while items are deleted, and accessing selectedItems at that
    // time causes a crash ...

    connect(d->listView, &DItemsListView::itemSelectionChanged,
            this, &DItemsList::slotImageListChanged, Qt::QueuedConnection);

    connect(this, &DItemsList::signalImageListChanged,
            this, &DItemsList::slotImageListChanged);

    // --------------------------------------------------------

    connect(d->addButton, &CtrlButton::clicked,
            this, &DItemsList::slotAddItems);

    connect(d->removeButton, &CtrlButton::clicked,
            this, &DItemsList::slotRemoveItems);

    connect(d->moveUpButton, &CtrlButton::clicked,
            this, &DItemsList::slotMoveUpItems);

    connect(d->moveDownButton, &CtrlButton::clicked,
            this, &DItemsList::slotMoveDownItems);

    connect(d->clearButton, &CtrlButton::clicked,
            this, &DItemsList::slotClearItems);

    connect(d->loadButton, &CtrlButton::clicked,
            this, &DItemsList::slotLoadItems);

    connect(d->saveButton, &CtrlButton::clicked,
            this, &DItemsList::slotSaveItems);

    connect(d->progressTimer, &QTimer::timeout,
            this, &DItemsList::slotProgressTimerDone);

    // --------------------------------------------------------

    QTimer::singleShot(1000, this, SIGNAL(signalImageListChanged()));
}

DItemsList::~DItemsList()
{
    delete d;
}

void DItemsList::enableControlButtons(bool enable)
{
    d->controlButtonsEnabled = enable;
    slotImageListChanged();
}

void DItemsList::enableDragAndDrop(const bool enable)
{
    d->listView->enableDragAndDrop(enable);
}

void DItemsList::appendControlButtonsWidget(QWidget* const widget)
{
    d->extraWidget = widget;
}

QBoxLayout* DItemsList::setControlButtonsPlacement(ControlButtonPlacement placement)
{
    delete layout();

    QBoxLayout* lay               = nullptr;        // Layout instance to return;
    const int spacing             = layoutSpacing();


    QGridLayout* const mainLayout = new QGridLayout;
    mainLayout->addWidget(d->listView, 1, 1, 1, 1);
    mainLayout->setRowStretch(1, 10);
    mainLayout->setColumnStretch(1, 10);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);

    // --------------------------------------------------------

    QHBoxLayout* const hBtnLayout = new QHBoxLayout;
    hBtnLayout->addWidget(d->moveUpButton);
    hBtnLayout->addWidget(d->moveDownButton);
    hBtnLayout->addWidget(d->addButton);
    hBtnLayout->addWidget(d->removeButton);
    hBtnLayout->addWidget(d->loadButton);
    hBtnLayout->addWidget(d->saveButton);
    hBtnLayout->addWidget(d->clearButton);
    hBtnLayout->addStretch(1);

    if (d->extraWidget)
    {
        hBtnLayout->addWidget(d->extraWidget);
    }

    // --------------------------------------------------------

    QVBoxLayout* const vBtnLayout = new QVBoxLayout;
    vBtnLayout->addWidget(d->moveUpButton);
    vBtnLayout->addWidget(d->moveDownButton);
    vBtnLayout->addWidget(d->addButton);
    vBtnLayout->addWidget(d->removeButton);
    vBtnLayout->addWidget(d->loadButton);
    vBtnLayout->addWidget(d->saveButton);
    vBtnLayout->addWidget(d->clearButton);
    vBtnLayout->addStretch(1);

    if (d->extraWidget)
    {
        vBtnLayout->addWidget(d->extraWidget);
    }

    // --------------------------------------------------------

    switch (placement)
    {
        case ControlButtonsAbove:
        {
            lay = hBtnLayout;
            mainLayout->addLayout(hBtnLayout, 0, 1, 1, 1);
            delete vBtnLayout;
            break;
        }

        case ControlButtonsBelow:
        {
            lay = hBtnLayout;
            mainLayout->addLayout(hBtnLayout, 2, 1, 1, 1);
            delete vBtnLayout;
            break;
        }

        case ControlButtonsLeft:
        {
            lay = vBtnLayout;
            mainLayout->addLayout(vBtnLayout, 1, 0, 1, 1);
            delete hBtnLayout;
            break;
        }

        case ControlButtonsRight:
        {
            lay = vBtnLayout;
            mainLayout->addLayout(vBtnLayout, 1, 2, 1, 1);
            delete hBtnLayout;
            break;
        }

        case NoControlButtons:
        default:
        {
            delete vBtnLayout;
            delete hBtnLayout;

            // set all buttons invisible

            setControlButtons(ControlButtons());

            if (d->extraWidget)
            {
                d->extraWidget->setVisible(false);
            }

            break;
        }
    }

    setLayout(mainLayout);

    return lay;
}

void DItemsList::setControlButtons(ControlButtons buttonMask)
{
    d->addButton->setVisible(buttonMask & Add);
    d->removeButton->setVisible(buttonMask & Remove);
    d->moveUpButton->setVisible(buttonMask & MoveUp);
    d->moveDownButton->setVisible(buttonMask & MoveDown);
    d->clearButton->setVisible(buttonMask & Clear);
    d->loadButton->setVisible(buttonMask & Load);
    d->saveButton->setVisible(buttonMask & Save);
}

void DItemsList::setIface(DInfoInterface* const iface)
{
    d->iface = iface;
}

DInfoInterface* DItemsList::iface() const
{
    return d->iface;
}

void DItemsList::setAllowDuplicate(bool allow)
{
  d->allowDuplicate = allow;
}

void DItemsList::setAllowRAW(bool allow)
{
    d->allowRAW = allow;
}

void DItemsList::setIconSize(int size)
{
    if      (size < 16)
    {
        d->iconSize = 16;
    }
    else if (size > 128)
    {
        d->iconSize = 128;
    }
    else
    {
        d->iconSize = size;
    }

    d->listView->setIconSize(QSize(iconSize(), iconSize()));
}

int DItemsList::iconSize() const
{
    return d->iconSize;
}

void DItemsList::loadImagesFromCurrentSelection()
{
    bool selection = checkSelection();

    if (selection && d->iface)
    {
        QList<QUrl> images = d->iface->currentSelectedItems();

        if (!images.isEmpty())
        {
            slotAddImages(images);
        }
    }
    else
    {
        loadImagesFromCurrentAlbum();
    }
}

void DItemsList::loadImagesFromCurrentAlbum()
{
    if (!d->iface)
    {
        return;
    }

    QList<QUrl> images = d->iface->currentAlbumItems();

    if (!images.isEmpty())
    {
        slotAddImages(images);
    }
}

bool DItemsList::checkSelection()
{
    if (!d->iface)
    {
        return false;
    }

    QList<QUrl> images = d->iface->currentSelectedItems();

    return (!images.isEmpty());
}

void DItemsList::slotAddImages(const QList<QUrl>& list)
{
    if (list.count() == 0)
    {
        return;
    }

    QList<QUrl> urls;
    bool raw = false;

    for (QList<QUrl>::ConstIterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        QUrl imageUrl = *it;

        // Check if the new item already exist in the list.

        bool found    = false;

        QTreeWidgetItemIterator iter(d->listView);

        while (*iter)
        {
            const DItemsListViewItem* const item = dynamic_cast<DItemsListViewItem*>(*iter);

            if (item && (item->url() == imageUrl))
            {
                found = true;
            }

            ++iter;
        }

        if (d->allowDuplicate || !found)
        {
            // if RAW files are not allowed, skip the image

            if (!d->allowRAW && DRawDecoder::isRawFile(imageUrl))
            {
                raw = true;
                continue;
            }

            new DItemsListViewItem(listView(), imageUrl);
            urls.append(imageUrl);
        }
    }

    Q_EMIT signalAddItems(urls);
    Q_EMIT signalImageListChanged();
    Q_EMIT signalFoundRAWImages(raw);
}

void DItemsList::slotAddItems()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group(objectName());
    QUrl lastFileUrl        = QUrl::fromLocalFile(grp.readEntry("Last Image Path",
                                                  QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)));

    QList<QUrl> urls        = ImageDialog::getImageURLs(this, lastFileUrl);

    if (!urls.isEmpty())
    {
        slotAddImages(urls);
        grp.writeEntry("Last Image Path", urls.first().adjusted(QUrl::RemoveFilename).toLocalFile());
        config->sync();
    }
}

void DItemsList::slotRemoveItems()
{
    QList<QTreeWidgetItem*> selectedItemsList = d->listView->selectedItems();
    QList<int> itemsIndex;

    for (QList<QTreeWidgetItem*>::const_iterator it = selectedItemsList.constBegin() ;
         it != selectedItemsList.constEnd() ; ++it)
    {
        DItemsListViewItem* const item = dynamic_cast<DItemsListViewItem*>(*it);

        if (item)
        {
            itemsIndex.append(d->listView->indexFromItem(item).row());

            if (d->processItems.contains(item->url()))
            {
                d->processItems.removeAll(item->url());
            }

            d->listView->removeItemWidget(*it, 0);
            delete *it;
        }
    }

    Q_EMIT signalRemovedItems(itemsIndex);
    Q_EMIT signalImageListChanged();
}

void DItemsList::slotMoveUpItems()
{
    // move above item down, then we don't have to fix the focus

    QModelIndex curIndex   = listView()->currentIndex();

    if (!curIndex.isValid())
    {
        return;
    }

    QModelIndex aboveIndex = listView()->indexAbove(curIndex);

    if (!aboveIndex.isValid())
    {
        return;
    }

    QTreeWidgetItem* const temp  = listView()->takeTopLevelItem(aboveIndex.row());
    listView()->insertTopLevelItem(curIndex.row(), temp);

    // this is a quick fix. We loose the extra tags in flickr upload, but at list we don't get a crash

    DItemsListViewItem* const uw = dynamic_cast<DItemsListViewItem*>(temp);

    if (uw)
    {
        uw->updateItemWidgets();
    }

    Q_EMIT signalImageListChanged();
    Q_EMIT signalMoveUpItem();
}

void DItemsList::slotMoveDownItems()
{
    // move below item up, then we don't have to fix the focus

    QModelIndex curIndex   = listView()->currentIndex();

    if (!curIndex.isValid())
    {
        return;
    }

    QModelIndex belowIndex = listView()->indexBelow(curIndex);

    if (!belowIndex.isValid())
    {
        return;
    }

    QTreeWidgetItem* const temp  = listView()->takeTopLevelItem(belowIndex.row());
    listView()->insertTopLevelItem(curIndex.row(), temp);

    // This is a quick fix. We can loose extra tags in uploader, but at least we don't get a crash

    DItemsListViewItem* const uw = dynamic_cast<DItemsListViewItem*>(temp);

    if (uw)
    {
        uw->updateItemWidgets();
    }

    Q_EMIT signalImageListChanged();
    Q_EMIT signalMoveDownItem();
}

void DItemsList::slotClearItems()
{
    listView()->selectAll();
    slotRemoveItems();
    listView()->clear();
}

void DItemsList::slotLoadItems()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group(objectName());
    QUrl lastFileUrl        = QUrl::fromLocalFile(grp.readEntry("Last Images List Path",
                                                  QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
    QUrl loadLevelsFile;
    loadLevelsFile          = DFileDialog::getOpenFileUrl(this,
                                                          i18nc("@title:window", "Select the Image File List to Load"), lastFileUrl,
                                                          i18nc("@option", "All Files (*)"));

    if (loadLevelsFile.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "empty url";
        return;
    }

    QFile file(loadLevelsFile.toLocalFile());

    qCDebug(DIGIKAM_GENERAL_LOG) << "file path " << loadLevelsFile.toLocalFile();

    if (!file.open(QIODevice::ReadOnly))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot open file";
        return;
    }

    QXmlStreamReader xmlReader;
    xmlReader.setDevice(&file);

    while (!xmlReader.atEnd())
    {
        if      (xmlReader.isStartElement() && (xmlReader.name() == QLatin1String("Image")))
        {
            // get all attributes and its value of a tag in attrs variable.

            QXmlStreamAttributes attrs = xmlReader.attributes();

            // get value of each attribute from QXmlStreamAttributes

            QStringView url             = attrs.value(QLatin1String("url"));

            if (url.isEmpty())
            {
                xmlReader.readNext();
                continue;
            }

            QList<QUrl> urls;
            urls.append(QUrl(url.toString()));

            //allow tools to append a new file

            slotAddImages(urls);

            // read tool Image custom attributes and children element

            Q_EMIT signalXMLLoadImageElement(xmlReader);
        }
        else if (xmlReader.isStartElement() && (xmlReader.name() != QLatin1String("Images")))
        {
            // unmanaged start element (it should be tools one)

            Q_EMIT signalXMLCustomElementsToRead(xmlReader);
        }
        else if (xmlReader.isEndElement() && (xmlReader.name() == QLatin1String("Images")))
        {
            // if EndElement is Images return

            grp.writeEntry("Last Images List Path", loadLevelsFile.adjusted(QUrl::RemoveFilename).toLocalFile());
            config->sync();
            file.close();
            return;
        }

        xmlReader.readNext();
    }

    file.close();
}

void DItemsList::slotSaveItems()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup grp        = config->group(objectName());
    QUrl lastFileUrl        = QUrl::fromLocalFile(grp.readEntry("Last Images List Path",
                                                  QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
    QUrl saveLevelsFile;
    saveLevelsFile          = DFileDialog::getSaveFileUrl(this,
                                                          i18nc("@title:window", "Select the Image File List to Save"),
                                                          lastFileUrl,
                                                          i18nc("@option", "All Files (*)"));

    qCDebug(DIGIKAM_GENERAL_LOG) << "file url " << saveLevelsFile.toDisplayString();

    if (saveLevelsFile.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "empty url";
        return;
    }

    QFile file(saveLevelsFile.toLocalFile());

    if (!file.open(QIODevice::WriteOnly))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot open target file";
        return;
    }

    QXmlStreamWriter xmlWriter;
    xmlWriter.setDevice(&file);

    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();

    xmlWriter.writeStartElement(QLatin1String("Images"));

    QTreeWidgetItemIterator it(listView());

    while (*it)
    {
        DItemsListViewItem* const lvItem = dynamic_cast<DItemsListViewItem*>(*it);

        if (lvItem)
        {
            xmlWriter.writeStartElement(QLatin1String("Image"));

            xmlWriter.writeAttribute(QLatin1String("url"), lvItem->url().toDisplayString());

            Q_EMIT signalXMLSaveItem(xmlWriter, listView()->indexFromItem(lvItem).row());

            xmlWriter.writeEndElement(); // Image
        }

        ++it;
    }

    Q_EMIT signalXMLCustomElementsToWrite(xmlWriter);

    xmlWriter.writeEndElement();  // Images

    xmlWriter.writeEndDocument(); // end document

    grp.writeEntry("Last Images List Path", saveLevelsFile.adjusted(QUrl::RemoveFilename).toLocalFile());
    config->sync();
    file.close();
}

void DItemsList::removeItemByUrl(const QUrl& url)
{
    bool found;
    QList<int> itemsIndex;

    do
    {
        found = false;
        QTreeWidgetItemIterator it(d->listView);

        while (*it)
        {
            DItemsListViewItem* const item = dynamic_cast<DItemsListViewItem*>(*it);

            if (item && (item->url() == url))
            {
                itemsIndex.append(d->listView->indexFromItem(item).row());

                if (d->processItems.contains(item->url()))
                {
                    d->processItems.removeAll(item->url());
                }

                delete item;
                found = true;
                break;
            }

            ++it;
        }
    }
    while (found);

    Q_EMIT signalRemovedItems(itemsIndex);
    Q_EMIT signalImageListChanged();
}

QList<QUrl> DItemsList::imageUrls(bool onlyUnprocessed) const
{
    QList<QUrl> list;
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        const DItemsListViewItem* const item = dynamic_cast<DItemsListViewItem*>(*it);

        if (item)
        {
            if ((onlyUnprocessed == false) || (item->state() != DItemsListViewItem::Success))
            {
                list.append(item->url());
            }
        }

        ++it;
    }

    return list;
}

void DItemsList::slotProgressTimerDone()
{
    if (!d->processItems.isEmpty())
    {
        for (const QUrl& url : std::as_const(d->processItems))
        {
            DItemsListViewItem* const item = listView()->findItem(url);

            if (item)
            {
                item->setProgressAnimation(d->progressPix->frameAt(d->progressCount));
            }
        }

        d->progressCount++;

        if (d->progressCount == 8)
        {
            d->progressCount = 0;
        }

        d->progressTimer->start(300);
    }
}

void DItemsList::processing(const QUrl& url)
{
    DItemsListViewItem* const item = listView()->findItem(url);

    if (item)
    {
        d->processItems.append(url);
        d->listView->setCurrentItem(item, true);
        d->listView->scrollToItem(item);
        d->progressTimer->start(300);
    }
}

void DItemsList::processed(const QUrl& url, bool success)
{
    DItemsListViewItem* const item = listView()->findItem(url);

    if (item)
    {
        d->processItems.removeAll(url);
        item->setProcessedIcon(QIcon::fromTheme(success ? QLatin1String("dialog-ok-apply")
                                                        : QLatin1String("dialog-cancel")).pixmap(16, 16));
        item->setState(success ? DItemsListViewItem::Success
                               : DItemsListViewItem::Failed);

        if (d->processItems.isEmpty())
        {
            d->progressTimer->stop();
        }
    }
}

void DItemsList::cancelProcess()
{
    for (const QUrl& url : std::as_const(d->processItems))
    {
        processed(url, false);
    }
}

void DItemsList::clearProcessedStatus()
{
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        DItemsListViewItem* const lvItem = dynamic_cast<DItemsListViewItem*>(*it);

        if (lvItem)
        {
            lvItem->setProcessedIcon(QIcon());
        }

        ++it;
    }
}

DItemsListView* DItemsList::listView() const
{
    return d->listView;
}

void DItemsList::slotImageListChanged()
{
    const QList<QTreeWidgetItem*> selectedItemsList = d->listView->selectedItems();
    const bool haveImages                           = !(imageUrls().isEmpty())         && d->controlButtonsEnabled;
    const bool haveSelectedImages                   = !(selectedItemsList.isEmpty())   && d->controlButtonsEnabled;
    const bool haveOnlyOneSelectedImage             = (selectedItemsList.count() == 1) && d->controlButtonsEnabled;

    d->removeButton->setEnabled(haveSelectedImages);
    d->moveUpButton->setEnabled(haveOnlyOneSelectedImage);
    d->moveDownButton->setEnabled(haveOnlyOneSelectedImage);
    d->clearButton->setEnabled(haveImages);

    // All buttons are enabled / disabled now, but the "Add" button should always be
    // enabled, if the buttons are not explicitly disabled with enableControlButtons()

    d->addButton->setEnabled(d->controlButtonsEnabled);

    // TODO: should they be enabled by default now?

    d->loadButton->setEnabled(d->controlButtonsEnabled);
    d->saveButton->setEnabled(d->controlButtonsEnabled);
}

void DItemsList::updateThumbnail(const QUrl& url)
{
    d->thumbLoadThread->find(ThumbnailIdentifier(url.toLocalFile()));
}

void DItemsList::slotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        DItemsListViewItem* const item = dynamic_cast<DItemsListViewItem*>(*it);

        if (item && (item->url() == QUrl::fromLocalFile(desc.filePath)))
        {
            if (!pix.isNull())
            {
                item->setThumb(pix.scaled(d->iconSize, d->iconSize, Qt::KeepAspectRatio));
            }

            if (!d->allowDuplicate)
            {
                return;
            }
        }

        ++it;
    }
}

DItemsListViewItem* DItemsListView::getCurrentItem() const
{
    QTreeWidgetItem* const currentTreeItem = currentItem();

    if (!currentTreeItem)
    {
        return nullptr;
    }

    return dynamic_cast<DItemsListViewItem*>(currentTreeItem);
}

QUrl DItemsList::getCurrentUrl() const
{
    const DItemsListViewItem* const currentItem = d->listView->getCurrentItem();

    if (!currentItem)
    {
        return QUrl();
    }

    return currentItem->url();
}

void DItemsList::setCurrentUrl(const QUrl& url)
{
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        DItemsListViewItem* const item = dynamic_cast<DItemsListViewItem*>(*it);

        if (item && (item->url() == url))
        {
            d->listView->setCurrentItem(item);
            return;
        }

        ++it;
    }
}

void DItemsList::setIsLessThanHandler(DItemsListIsLessThanHandler fncptr)
{
    d->isLessThan = fncptr;
}

DItemsListIsLessThanHandler DItemsList::isLessThanHandler() const
{
    return d->isLessThan;
}

} // namespace Digikam

#include "moc_ditemslist.cpp"

#include "moc_ditemslist_p.cpp"
