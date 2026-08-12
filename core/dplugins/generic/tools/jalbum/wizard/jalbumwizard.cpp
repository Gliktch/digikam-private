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

#include "jalbumwizard.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QMenu>
#include <QApplication>
#include <QComboBox>
#include <QListWidget>
#include <QTextBrowser>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>


// Local includes

#include "dwizardpage.h"
#include "digikam_debug.h"
#include "jalbumfinalpage.h"
#include "jalbumsettings.h"
#include "jalbumintropage.h"
#include "jalbumoutputpage.h"
#include "jalbumselectionpage.h"

namespace DigikamGenericJAlbumPlugin
{

class Q_DECL_HIDDEN JAlbumWizard::Private
{
public:

    Private() = default;

public:

    JAlbumSettings*        settings         = nullptr;

    JAlbumIntroPage*       introPage        = nullptr;
    JAlbumSelectionPage*   selectionPage    = nullptr;
    JAlbumOutputPage*      outputPage       = nullptr;
    JAlbumFinalPage*       finalPage        = nullptr;
};

JAlbumWizard::JAlbumWizard(QWidget* const parent, DInfoInterface* const iface)
    : DWizardDlg(parent, QLatin1String("jAlbum Album Creation Dialog")),
      d         (new Private)
{
    setOption(QWizard::NoCancelButtonOnLastPage);
    setWindowTitle(i18nc("@title:window", "Create jAlbum Album"));

    d->settings             = new JAlbumSettings(iface);

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("jAlbum tool"));
    d->settings->readSettings(group);

    d->introPage         = new JAlbumIntroPage(this,         i18n("Welcome to jAlbum Export Tool"));
    d->selectionPage     = new JAlbumSelectionPage(this,     i18n("Items Selection"));
    d->outputPage        = new JAlbumOutputPage(this,        i18n("Paths Selection"));
    d->finalPage         = new JAlbumFinalPage(this,         i18n("Generating jAlbum"));
}

JAlbumWizard::~JAlbumWizard()
{
    delete d;
}

void JAlbumWizard::setItemsList(const QList<QUrl>& urls)
{
    d->selectionPage->setItemsList(urls);
}

bool JAlbumWizard::validateCurrentPage()
{
    if (!DWizardDlg::validateCurrentPage())
    {
        return false;
    }

    if (currentPage() == d->outputPage)
    {
        KSharedConfigPtr config = KSharedConfig::openConfig();
        KConfigGroup group      = config->group(QLatin1String("jAlbum tool"));
        d->settings->writeSettings(group);
    }

    return true;
}

int JAlbumWizard::nextId() const
{
    return DWizardDlg::nextId();
}

JAlbumSettings* JAlbumWizard::settings() const
{
    return d->settings;
}

} // namespace DigikamGenericJAlbumPlugin

#include "moc_jalbumwizard.cpp"
