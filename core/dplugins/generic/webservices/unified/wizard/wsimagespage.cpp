/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : page visualizing photos user choosing to upload and
 *               user albums list to upload photos to. Creating new album
 *               is also available on this page.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "wsimagespage.h"

// Qt includes

#include <QIcon>
#include <QPixmap>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "ditemslist.h"
#include "wswizard.h"
#include "wsnewalbumdialog.h"

namespace DigikamGenericUnifiedPlugin
{

class Q_DECL_HIDDEN WSImagesPage::Private
{
public:

    explicit Private(QWizard* const dialog)
    {
        wizard = dynamic_cast<WSWizard*>(dialog);

        if (wizard)
        {
            iface  = wizard->iface();
            wsAuth = wizard->wsAuth();
        }
    }

    DItemsList*       imageList         = nullptr;

    QTreeWidget*      albumView         = nullptr;
    QString           currentAlbumId;
    QPushButton*      newAlbumBtn       = nullptr;
    QPushButton*      reloadAlbumsBtn   = nullptr;

    WSWizard*         wizard            = nullptr;
    DInfoInterface*   iface             = nullptr;
    WSAuthentication* wsAuth            = nullptr;
};

WSImagesPage::WSImagesPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d          (new Private(dialog))
{
    DHBox* const hbox       = new DHBox(this);

    /* --------------------
     * Widget for Images list
     */

    DVBox* const vboxImage  = new DVBox(hbox);
    QLabel* const descImage = new QLabel(vboxImage);
    descImage->setText(i18n("<h3>This view lists all items to export.</h3>"));

    d->imageList            = new DItemsList(vboxImage);
    d->imageList->setObjectName(QLatin1String("WebService ImagesList"));
    d->imageList->setControlButtonsPlacement(DItemsList::ControlButtonsBelow);

    connect(d->imageList, SIGNAL(signalImageListChanged()),
            this, SIGNAL(completeChanged()));

    /*
     * User albums list
     */

    DVBox* const vboxAlbum  = new DVBox(hbox);

    QLabel* const descAlbum = new QLabel(vboxAlbum);
    descAlbum->setText(i18n("<h3>This view lists user albums.</h3>"));

    d->albumView            = new QTreeWidget(vboxAlbum);
    d->albumView->setHeaderLabel(QLatin1String(""));

    DHBox* const buttonBox  = new DHBox(vboxAlbum);

    // Disable New Album button for now, because creating album on facebook is encountering a strange error

    d->newAlbumBtn          = new QPushButton(QLatin1String("New Album"), buttonBox);
    //d->newAlbumBtn->setDisabled(true);

    d->reloadAlbumsBtn      = new QPushButton(QLatin1String("Reload"), buttonBox);

    connect(d->newAlbumBtn, SIGNAL(clicked()),
            d->wsAuth, SLOT(slotNewAlbumRequest()));

    connect(d->reloadAlbumsBtn, SIGNAL(clicked()),
            this, SIGNAL(signalListAlbumsRequest()));

    connect(this, SIGNAL(signalListAlbumsRequest()),
            d->wsAuth, SLOT(slotListAlbumsRequest()));

    connect(d->wsAuth, SIGNAL(signalCreateAlbumDone(int,QString,QString)),
            this, SLOT(slotCreateAlbumDone(int,QString,QString)));

    connect(d->wsAuth, SIGNAL(signalListAlbumsDone(QMap<QString,AlbumSimplified>,QStringList,QString)),
            this, SLOT(slotListAlbumsDone(QMap<QString,AlbumSimplified>,QStringList,QString)));

    /*
     * General settings for imagespage
     */

    hbox->setStretchFactor(vboxImage, 2);
    hbox->setStretchFactor(vboxAlbum, 1);

    setPageWidget(hbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("view-list-details")));
}

WSImagesPage::~WSImagesPage()
{
    delete d;
}

void WSImagesPage::setItemsList(const QList<QUrl>& urls)
{
    d->imageList->slotAddImages(urls);
}

void WSImagesPage::setCurrentAlbumId(const QString& currentAlbumId)
{
    d->currentAlbumId = currentAlbumId;
    d->wizard->settings()->currentAlbumId = currentAlbumId;
}

void WSImagesPage::initializePage()
{
    d->imageList->setIface(d->iface);
    d->imageList->listView()->clear();

    // List current albums in user account

    Q_EMIT signalListAlbumsRequest();
}

bool WSImagesPage::validatePage()
{
    /*
     * If user album is not empty, get id for album to upload photos from currentItem on the list.
     * Otherwise, set album id as empty string and uploading to empty album will be handled in
     * specific talker of each web service
     */

    if (d->albumView->currentItem())
    {
        setCurrentAlbumId(d->albumView->currentItem()->data(0, Qt::AccessibleDescriptionRole).toString());
    }
    else
    {
        setCurrentAlbumId(QLatin1String(""));
    }

    if (d->imageList->imageUrls().isEmpty())
    {
        return false;
    }

    d->wizard->settings()->inputImages = d->imageList->imageUrls();

    return true;
}

bool WSImagesPage::isComplete() const
{
    return (!d->imageList->imageUrls().isEmpty());
}

void WSImagesPage::addChildToTreeView(QTreeWidgetItem* const parent,
                                      const QMap<QString, AlbumSimplified>& albumTree,
                                      const QStringList& childrenAlbums)
{
    if (childrenAlbums.isEmpty())
    {
        return;
    }

    for (const QString& albumId : std::as_const(childrenAlbums))
    {
        QTreeWidgetItem* const item = new QTreeWidgetItem(parent);
        item->setText(0, albumTree[albumId].title);
        item->setData(0, Qt::AccessibleDescriptionRole, albumId);

        /*
         * Verify if album is editable. If yes, let it enable on view for albums list and
         * add a description for that. Otherwise, disable it and add description.
         *
         * However, in case of Facebook, GET album may return False for uploadable, but
         * indeed we can still upload to it.
         *
         * Hence, this functionality needs a more particular solution.
         */

        if (albumTree[albumId].uploadable)
        {
            item->setWhatsThis(0, QLatin1String("Albums that can be uploaded."));
        }
        else
        {
            // item->setDisabled(true);
            item->setWhatsThis(0, QLatin1String("Albums that cannot be uploaded."));
        }

        /*
         * Condition to call setCurrentItem for QTreeWidget is tested here to assure that after clicking on Reload, currentItem still points
         * to the same album as before
         */

        if (albumId == d->currentAlbumId)
        {
            d->albumView->setCurrentItem(item);
        }

        addChildToTreeView(item, albumTree, albumTree[albumId].childrenIDs);
    }
}

void WSImagesPage::slotListAlbumsDone(const QMap<QString, AlbumSimplified>& albumTree,
                                      const QStringList& rootAlbums,
                                      const QString& currentAlbumId)
{
    d->albumView->clear();

    if (rootAlbums.isEmpty() || albumTree.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "WARNING: albums list is empty";
        return;
    }

    if (currentAlbumId.isEmpty())
    {
        d->currentAlbumId = rootAlbums.first();
    }
    else
    {
        d->currentAlbumId = currentAlbumId;
    }

    for (const QString& albumId : std::as_const(rootAlbums))
    {
        QTreeWidgetItem* const item = new QTreeWidgetItem(d->albumView);
        item->setText(0, albumTree[albumId].title);
        item->setData(0, Qt::AccessibleDescriptionRole, albumId); // set ID hidden, so that we can use it for uploading photos to album

        /*
         * Verify if album is editable. If yes, let it enable on view for albums list and
         * add a description for that. Otherwise, disable it and add description.
         *
         * However, in case of Facebook, GET album may return False for uploadable, but
         * indeed we can still upload to it.
         *
         * Hence, this functionality needs a more particular solution.
         */

        if (albumTree[albumId].uploadable)
        {
            item->setWhatsThis(0, QLatin1String("Albums that can be uploaded."));
        }
        else
        {
            // item->setDisabled(true);
            item->setWhatsThis(0, QLatin1String("Albums that cannot be uploaded."));
        }

        /*
         * Condition to call setCurrentItem for QTreeWidget is tested here to assure that after clicking on Reload, currentItem still points
         * to the same album as before
         */

        if (d->currentAlbumId == albumId)
        {
            d->albumView->setCurrentItem(item);
        }

        addChildToTreeView(item, albumTree, albumTree[albumId].childrenIDs);
    }
}

void WSImagesPage::slotCreateAlbumDone(int errCode, const QString& errMsg, const QString& newAlbumId)
{
    if (errCode != 0)
    {
        QMessageBox::critical(QApplication::activeWindow(),
                              i18nc("@title:window", "%1 - Create Album Failed",  d->wsAuth->webserviceName()),
                              i18n("Code: %1. %2", errCode, errMsg));
        return;
    }

    // Pre-set currentAlbumId so that after refreshing, new album will be pre-selected

    setCurrentAlbumId(newAlbumId);

    // We need to refresh albums view

    Q_EMIT signalListAlbumsRequest();
}

} // namespace DigikamGenericUnifiedPlugin

#include "moc_wsimagespage.cpp"
