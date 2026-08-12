/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "expoblendingitemspage.h"

// Qt includes

#include <QLabel>
#include <QVBoxLayout>
#include <QPixmap>
#include <QTimer>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "ditemslist.h"
#include "expoblendingmanager.h"
#include "expoblendingthread.h"
#include "dlayoutbox.h"

namespace DigikamGenericExpoBlendingPlugin
{

class Q_DECL_HIDDEN ItemsPage::Private
{
public:

    Private() = default;

public:

    DItemsList*          list = nullptr;
    ExpoBlendingManager* mngr = nullptr;
};

ItemsPage::ItemsPage(ExpoBlendingManager* const mngr, QWizard* const dlg)
    : DWizardPage(dlg, QString::fromLatin1("<b>%1</b>").arg(i18nc("@title:window", "Set Bracketed Images"))),
      d          (new Private)
{
    d->mngr = mngr;

    DVBox* const vbox    = new DVBox(this);
    QLabel* const label1 = new QLabel(vbox);
    label1->setWordWrap(true);
    label1->setText(QString::fromUtf8("<qt>"
                                      "<p>%1</p>"
                                      "<ul><li>%2</li>"
                                      "<li>%3</li>"
                                      "<li>%4</li></ul>"
                                      "</qt>")
                   .arg(i18nc("@info", "Set here the list of your bracketed images to fuse. Please follow these conditions"))
                   .arg(i18nc("@info", "At least 2 images from the same subject must be added to the stack."))
                   .arg(i18nc("@info", "Do not mix images with different color depth."))
                   .arg(i18nc("@info", "All images must have the same dimensions.")));

    d->list = new DItemsList(vbox);
    d->list->setObjectName(QLatin1String("ExpoBlending ImagesList"));
    d->list->listView()->setColumn(DItemsListView::User1, i18nc("@title: column", "Exposure (EV)"), true);
    d->list->slotAddImages(d->mngr->itemsList());

    setPageWidget(vbox);

    QPixmap leftPix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/assistant-stack.webp")));
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));

    connect(d->mngr->thread(), SIGNAL(signalStarting(DigikamGenericExpoBlendingPlugin::ExpoBlendingActionData)),
            this, SLOT(slotExpoBlendingAction(DigikamGenericExpoBlendingPlugin::ExpoBlendingActionData)));

    connect(d->mngr->thread(), SIGNAL(signalFinished(DigikamGenericExpoBlendingPlugin::ExpoBlendingActionData)),
            this, SLOT(slotExpoBlendingAction(DigikamGenericExpoBlendingPlugin::ExpoBlendingActionData)));

    connect(d->list, SIGNAL(signalAddItems(QList<QUrl>)),
            this, SLOT(slotAddItems(QList<QUrl>)));

    connect(d->list, SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    QTimer::singleShot(0, this, SLOT(slotSetupList()));
}

ItemsPage::~ItemsPage()
{
    delete d;
}

void ItemsPage::slotSetupList()
{
    slotAddItems(d->mngr->itemsList());
}

void ItemsPage::slotAddItems(const QList<QUrl>& urls)
{
    if (!urls.isEmpty())
    {
        d->mngr->thread()->identifyFiles(urls);

        if (!d->mngr->thread()->isRunning())
        {
            d->mngr->thread()->start();
        }
    }

    slotImageListChanged();
}

QList<QUrl> ItemsPage::itemUrls() const
{
    return d->list->imageUrls();
}

void ItemsPage::setIdentity(const QUrl& url, const QString& identity)
{
    DItemsListViewItem* item = d->list->listView()->findItem(url);

    if (item)
    {
        item->setText(DItemsListView::User1, identity);
    }
}

void ItemsPage::slotImageListChanged()
{
    Q_EMIT signalItemsPageIsValid(d->list->imageUrls().count() > 1);
}

void ItemsPage::slotExpoBlendingAction(const DigikamGenericExpoBlendingPlugin::ExpoBlendingActionData& ad)
{
    if (!ad.starting)           // Something is complete...
    {
        switch (ad.action)
        {
            case EXPOBLENDING_IDENTIFY:
            {
                setIdentity(ad.inUrls[0], ad.message);
                break;
            }

            default:
            {
                qCWarning(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Unknown action";
                break;
            }
        }
    }
}

} // namespace DigikamGenericExpoBlendingPlugin

#include "moc_expoblendingitemspage.cpp"
