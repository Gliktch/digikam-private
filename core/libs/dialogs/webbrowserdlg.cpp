/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-21
 * Description : a simple web browser dialog based on Qt WebEngine.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "webbrowserdlg.h"
#include "digikam_config.h"

// Qt includes

#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QIcon>
#include <QToolBar>
#include <QDesktopServices>
#include <QNetworkCookieJar>
#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>
#include <QWebEngineFindTextResult>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "statusprogressbar.h"
#include "searchtextbar.h"
#include "dxmlguiwindow.h"
#include "digikam_debug.h"
#include "digikam_globals.h"

namespace Digikam
{

class Q_DECL_HIDDEN WebBrowserDlg::Private
{
public:

    Private() = default;

public:

    QUrl               home;
    QWebEngineView*    browser      = nullptr;
    QToolBar*          toolbar      = nullptr;
    StatusProgressBar* progressbar  = nullptr;
    SearchTextBar*     searchbar    = nullptr;
};

WebBrowserDlg::WebBrowserDlg(const QUrl& url, QWidget* const parent, bool hideDeskBrowser)
    : QDialog(parent),
      d      (new Private)
{
    setModal(false);
    setAttribute(Qt::WA_DeleteOnClose, true);

    d->home    = url;

    d->browser = new QWebEngineView(this);
    d->browser->settings()->setAttribute(QWebEngineSettings::WebGLEnabled, true);
    d->browser->settings()->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, false);
    d->browser->page()->profile()->cookieStore()->deleteAllCookies();

    // --------------------------

    d->toolbar = new QToolBar(this);
    d->toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    d->toolbar->addAction(d->browser->pageAction(QWebEnginePage::Back));
    d->toolbar->addAction(d->browser->pageAction(QWebEnginePage::Forward));
    d->toolbar->addAction(d->browser->pageAction(QWebEnginePage::Reload));
    d->toolbar->addAction(d->browser->pageAction(QWebEnginePage::Stop));

    QAction* const gohome  = new QAction(QIcon::fromTheme(QLatin1String("go-home")),
                                         i18n("Home"), this);
    gohome->setToolTip(i18n("Go back to Home page"));
    d->toolbar->addAction(gohome);

    QAction* const deskweb = new QAction(QIcon::fromTheme(QLatin1String("internet-web-browser")),
                                         i18n("Desktop Browser"), this);
    deskweb->setToolTip(i18n("Open Home page with default desktop Web browser"));

    if (!hideDeskBrowser)
    {
        d->toolbar->addAction(deskweb);
    }

    // --------------------------

    d->searchbar   = new SearchTextBar(this, QLatin1String("WebBrowserDlgSearchBar"));
    d->searchbar->setHighlightOnResult(true);

    d->progressbar = new StatusProgressBar(this);
    d->progressbar->setProgressTotalSteps(100);
    d->progressbar->setAlignment(Qt::AlignLeft);
    d->progressbar->setNotify(false);

    // ----------------------

    QGridLayout* const grid = new QGridLayout(this);
    grid->setSpacing(layoutSpacing());

    grid->addWidget(d->toolbar,     0, 0, 1, 1);
    grid->addWidget(d->searchbar,   0, 2, 1, 1);
    grid->addWidget(d->browser,     1, 0, 1, 3);
    grid->addWidget(d->progressbar, 2, 0, 1, 3);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(1, 10);
    setLayout(grid);

    // ----------------------

    connect(d->browser, SIGNAL(titleChanged(QString)),
            this, SLOT(slotTitleChanged(QString)));

    connect(d->browser, SIGNAL(urlChanged(QUrl)),
            this, SLOT(slotUrlChanged(QUrl)));

    connect(d->browser, SIGNAL(loadStarted()),
            this, SLOT(slotLoadingStarted()));

    connect(d->browser, SIGNAL(loadFinished(bool)),
            this, SLOT(slotLoadingFinished(bool)));

    connect(d->searchbar, SIGNAL(signalSearchTextSettings(SearchTextSettings)),
            this, SLOT(slotSearchTextChanged(SearchTextSettings)));

    connect(d->browser, SIGNAL(loadProgress(int)),
            d->progressbar, SLOT(setProgressValue(int)));

    connect(gohome, SIGNAL(triggered()),
            this, SLOT(slotGoHome()));

    connect(deskweb, SIGNAL(triggered()),
            this, SLOT(slotDesktopWebBrowser()));

    // ----------------------

    slotGoHome();
}

WebBrowserDlg::~WebBrowserDlg()
{
    delete d;
}

void WebBrowserDlg::showEvent(QShowEvent* e)
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("WebBrowserDlg"));

    DXmlGuiWindow::setGoodDefaultWindowSize(windowHandle());
    DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size());

    QDialog::showEvent(e);
}

void WebBrowserDlg::closeEvent(QCloseEvent* e)
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("WebBrowserDlg"));

    DXmlGuiWindow::saveWindowSize(windowHandle(), group);

    Q_EMIT closeView(false);

    e->accept();
}

void WebBrowserDlg::slotUrlChanged(const QUrl& url)
{
    d->progressbar->setText(url.toString());

    Q_EMIT urlChanged(url);
}

void WebBrowserDlg::slotTitleChanged(const QString& title)
{
    setWindowTitle(title);
}

void WebBrowserDlg::slotLoadingStarted()
{
    d->progressbar->setProgressBarMode(StatusProgressBar::ProgressBarMode);
}

void WebBrowserDlg::slotLoadingFinished(bool b)
{
    QString curUrl = d->browser->url().toString();

    d->progressbar->setProgressBarMode(StatusProgressBar::TextMode, curUrl);

    if (!b)
    {
        d->progressbar->setText(i18n("Cannot load page %1", curUrl));
    }
}

void WebBrowserDlg::slotSearchTextChanged(const SearchTextSettings& settings)
{

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

    d->browser->findText(settings.text,
                         (settings.caseSensitive == Qt::CaseSensitive) ? QWebEnginePage::FindCaseSensitively
                                                                       : QWebEnginePage::FindFlags(),
                         [this](const QWebEngineFindTextResult& result)
                            {
                                d->searchbar->slotSearchResult(result.activeMatch());
                            }
                        );

#else

    d->browser->findText(settings.text,
                         (settings.caseSensitive == Qt::CaseSensitive) ? QWebEnginePage::FindCaseSensitively
                                                                       : QWebEnginePage::FindFlags(),
                         [this](bool found)
                            {
                                d->searchbar->slotSearchResult(found);
                            }
                        );

#endif

}

void WebBrowserDlg::slotGoHome()
{
    d->browser->setUrl(d->home);
}

void WebBrowserDlg::slotDesktopWebBrowser()
{
    QDesktopServices::openUrl(d->home);
}

} // namespace Digikam

#include "moc_webbrowserdlg.cpp"
