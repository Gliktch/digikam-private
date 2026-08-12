/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "htmlselectionpage.h"

// Qt includes

#include <QIcon>
#include <QPixmap>
#include <QStackedWidget>

// Local includes

#include "htmlwizard.h"
#include "galleryinfo.h"
#include "ditemslist.h"

namespace DigikamGenericHtmlGalleryPlugin
{

class Q_DECL_HIDDEN HTMLSelectionPage::Private
{
public:

    explicit Private(QWizard* const dialog)
    {
        wizard = dynamic_cast<HTMLWizard*>(dialog);

        if (wizard)
        {
            info  = wizard->galleryInfo();
            iface = info->m_iface;
        }
    }

    bool             albumSupport   = false;
    QWidget*         albumSelector  = nullptr;
    DItemsList*      imageList      = nullptr;
    QStackedWidget*  stack          = nullptr;
    HTMLWizard*      wizard         = nullptr;
    GalleryInfo*     info           = nullptr;
    DInfoInterface*  iface          = nullptr;
};

HTMLSelectionPage::HTMLSelectionPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d          (new Private(dialog))
{
    setObjectName(QLatin1String("AlbumSelectorPage"));

    d->stack              = new QStackedWidget(this);
    d->albumSupport       = (d->iface && d->iface->supportAlbums());

    if (d->albumSupport)
    {
        d->albumSelector = d->iface->albumChooser(this);
    }
    else
    {
        d->albumSelector = new QWidget(this);
    }

    d->stack->insertWidget(GalleryInfo::ALBUMS, d->albumSelector);

    d->imageList          = new DItemsList(this);
    d->imageList->setObjectName(QLatin1String("HTMLGalleries ImagesList"));
    d->imageList->setControlButtonsPlacement(DItemsList::ControlButtonsBelow);
    d->stack->insertWidget(GalleryInfo::IMAGES, d->imageList);

    setPageWidget(d->stack);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-pictures")));

    if (d->albumSupport)
    {
        connect(d->iface, SIGNAL(signalAlbumChooserSelectionChanged()),
                this, SIGNAL(completeChanged()));
    }

    connect(d->imageList, SIGNAL(signalImageListChanged()),
            this, SIGNAL(completeChanged()));
}

HTMLSelectionPage::~HTMLSelectionPage()
{
    delete d;
}

void HTMLSelectionPage::setItemsList(const QList<QUrl>& urls)
{
    d->imageList->slotAddImages(urls);
}

void HTMLSelectionPage::initializePage()
{
    d->imageList->setIface(d->iface);

    if (d->info->m_getOption == GalleryInfo::IMAGES)
    {
        d->imageList->loadImagesFromCurrentSelection();
    }

    d->stack->setCurrentIndex(d->info->m_getOption);
}

bool HTMLSelectionPage::validatePage()
{
    if (d->stack->currentIndex() == GalleryInfo::ALBUMS)
    {
        if (d->albumSupport)
        {
            if (d->iface->albumChooserItems().isEmpty())
                return false;

            d->info->m_albumList = d->iface->albumChooserItems();
        }
        else
        {
            return false;
        }
    }
    else
    {
        if (d->imageList->imageUrls().isEmpty())
        {
            return false;
        }

        d->info->m_imageList = d->imageList->imageUrls();
    }

    return true;
}

bool HTMLSelectionPage::isComplete() const
{
    if (d->stack->currentIndex() == GalleryInfo::ALBUMS)
    {
        if (!d->albumSupport)
        {
            return false;
        }

        return (!d->iface->albumChooserItems().isEmpty());
    }

    return (!d->imageList->imageUrls().isEmpty());
}

} // namespace DigikamGenericHtmlGalleryPlugin

#include "moc_htmlselectionpage.cpp"
