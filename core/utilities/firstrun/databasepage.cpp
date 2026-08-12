/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * SPDX-FileCopyrightText: 2009-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "databasepage.h"

// Qt includes

#include <QApplication>
#include <QStyle>
#include <QLabel>
#include <QVBoxLayout>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "dbsettingswidget.h"

namespace Digikam
{

class Q_DECL_HIDDEN DatabasePage::Private
{
public:

    Private() = default;

    DatabaseSettingsWidget* dbsettingswidget = nullptr;
};

DatabasePage::DatabasePage(QWizard* const dlg)
    : DWizardPage(dlg, i18n("<b>Configure where you will store databases</b>")),
      d          (new Private)
{
    d->dbsettingswidget = new DatabaseSettingsWidget(this);

    setPageWidget(d->dbsettingswidget);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("network-server-database")));
}

DatabasePage::~DatabasePage()
{
    delete d;
}

void DatabasePage::setDatabasePath(const QString& path)
{
    d->dbsettingswidget->setDatabasePath(path);
}

DbEngineParameters DatabasePage::getDbEngineParameters() const
{
    return d->dbsettingswidget->getDbEngineParameters();
}

void DatabasePage::saveSettings()
{
    DbEngineParameters params = d->dbsettingswidget->getDbEngineParameters();
    params.writeToConfig();

    KSharedConfig::openConfig()->sync();
}

bool DatabasePage::checkSettings()
{
    // TODO : add checks for Mysql Server.

    return d->dbsettingswidget->checkDatabaseSettings();
}

} // namespace Digikam

#include "moc_databasepage.cpp"
