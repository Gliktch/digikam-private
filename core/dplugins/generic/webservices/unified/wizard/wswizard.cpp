/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to export items to web services.
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "wswizard.h"

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
#include "wsintropage.h"
#include "wsauthenticationpage.h"
#include "wsalbumspage.h"
#include "wsimagespage.h"
#include "wssettingspage.h"
#include "wsfinalpage.h"
#include "wstoolutils.h"

namespace DigikamGenericUnifiedPlugin
{

class Q_DECL_HIDDEN WSWizard::Private
{
public:

    Private() = default;

    DInfoInterface*             iface           = nullptr;

    WSSettings*                 settings        = nullptr;
    WSIntroPage*                introPage       = nullptr;
    WSAuthenticationWizard*     authPage        = nullptr;
    WSAlbumsPage*               albumsPage      = nullptr;
    WSImagesPage*               imagesPage      = nullptr;
    WSSettingsPage*             settingsPage    = nullptr;
    WSFinalPage*                finalPage       = nullptr;

    WSAuthentication*           wsAuth          = nullptr;
};

WSWizard::WSWizard(DInfoInterface* const iface, QWidget* const parent)
    : DWizardDlg(parent, QLatin1String("Web Services Dialog")),
      d         (new Private)
{
    setOptions(QWizard::NoBackButtonOnStartPage | QWizard::NoCancelButtonOnLastPage);
    setWindowTitle(i18nc("@title:window", "Export to Web Services"));

    d->iface                = iface;
    d->settings             = new WSSettings(this);

    d->wsAuth               = new WSAuthentication(this, d->iface);

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("Web Services Dialog Settings"));
    d->settings->readSettings(group);

    d->introPage            = new WSIntroPage(this,    i18n("Welcome to Web Services Tool"));
    d->authPage             = new WSAuthenticationWizard(this, i18n("Authentication dialog"));
    d->albumsPage           = new WSAlbumsPage(this,   i18n("Albums Selection"));
    d->imagesPage           = new WSImagesPage(this,   i18n("Images List"));
    d->settingsPage         = new WSSettingsPage(this, i18n("Web Service Settings"));
    d->finalPage            = new WSFinalPage(this,    i18n("Export by Web Service"));
}

WSWizard::~WSWizard()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group      = config->group(QLatin1String("Web Services Dialog Settings"));
    d->settings->writeSettings(group);

    delete d;
}

void WSWizard::setItemsList(const QList<QUrl>& urls)
{
    d->imagesPage->setItemsList(urls);
}

DInfoInterface* WSWizard::iface() const
{
    return d->iface;
}

WSSettings* WSWizard::settings() const
{
    return d->settings;
}

WSAuthentication* WSWizard::wsAuth() const
{
    return d->wsAuth;
}

QSettings* WSWizard::oauthSettings() const
{
    return d->settings->oauthSettings;
}

O0SettingsStore* WSWizard::oauthSettingsStore() const
{
    return d->settings->oauthSettingsStore;
}

bool WSWizard::validateCurrentPage()
{
    if (!DWizardDlg::validateCurrentPage())
    {
        return false;
    }

    return true;
}

int WSWizard::nextId() const
{
    if (currentPage() == d->authPage)
    {
        if (d->settings->selMode == WSSettings::IMPORT)
        {
            return d->albumsPage->id();
        }
        else
        {
            return d->imagesPage->id();
        }
    }

    return DWizardDlg::nextId();
}

void WSWizard::slotBusy(bool val)
{
    if (val)
    {
        setCursor(Qt::WaitCursor);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
    }
}

} // namespace DigikamGenericUnifiedPlugin

#include "moc_wswizard.cpp"
