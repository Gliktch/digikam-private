/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "expoblendingwizard.h"

// KDE includes

#include <klocalizedstring.h>

// Locale includes

#include "expoblendingintropage.h"
#include "expoblendingitemspage.h"
#include "expoblendinglastpage.h"
#include "expoblendingmanager.h"
#include "expoblendingpreprocesspage.h"

namespace DigikamGenericExpoBlendingPlugin
{

class Q_DECL_HIDDEN ExpoBlendingWizard::Private
{
public:

    Private() = default;

    ExpoBlendingManager*        mngr                = nullptr;

    ExpoBlendingIntroPage*      introPage           = nullptr;
    ItemsPage*                  itemsPage           = nullptr;
    ExpoBlendingPreProcessPage* preProcessingPage   = nullptr;
    ExpoBlendingLastPage*       lastPage            = nullptr;

    bool                        preProcessed        = false;
};

ExpoBlendingWizard::ExpoBlendingWizard(ExpoBlendingManager* const mngr, QWidget* const parent)
    : DWizardDlg(parent, QLatin1String("ExpoBlending Wizard")),
      d         (new Private)
{
    setModal(false);
    setWindowTitle(i18nc("@title:window", "Stacked Images Tool"));

    d->mngr              = mngr;
    d->introPage         = new ExpoBlendingIntroPage(d->mngr, this);
    d->itemsPage         = new ItemsPage(d->mngr, this);
    d->preProcessingPage = new ExpoBlendingPreProcessPage(d->mngr, this);
    d->lastPage          = new ExpoBlendingLastPage(d->mngr, this);

    // ---------------------------------------------------------------

    connect(d->introPage, SIGNAL(signalExpoBlendingIntroPageIsValid(bool)),
            this, SLOT(slotExpoBlendingIntroPageIsValid(bool)));

    connect(d->itemsPage, SIGNAL(signalItemsPageIsValid(bool)),
            this, SLOT(slotItemsPageIsValid(bool)));

    connect(d->preProcessingPage, SIGNAL(signalPreProcessed(ExpoBlendingItemUrlsMap)),
            this, SLOT(slotPreProcessed(ExpoBlendingItemUrlsMap)));

    connect(this, SIGNAL(currentIdChanged(int)),
            this, SLOT(slotCurrentIdChanged(int)));

    d->introPage->setComplete(d->introPage->binariesFound());
}

ExpoBlendingWizard::~ExpoBlendingWizard()
{
    delete d;
}

ExpoBlendingManager* ExpoBlendingWizard::manager() const
{
    return d->mngr;
}

QList<QUrl> ExpoBlendingWizard::itemUrls() const
{
    return d->itemsPage->itemUrls();
}

bool ExpoBlendingWizard::validateCurrentPage()
{
    if      (currentPage() == d->itemsPage)
    {
        d->mngr->setItemsList(d->itemsPage->itemUrls());
    }
    else if (currentPage() == d->preProcessingPage && !d->preProcessed)
    {
        // Do not give access to Next button during alignment process.

        d->preProcessingPage->setComplete(false);
        d->preProcessingPage->process();
        d->preProcessed = true;

        // Next is handled with signals/slots

        return false;
    }

    return true;
}

void ExpoBlendingWizard::slotCurrentIdChanged(int id)
{
    if ((page(id) != d->lastPage) && d->preProcessed)
    {
        d->preProcessed = false;
        d->preProcessingPage->cancel();
        d->preProcessingPage->setComplete(true);
    }
}

void ExpoBlendingWizard::slotExpoBlendingIntroPageIsValid(bool binariesFound)
{
    d->introPage->setComplete(binariesFound);
}

void ExpoBlendingWizard::slotPreProcessed(const ExpoBlendingItemUrlsMap& map)
{
    if (map.isEmpty())
    {
        // pre-processing failed.

        d->preProcessingPage->setComplete(false);
        d->preProcessed = false;
    }
    else
    {
        // pre-processing Done.

        d->mngr->setPreProcessedMap(map);
        next();
    }
}

void ExpoBlendingWizard::slotItemsPageIsValid(bool valid)
{
    d->itemsPage->setComplete(valid);
}

} // namespace DigikamGenericExpoBlendingPlugin

#include "moc_expoblendingwizard.cpp"
