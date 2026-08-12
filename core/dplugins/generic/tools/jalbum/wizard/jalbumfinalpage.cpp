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

#include "jalbumfinalpage.h"

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
#include "jalbumwizard.h"
#include "jalbumsettings.h"
#include "jalbumgenerator.h"
#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "dprogresswdg.h"
#include "dhistoryview.h"
#include "webbrowserdlg.h"

namespace DigikamGenericJAlbumPlugin
{

class Q_DECL_HIDDEN JAlbumFinalPage::Private
{
public:

    Private() = default;

public:

    DHistoryView* progressView  = nullptr;
    DProgressWdg* progressBar   = nullptr;
    bool          complete      = false;
};

JAlbumFinalPage::JAlbumFinalPage(QWizard* const dialog, const QString& title)
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

JAlbumFinalPage::~JAlbumFinalPage()
{
    delete d;
}

void JAlbumFinalPage::initializePage()
{
    d->complete = false;

    Q_EMIT completeChanged();

    QTimer::singleShot(0, this, SLOT(slotProcess()));
}

void JAlbumFinalPage::slotProcess()
{
    JAlbumWizard* const wizard = dynamic_cast<JAlbumWizard*>(assistant());

    if (!wizard)
    {
        d->progressView->addEntry(i18n("Internal Error"),
                                  DHistoryView::ErrorEntry);
        return;
    }

    d->progressView->clear();
    d->progressBar->reset();

    JAlbumSettings* const info  = wizard->settings();

    // Generate JAlbumSettings

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << info;

    d->progressView->addEntry(i18n("Starting to generate jAlbum..."),
                              DHistoryView::ProgressEntry);

    if (info->m_getOption == JAlbumSettings::ALBUMS)
    {
        if (!info->m_iface)
            return;

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
                              QDir::toNativeSeparators(info->m_destPath)),
                              DHistoryView::ProgressEntry);

    JAlbumGenerator generator(info);
    generator.setProgressWidgets(d->progressView, d->progressBar);

    if (!generator.run())
    {
        return;
    }

    if (generator.warnings())
    {
        d->progressView->addEntry(i18n("Jalbum is completed, but some warnings occurred."),
                                  DHistoryView::WarningEntry);
    }
    else
    {
        d->progressView->addEntry(i18n("Jalbum completed."),
                                  DHistoryView::ProgressEntry);
    }

    d->complete = true;

    Q_EMIT completeChanged();
}

bool JAlbumFinalPage::isComplete() const
{
    return d->complete;
}

} // namespace DigikamGenericJAlbumPlugin

#include "moc_jalbumfinalpage.cpp"
