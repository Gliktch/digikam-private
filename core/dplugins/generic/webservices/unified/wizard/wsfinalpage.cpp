/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : finish page of export tool, where user can watch upload process.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "wsfinalpage.h"

// Qt includes

#include <QIcon>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <QStyle>
#include <QTimer>
#include <QDir>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "wswizard.h"
#include "dlayoutbox.h"
#include "digikam_globals.h"
#include "digikam_debug.h"
#include "dprogresswdg.h"
#include "dhistoryview.h"
#include "wsauthentication.h"

namespace DigikamGenericUnifiedPlugin
{

class Q_DECL_HIDDEN WSFinalPage::Private
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

    DHistoryView*     progressView  = nullptr;
    DProgressWdg*     progressBar   = nullptr;
    bool              complete      = false;
    WSWizard*         wizard        = nullptr;
    DInfoInterface*   iface         = nullptr;
    WSAuthentication* wsAuth        = nullptr;
};

WSFinalPage::WSFinalPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d          (new Private(dialog))
{
    DVBox* const vbox = new DVBox(this);
    d->progressView   = new DHistoryView(vbox);
    d->progressBar    = new DProgressWdg(vbox);

    vbox->setStretchFactor(d->progressBar, 10);
    vbox->setContentsMargins(QMargins());
    vbox->setSpacing(layoutSpacing());

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("WS_send")));

    connect(d->wsAuth, SIGNAL(signalProgress(int)),
            d->progressBar, SLOT(setValue(int)));

    connect(d->wsAuth, SIGNAL(signalMessage(QString,bool)),
            this, SLOT(slotMessage(QString,bool)));

    connect(d->wsAuth, SIGNAL(signalDone()),
            this, SLOT(slotDone()));
}

WSFinalPage::~WSFinalPage()
{
    if (d->wsAuth)
    {
        d->wsAuth->slotCancel();
    }

    delete d;
}

void WSFinalPage::initializePage()
{
    d->complete = false;

    Q_EMIT completeChanged();

    QTimer::singleShot(0, this, SLOT(slotProcess()));
}

void WSFinalPage::slotDone()
{
    d->complete = true;

    Q_EMIT completeChanged();
}

void WSFinalPage::slotProcess()
{
    if (!d->wizard)
    {
        d->progressView->addEntry(i18n("Internal Error"), DHistoryView::ErrorEntry);
        return;
    }

    d->progressView->clear();
    d->progressBar->reset();

    d->progressView->addEntry(i18n("Preparing files..."), DHistoryView::ProgressEntry);
    d->wsAuth->prepareForUpload();

    d->progressView->addEntry(i18n("%1 input items to process", d->wsAuth->numberItemsUpload()),
                              DHistoryView::ProgressEntry);

    d->progressView->addEntry(i18n("Start transferring process..."), DHistoryView::ProgressEntry);
    d->wsAuth->startTransfer();

    d->progressBar->setMinimum(0);
    d->progressBar->setMaximum(d->wsAuth->numberItemsUpload());
}

void WSFinalPage::cleanupPage()
{
    if (d->wsAuth)
    {
        d->wsAuth->slotCancel();
    }
}

void WSFinalPage::slotMessage(const QString& mess, bool err)
{
    d->progressView->addEntry(mess, err ? DHistoryView::ErrorEntry
                                        : DHistoryView::ProgressEntry);
}

bool WSFinalPage::isComplete() const
{
    return d->complete;
}

} // namespace DigikamGenericUnifiedPlugin

#include "moc_wsfinalpage.cpp"
