/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-12-31
 * Description : Online version dialog.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "onlineversiondlg.h"

// Qt includes

#include <QUrl>
#include <QTimer>
#include <QAction>
#include <QList>
#include <QFile>
#include <QFont>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QFileDevice>
#include <QStyle>
#include <QIcon>
#include <QLabel>
#include <QImage>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QPushButton>
#include <QProgressBar>
#include <QLocale>
#include <QProcess>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QTextBrowser>
#include <QMargins>
#include <QGroupBox>
#include <QTextDocument>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "onlineversionchecker.h"
#include "onlineversiondwnl.h"
#include "dfileoperations.h"
#include "itempropertiestab.h"

namespace Digikam
{

class Q_DECL_HIDDEN OnlineVersionDlg::Private
{
public:

    Private() = default;

public:

    bool                   preRelease       = false;
    bool                   updateWithDebug  = false;

    QString                curVersion;
    QDateTime              curBuildDt;

    QDateTime              onlineDt;        ///< Build date for pre-release only.
    QString                newVersion;      ///< For stable => version IDs ; for pre-release => build ISO date.

    QProgressBar*          bar              = nullptr;
    QLabel*                label            = nullptr;
    QWidget*               stats            = nullptr;
    QLabel*                received         = nullptr;
    QLabel*                total            = nullptr;
    QLabel*                rate             = nullptr;
    QLabel*                remain           = nullptr;
    QLabel*                logo             = nullptr;
    QDialogButtonBox*      buttons          = nullptr;
    QTimer*                speedTimer       = nullptr;
    QDateTime              dwnlStart;

    OnlineVersionChecker*  checker          = nullptr;
    OnlineVersionDwnl*     dwnloader        = nullptr;
};

OnlineVersionDlg::OnlineVersionDlg(QWidget* const parent,
                                   const QString& version,
                                   const QDateTime& buildDt,
                                   bool checkPreRelease,
                                   bool updateWithDebug)
    : QDialog(parent),
      d      (new Private)
{
    setModal(true);

    d->curVersion      = version;
    d->curBuildDt      = buildDt;
    d->preRelease      = checkPreRelease;
    d->updateWithDebug = updateWithDebug;
    d->checker         = new OnlineVersionChecker(this, d->preRelease);
    d->checker->setCurrentVersion(d->curVersion);
    d->checker->setCurrentBuildDate(d->curBuildDt);
    d->dwnloader       = new OnlineVersionDwnl(this, d->preRelease, d->updateWithDebug);

    connect(d->checker, SIGNAL(signalNewVersionAvailable(QString)),
            this, SLOT(slotNewVersionAvailable(QString)));

    connect(d->checker, SIGNAL(signalNewVersionCheckError(QString)),
            this, SLOT(slotNewVersionCheckError(QString)));

    connect(d->dwnloader, SIGNAL(signalDownloadError(QString)),
            this, SLOT(slotDownloadError(QString)));

    connect(d->dwnloader, SIGNAL(signalDownloadProgress(qint64,qint64)),
            this, SLOT(slotDownloadProgress(qint64,qint64)));

    connect(d->dwnloader, SIGNAL(signalComputeChecksum()),
            this, SLOT(slotComputeChecksum()));

    d->speedTimer            = new QTimer(this);

    connect(d->speedTimer, SIGNAL(timeout()),
            this, SLOT(slotUpdateStats()));

    QWidget* const page      = new QWidget(this);
    QGridLayout* const grid  = new QGridLayout(page);
    d->label                 = new QLabel(page);
    d->label->setOpenExternalLinks(true);

    d->stats                 = new QWidget(page);
    d->stats->setVisible(false);

    QFont sfnt               = QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);
    sfnt.setItalic(true);
    sfnt.setStyleHint(QFont::Monospace);
    d->stats->setFont(sfnt);
    QFontMetrics fontMt(sfnt);
    QRect fontRect           = fontMt.boundingRect(0, 0, width(), height(), 0, QLatin1String("XXXXXXXXXX"));

    d->received              = new QLabel(d->stats);
    d->received->setAlignment(Qt::AlignRight);
    d->total                 = new QLabel(d->stats);
    d->total->setAlignment(Qt::AlignRight);
    d->rate                  = new QLabel(d->stats);
    d->rate->setAlignment(Qt::AlignRight);
    d->remain                = new QLabel(d->stats);
    d->remain->setAlignment(Qt::AlignRight);

    QGridLayout* const grid2 = new QGridLayout(d->stats);
    grid2->addWidget(new QLabel(i18nc("@info: download status", "Received:")),  0, 0, 1, 1);
    grid2->addWidget(new QLabel(i18nc("@info: download status", "Total:")),     1, 0, 1, 1);
    grid2->addWidget(new QLabel(i18nc("@info: download status", "Rate:")),      2, 0, 1, 1);
    grid2->addWidget(new QLabel(i18nc("@info: download status", "ETA:")),       3, 0, 1, 1);
    grid2->addWidget(d->received,                                               0, 2, 1, 1);
    grid2->addWidget(d->total,                                                  1, 2, 1, 1);
    grid2->addWidget(d->rate,                                                   2, 2, 1, 1);
    grid2->addWidget(d->remain,                                                 3, 2, 1, 1);
    grid2->setContentsMargins(QMargins());
    grid2->setSpacing(0);
    grid2->setColumnMinimumWidth(2, fontRect.width());

    if (d->preRelease)
    {
        setWindowTitle(i18nc("@title:window", "Online Version Checker - Pre-Release"));
        d->label->setText(i18n("Check for new pre-release version available, please wait..."));
    }
    else
    {
        setWindowTitle(i18nc("@title:window", "Online Version Checker - Stable Version"));
        d->label->setText(i18n("Check for new stable version available, please wait..."));
    }

    d->logo                = new QLabel(page);

    if (QApplication::applicationName() == QLatin1String("digikam"))
    {
        d->logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48, 48)));
    }
    else
    {
        d->logo->setPixmap(QIcon::fromTheme(QLatin1String("showfoto")).pixmap(QSize(48, 48)));
    }

    d->bar                 = new QProgressBar(page);
    d->bar->setFormat(i18nc("%p is the percent value, % is the percent sign", "%p%"));
    d->bar->setMaximum(1);
    d->bar->setMinimum(0);
    d->bar->setValue(0);

    d->buttons             = new QDialogButtonBox(QDialogButtonBox::Help  |
                                                  QDialogButtonBox::Apply |         // Download button
                                                  QDialogButtonBox::Reset |         // Configure button
                                                  QDialogButtonBox::Cancel,
                                                  page);
    d->buttons->button(QDialogButtonBox::Cancel)->setDefault(true);

    grid->addWidget(d->logo,     0, 0, 1, 1);
    grid->addWidget(d->label,    0, 1, 1, 2);
    grid->addWidget(d->stats,    0, 3, 1, 1);
    grid->addWidget(d->bar,      1, 0, 1, 4);
    grid->setSpacing(layoutSpacing());
    grid->setContentsMargins(QMargins());
    grid->setColumnStretch(2, 10);
    grid->setRowStretch(2, 10);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(close()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    adjustSize();

    d->buttons->button(QDialogButtonBox::Apply)->setVisible(false);
    d->buttons->button(QDialogButtonBox::Apply)->setEnabled(false);
    d->buttons->button(QDialogButtonBox::Reset)->setVisible(true);
    d->buttons->button(QDialogButtonBox::Reset)->setEnabled(true);
    d->buttons->button(QDialogButtonBox::Reset)->setText(i18n("Configure..."));
    d->buttons->button(QDialogButtonBox::Reset)->setToolTip(i18n("Open setup dialog page to configure updates."));
    d->buttons->button(QDialogButtonBox::Reset)->setIcon(QIcon::fromTheme(QLatin1String("configure")));

    disconnect(d->buttons->button(QDialogButtonBox::Reset), SIGNAL(clicked()), 0, 0);

    connect(d->buttons->button(QDialogButtonBox::Reset), SIGNAL(clicked()),
            this, SLOT(slotSetupUpdate()));

    d->checker->checkForNewVersion();
}

OnlineVersionDlg::~OnlineVersionDlg()
{
    d->checker->cancelCheck();
    d->dwnloader->cancelDownload();

    delete d;
}

void OnlineVersionDlg::slotNewVersionAvailable(const QString& version)
{
    d->newVersion = version;
    d->bar->hide();
    d->stats->hide();
    d->speedTimer->stop();

    d->buttons->button(QDialogButtonBox::Apply)->setVisible(true);
    d->buttons->button(QDialogButtonBox::Apply)->setEnabled(true);
    d->buttons->button(QDialogButtonBox::Apply)->setText(i18n("Download"));
    d->buttons->button(QDialogButtonBox::Apply)->setIcon(QIcon::fromTheme(QLatin1String("download")));
    d->buttons->button(QDialogButtonBox::Reset)->setVisible(true);

    disconnect(d->buttons->button(QDialogButtonBox::Apply), SIGNAL(clicked()), 0, 0);

    connect(d->buttons->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(slotDownloadInstaller()));

    if (d->preRelease)
    {
        d->onlineDt = asDateTimeUTC(QDateTime::fromString(d->newVersion, QLatin1String("yyyyMMddTHHmmss")));

        d->label->setText(i18n("<p>Current <b>%1</b> pre-release date is <i>%2</i>.</p>"
                               "<p>New pre-release built on <i>%3</i> is available on this <a href='%4'>repository</a>.</p>"
                               "<p>Press <b>Download</b> to get the file...</p>"
                               "<p>Note: from Setup/Misc panel, you can switch to check for stable release only.<br>"
                               "Stable versions are safe to use in production.</p>"
                               "<p>Press <b>Configure</b> if you want to customize update options from setup dialog.</p>",
                               qApp->applicationName(),
                               QLocale().toString(d->curBuildDt, QLocale::ShortFormat),
                               QLocale().toString(d->onlineDt, QLocale::ShortFormat),
                               d->dwnloader->downloadUrl()));
    }
    else
    {
        d->label->setText(i18n("<p>Current <b>%1</b> stable version is <i>%2</i></p>"
                               "<p>New stable version <i>%3</i> is available on this <a href='%4'>repository</a>.</p>"
                               "<p>Press <b>Download</b> to get the file...</p>"
                               "<p>Note: from Setup/Misc panel, you can switch to check for weekly pre-release.<br>"
                               "Pre-release versions are dedicated to test quickly new features.<br>"
                               "It is not recommended to use pre-releases in production as bugs can remain,<br>"
                               "unless you know what you are doing.</p>"
                               "<p>Press <b>Configure</b> if you want to customize update options from setup dialog.</p>",
                               qApp->applicationName(),
                               d->curVersion,
                               version,
                               d->dwnloader->downloadUrl()));
    }
}

void OnlineVersionDlg::slotNewVersionCheckError(const QString& error)
{
    d->bar->hide();
    d->stats->hide();
    d->speedTimer->stop();
    d->buttons->button(QDialogButtonBox::Apply)->setVisible(false);
    d->buttons->button(QDialogButtonBox::Cancel)->setText(i18n("Close"));
    d->buttons->button(QDialogButtonBox::Cancel)->setIcon(QIcon::fromTheme(QLatin1String("close")));

    if (error.isEmpty())
    {
        if (d->preRelease)
        {
            d->label->setText(i18n("Your software is up-to-date.\n"
                                   "%1 pre-release built on %2 is the most recent version available.",
                                   qApp->applicationName(),
                                   QLocale().toString(d->curBuildDt, QLocale::ShortFormat)));
        }
        else
        {
            d->label->setText(i18n("Your software is up-to-date.\n"
                                   "%1 stable version %2 is the most recent version available.",
                                   qApp->applicationName(),
                                   QString::fromLatin1(digikam_version_short)));
        }
    }
    else
    {
        d->label->setText(i18n("Error while trying to check for new %1 version:\n\"%2\"",
                               qApp->applicationName(),
                               error));
    }
}

void OnlineVersionDlg::slotDownloadInstaller()
{
    if (d->preRelease)
    {
        QString version = d->updateWithDebug ? i18n("built on %1 with debug symbols", QLocale().toString(d->onlineDt, QLocale::ShortFormat))
                                             : i18n("built on %1", QLocale().toString(d->onlineDt, QLocale::ShortFormat));

        d->label->setText(i18n("Downloading new %1\n%2\nin progress, please wait...",
                               qApp->applicationName(),
                               version));
    }
    else
    {
        QString version = d->updateWithDebug ? i18n("version %1 with debug symbols", d->newVersion)
                                             : i18n("version %1", d->newVersion);

        d->label->setText(i18n("Downloading new %1\nversion %2\nin progress, please wait...",
                               qApp->applicationName(),
                               version));
    }

    d->buttons->button(QDialogButtonBox::Apply)->setEnabled(false);
    d->buttons->button(QDialogButtonBox::Reset)->setEnabled(false);

    d->bar->setMaximum(1);
    d->bar->setMinimum(0);
    d->bar->setValue(0);
    d->bar->show();
    d->stats->show();
    d->speedTimer->start(1000);
    d->dwnlStart = QDateTime::currentDateTime();

    if (d->preRelease)
    {
        d->dwnloader->startDownload(d->checker->preReleaseFileName());
    }
    else
    {
        d->dwnloader->startDownload(d->newVersion);
    }
}

void OnlineVersionDlg::slotComputeChecksum()
{
    if (d->preRelease)
    {
        QString version = d->updateWithDebug ? i18n("built on %1 with debug symbols", QLocale().toString(d->onlineDt, QLocale::ShortFormat))
                                             : i18n("built on %1", QLocale().toString(d->onlineDt, QLocale::ShortFormat));

        d->label->setText(i18n("Verify Checksum for new %1\n%2\nin progress, please wait...",
                               qApp->applicationName(),
                               version));
    }
    else
    {
        QString version = d->updateWithDebug ? i18n("version %1 with debug symbols", d->newVersion)
                                             : i18n("version %1", d->newVersion);

        d->label->setText(i18n("Verify checksum for %1\nversion %2\nin progress, please wait...",
                               qApp->applicationName(),
                               version));
    }

    d->buttons->button(QDialogButtonBox::Apply)->setEnabled(false);
    d->buttons->button(QDialogButtonBox::Reset)->setEnabled(false);

    d->bar->setMaximum(0);
    d->bar->setMinimum(0);
    d->bar->setValue(0);
    d->bar->show();
    d->stats->hide();
    d->speedTimer->stop();
}

void OnlineVersionDlg::slotDownloadError(const QString& error)
{
    d->bar->hide();
    d->stats->hide();
    d->speedTimer->stop();

    if (error.isEmpty())        // empty error want mean a complete download.
    {

#ifdef Q_OS_LINUX

        d->buttons->button(QDialogButtonBox::Apply)->setText(i18nc("@action: open bundle file", "Open..."));
        d->buttons->button(QDialogButtonBox::Apply)->setIcon(QIcon::fromTheme(QLatin1String("system-file-manager")));
        d->buttons->button(QDialogButtonBox::Apply)->setEnabled(true);

        disconnect(d->buttons->button(QDialogButtonBox::Apply), SIGNAL(clicked()), 0, 0);

        connect(d->buttons->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
                this, SLOT(slotOpenInFileManager()));

        if (d->preRelease)
        {
            d->label->setText(i18n("The new %1\npre-release built on %2\nhave been downloaded at:\n\n"
                                   "%3\n\n"
                                   "Press \"Open\" to show the bundle in file-manager...",
                                   qApp->applicationName(),
                                   QLocale().toString(d->onlineDt, QLocale::ShortFormat),
                                   d->dwnloader->downloadedPath()));
        }
        else
        {
            d->label->setText(i18n("The new %1\nstable version %2\nhave been downloaded at:\n\n"
                                   "%3\n\n"
                                   "Press \"Open\" to show the bundle in file-manager...",
                                   qApp->applicationName(),
                                   d->newVersion,
                                   d->dwnloader->downloadedPath()));
        }

#else // Windows and macOS

        d->buttons->button(QDialogButtonBox::Apply)->setEnabled(true);
        d->buttons->button(QDialogButtonBox::Apply)->setText(i18n("Install..."));
        d->buttons->button(QDialogButtonBox::Apply)->setIcon(QIcon::fromTheme(QLatin1String("run-install")));

        if (d->preRelease)
        {
            d->label->setText(i18n("The new pre-release %1\nbuilt on %2\nhave been downloaded at:\n\n"
                                   "%3\n\n"
                                   "Press \"Install\" to close current session and upgrade...",
                                   qApp->applicationName(),
                                   QLocale().toString(d->onlineDt, QLocale::ShortFormat),
                                   d->dwnloader->downloadedPath()));
        }
        else
        {
            d->label->setText(i18n("The new %1\nstable version %2\nhave been downloaded at:\n\n"
                                   "%3\n\n"
                                   "Press \"Install\" to close current session and upgrade...",
                                   qApp->applicationName(),
                                   d->newVersion,
                                   d->dwnloader->downloadedPath()));
        }

        disconnect(d->buttons->button(QDialogButtonBox::Apply), SIGNAL(clicked()), 0, 0);

        connect(d->buttons->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
                this, SLOT(slotRunInstaller()));

#endif

    }
    else
    {
        d->buttons->button(QDialogButtonBox::Apply)->setVisible(false);
        d->buttons->button(QDialogButtonBox::Cancel)->setText(i18n("Close"));
        d->buttons->button(QDialogButtonBox::Cancel)->setIcon(QIcon::fromTheme(QLatin1String("close")));

        if (d->preRelease)
        {
            d->label->setText(i18n("Error while trying to download %1\npre-release built on %2:\n\n\"%3\"",
                                   qApp->applicationName(),
                                   QLocale().toString(d->onlineDt, QLocale::ShortFormat),
                                   error));
        }
        else
        {
            d->label->setText(i18n("Error while trying to download %1\nstable version %2:\n\n\"%3\"",
                                   qApp->applicationName(),
                                   d->newVersion,
                                   error));
        }
    }
}

void OnlineVersionDlg::slotDownloadProgress(qint64 recv, qint64 total)
{
    if (total == 0)
    {
        return;
    }

    d->bar->setMaximum(total);
    d->bar->setMinimum(0);
    d->bar->setValue(recv);
}

void OnlineVersionDlg::slotUpdateStats()
{
    QDateTime now = QDateTime::currentDateTime();
    qint64 diff   = d->dwnlStart.secsTo(now);
    qint64 rate   = 0;
    qint64 remain = 0;

    if (diff)
    {
        rate = d->bar->value() / diff;

        if (rate)
        {
            remain = (d->bar->maximum() - d->bar->value()) / rate;
        }
    }

    QString durationString = QString::fromUtf8("%1s").arg(remain);

    if (remain)
    {
        unsigned int rr, dd, hh, mm, ss;
        rr = qAbs(remain * 1000);
        dd = rr / 86400000;
        rr = rr % 86400000;
        hh = rr / 3600000;
        rr = rr % 3600000;
        mm = rr / 60000;
        rr = rr % 60000;
        ss = rr / 1000;

        durationString = QString().asprintf("%d.%02d:%02d:%02d", dd, hh, mm, ss);
    }

    d->received->setText(ItemPropertiesTab::humanReadableBytesCount(d->bar->value()));
    d->total->setText(ItemPropertiesTab::humanReadableBytesCount(d->bar->maximum()));
    d->rate->setText(QString::fromUtf8("%1/s").arg(ItemPropertiesTab::humanReadableBytesCount(rate)));
    d->remain->setText(durationString);
}

void OnlineVersionDlg::slotRunInstaller()
{

#ifndef Q_OS_LINUX

    bool started = false;
    QString path = d->dwnloader->downloadedPath();
    qCDebug(DIGIKAM_GENERAL_LOG) << "Run installer:" << path;

#   ifdef Q_OS_WIN

    started = QProcess::startDetached(path, QStringList());

#   elif defined Q_OS_MACOS

    QStringList args;
    args << QLatin1String("-e");
    args << QLatin1String("tell application \"Finder\"");
    args << QLatin1String("-e");
    args << QString::fromUtf8("open POSIX file \"%1\"").arg(path);
    args << QLatin1String("-e");
    args << QLatin1String("end tell");
    args << QLatin1String("-e");
    args << QLatin1String("return");

    started = QProcess::startDetached(QLatin1String("/usr/bin/osascript"), args);

#   endif

    if (!started)
    {
        QMessageBox::critical(this, qApp->applicationName(),
                              i18n("Cannot start installer:\n%1", path));
        return;
    }

    // Close this dialog

    close();

    // Now close main window application after 3 secondes to wait Installer to start in background.
    // See bug #437813.

    qCDebug(DIGIKAM_GENERAL_LOG) << "Call to close all main windows";

    QTimer::singleShot(3000, qApp, SLOT(closeAllWindows()));

#endif

}

void OnlineVersionDlg::slotOpenInFileManager()
{
    QList<QUrl> lst = QList<QUrl>() << QUrl::fromLocalFile(d->dwnloader->downloadedPath());
    DFileOperations::openInFileManager(lst);
    close();
}

void OnlineVersionDlg::slotHelp()
{
    openOnlineDocumentation(QLatin1String("setup_application"),
                            QLatin1String("miscs_settings"),
                            QLatin1String("application-updates"));
}

void OnlineVersionDlg::slotSetupUpdate()
{
    close();

    Q_EMIT signalSetupUpdate();
}

} // namespace Digikam

#include "moc_onlineversiondlg.cpp"
