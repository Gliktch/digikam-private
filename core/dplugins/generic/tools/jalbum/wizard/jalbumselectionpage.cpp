/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate jAlbum image galleries
 *
 * SPDX-FileCopyrightText: 2013-2019 by Andrew Goodbody <ajg zero two at elfringham dot co dot uk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "jalbumselectionpage.h"

// Qt includes

#include <QIcon>
#include <QPixmap>
#include <QStackedWidget>

// Local includes

#include "jalbumwizard.h"
#include "jalbumsettings.h"
#include "ditemslist.h"

namespace DigikamGenericJAlbumPlugin
{

class Q_DECL_HIDDEN JAlbumSelectionPage::Private
{
public:

    explicit Private(QWizard* const dialog)
    {
        wizard = dynamic_cast<JAlbumWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
            iface    = settings->m_iface;
        }
    }

public:

    bool            albumSupport    = false;
    QWidget*        albumSelector   = nullptr;
    DItemsList*     imageList       = nullptr;
    QStackedWidget* stack           = nullptr;
    JAlbumWizard*   wizard          = nullptr;
    JAlbumSettings* settings        = nullptr;
    DInfoInterface* iface           = nullptr;
};

JAlbumSelectionPage::JAlbumSelectionPage(QWizard* const dialog, const QString& title)
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

    d->stack->insertWidget(JAlbumSettings::ALBUMS, d->albumSelector);

    d->imageList          = new DItemsList(this);
    d->imageList->setObjectName(QLatin1String("JAlbum ImagesList"));
    d->imageList->setControlButtonsPlacement(DItemsList::ControlButtonsBelow);
    d->stack->insertWidget(JAlbumSettings::IMAGES, d->imageList);

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

JAlbumSelectionPage::~JAlbumSelectionPage()
{
    delete d;
}


void JAlbumSelectionPage::setItemsList(const QList<QUrl>& urls)
{
    d->imageList->slotAddImages(urls);
}

void JAlbumSelectionPage::initializePage()
{
    d->imageList->setIface(d->iface);

    if (d->settings->m_getOption == JAlbumSettings::IMAGES)
    {
        d->imageList->loadImagesFromCurrentSelection();
    }

    d->stack->setCurrentIndex(d->settings->m_getOption);
}

bool JAlbumSelectionPage::validatePage()
{
    if (d->stack->currentIndex() == JAlbumSettings::ALBUMS)
    {
        if (d->albumSupport)
        {
            if (d->iface->albumChooserItems().isEmpty())
                return false;

            d->settings->m_albumList = d->iface->albumChooserItems();
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

        d->settings->m_imageList = d->imageList->imageUrls();
    }

    return true;
}

bool JAlbumSelectionPage::isComplete() const
{
    if (d->stack->currentIndex() == JAlbumSettings::ALBUMS)
    {
        if (!d->albumSupport)
        {
            return false;
        }

        return (!d->iface->albumChooserItems().isEmpty());
    }

    return (!d->imageList->imageUrls().isEmpty());
}

} // namespace DigikamGenericJAlbumPlugin

#include "moc_jalbumselectionpage.cpp"
