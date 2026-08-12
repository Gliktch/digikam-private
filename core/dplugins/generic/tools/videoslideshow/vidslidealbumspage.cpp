/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "vidslidealbumspage.h"

// Qt includes

#include <QIcon>
#include <QPixmap>

// Local includes

#include "vidslidewizard.h"

namespace DigikamGenericVideoSlideShowPlugin
{

class Q_DECL_HIDDEN VidSlideAlbumsPage::Private
{
public:

    explicit Private(QWizard* const dialog)
    {
        wizard = dynamic_cast<VidSlideWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
        }
    }

    bool              albumSupport  = false;
    QWidget*          albumSelector = nullptr;
    VidSlideWizard*   wizard        = nullptr;
    VidSlideSettings* settings      = nullptr;
};

VidSlideAlbumsPage::VidSlideAlbumsPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d          (new Private(dialog))
{
    if (d->settings && d->settings->iface)
    {
        d->albumSelector = d->settings->iface->albumChooser(this);

        connect(d->settings->iface, SIGNAL(signalAlbumChooserSelectionChanged()),
                this, SIGNAL(completeChanged()));
    }
    else
    {
        d->albumSelector = new QWidget(this);
    }

    setPageWidget(d->albumSelector);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-pictures")));
}

VidSlideAlbumsPage::~VidSlideAlbumsPage()
{
    delete d;
}

bool VidSlideAlbumsPage::validatePage()
{
    if (d->settings && d->wizard)
    {
        if (!d->settings->iface)
        {
            return false;
        }

        if (d->settings->iface->albumChooserItems().isEmpty())
        {
            return false;
        }

        d->wizard->settings()->inputImages.clear();

        // update image list with album contents.

        const auto urls = d->settings->iface->albumsItems(d->settings->iface->albumChooserItems());

        for (const QUrl& url : urls)
        {
            d->wizard->settings()->inputImages << url;
        }

        return true;
    }

    return false;
}

bool VidSlideAlbumsPage::isComplete() const
{
    if (d->settings && d->settings->iface)
    {
        return (!d->settings->iface->albumChooserItems().isEmpty());
    }

    return false;
}

} // namespace DigikamGenericVideoSlideShowPlugin

#include "moc_vidslidealbumspage.cpp"
