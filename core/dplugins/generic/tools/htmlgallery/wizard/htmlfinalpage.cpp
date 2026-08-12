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

#include "htmlfinalpage.h"

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

#include "digikam_globals.h"
#include "digikam_debug.h"
#include "abstractthemeparameter.h"
#include "gallerygenerator.h"
#include "webbrowserdlg.h"
#include "dprogresswdg.h"
#include "dhistoryview.h"
#include "galleryinfo.h"
#include "htmlwizard.h"
#include "dlayoutbox.h"

namespace DigikamGenericHtmlGalleryPlugin
{

class Q_DECL_HIDDEN HTMLFinalPage::Private
{
public:

    Private() = default;

    DHistoryView* progressView  = nullptr;
    DProgressWdg* progressBar   = nullptr;
    bool          complete      = false;
};

HTMLFinalPage::HTMLFinalPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d          (new Private)
{
    setObjectName(QLatin1String("FinalPage"));

    DVBox* const vbox = new DVBox(this);
    d->progressView   = new DHistoryView(vbox);
    d->progressBar    = new DProgressWdg(vbox);

    vbox->setStretchFactor(d->progressBar, 10);
    vbox->setContentsMargins(QMargins());
    vbox->setSpacing(layoutSpacing());

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("system-run")));
}

HTMLFinalPage::~HTMLFinalPage()
{
    delete d;
}

void HTMLFinalPage::initializePage()
{
    d->complete = false;

    Q_EMIT completeChanged();

    QTimer::singleShot(0, this, SLOT(slotProcess()));
}

void HTMLFinalPage::slotProcess()
{
    HTMLWizard* const wizard = dynamic_cast<HTMLWizard*>(assistant());

    if (!wizard)
    {
        d->progressView->addEntry(i18n("Internal Error"),
                                  DHistoryView::ErrorEntry);
        return;
    }

    d->progressView->clear();
    d->progressBar->reset();

    GalleryInfo* const info  = wizard->galleryInfo();

    // Generate GalleryInfo

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << info;

    d->progressView->addEntry(i18n("Starting to generate gallery..."),
                              DHistoryView::ProgressEntry);

    if (info->m_getOption == GalleryInfo::ALBUMS)
    {
        if (!info->m_iface)
        {
            return;
        }

        d->progressView->addEntry(i18n("%1 albums to process:", info->m_albumList.count()),
                                  DHistoryView::ProgressEntry);

        const auto urls = info->m_iface->albumsItems(info->m_albumList);

        for (const QUrl& url : urls)
        {
            d->progressView->addEntry(QDir::toNativeSeparators(url.toLocalFile()),
                                      DHistoryView::ProgressEntry);
        }
    }
    else
    {
        d->progressView->addEntry(i18n("%1 items to process", info->m_imageList.count()),
                                  DHistoryView::ProgressEntry);
    }

    d->progressView->addEntry(i18n("Output directory: %1",
                              QDir::toNativeSeparators(info->destUrl().toLocalFile())),
                              DHistoryView::ProgressEntry);

    GalleryGenerator generator(info);
    generator.setProgressWidgets(d->progressView, d->progressBar);

    connect(this, SIGNAL(signalCancel()),
            &generator, SLOT(slotCancel()));

    if (!generator.run())
    {
        return;
    }

    if (generator.warnings())
    {
        d->progressView->addEntry(i18n("Gallery is completed, but some warnings occurred."),
                                  DHistoryView::WarningEntry);
    }
    else
    {
        d->progressView->addEntry(i18n("Gallery completed."),
                                  DHistoryView::ProgressEntry);
    }

    QUrl url = info->destUrl().adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1String("/index.html"));

    switch (info->openInBrowser())
    {
        case GalleryConfig::DESKTOP:
        {
            QDesktopServices::openUrl(url);
            d->progressView->addEntry(i18n("Opening gallery with default desktop browser..."),
                                      DHistoryView::ProgressEntry);
            break;
        }

        case GalleryConfig::INTERNAL:
        {
            WebBrowserDlg* const browser = new WebBrowserDlg(url, this);
            browser->show();
            d->progressView->addEntry(i18n("Opening gallery with internal browser..."),
                                      DHistoryView::ProgressEntry);
            break;
        }

        default:
        {
            break;
        }
    }

    d->complete = true;

    Q_EMIT completeChanged();
}

void HTMLFinalPage::cleanupPage()
{
    Q_EMIT signalCancel();
}

bool HTMLFinalPage::isComplete() const
{
    return d->complete;
}

} // namespace DigikamGenericHtmlGalleryPlugin

#include "moc_htmlfinalpage.cpp"
