/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to print images
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "advprintalbumspage.h"

// Qt includes

#include <QIcon>

// Local includes

#include "advprintwizard.h"

namespace DigikamGenericPrintCreatorPlugin
{

class Q_DECL_HIDDEN AdvPrintAlbumsPage::Private
{
public:

    Private(QWizard* const dialog)
    {
        wizard = dynamic_cast<AdvPrintWizard*>(dialog);

        if (wizard)
        {
            iface = wizard->iface();
        }
    }

    bool             albumSupport   = false;
    QWidget*         albumSelector  = nullptr;
    AdvPrintWizard*  wizard         = nullptr;
    DInfoInterface*  iface          = nullptr;
};

AdvPrintAlbumsPage::AdvPrintAlbumsPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d          (new Private(dialog))
{
    if (d->iface)
    {
        d->albumSelector = d->iface->albumChooser(this);

        connect(d->iface, SIGNAL(signalAlbumChooserSelectionChanged()),
                this, SIGNAL(completeChanged()));
    }
    else
    {
        d->albumSelector = new QWidget(this);
    }

    setPageWidget(d->albumSelector);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-mail")));
}

AdvPrintAlbumsPage::~AdvPrintAlbumsPage()
{
    delete d;
}

bool AdvPrintAlbumsPage::validatePage()
{
    if (!d->iface)
    {
        return false;
    }

    if (d->iface->albumChooserItems().isEmpty())
    {
        return false;
    }

    d->wizard->settings()->inputImages.clear();

    // update image list with album contents.

    const auto urls = d->iface->albumsItems(d->iface->albumChooserItems());

    for (const QUrl& url : urls)
    {
        d->wizard->settings()->inputImages << url;
    }

    return true;
}

bool AdvPrintAlbumsPage::isComplete() const
{
    if (!d->iface)
    {
        return false;
    }

    return (!d->iface->albumChooserItems().isEmpty());
}

} // namespace DigikamGenericPrintCreatorPlugin

#include "moc_advprintalbumspage.cpp"
