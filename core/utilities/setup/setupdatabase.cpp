/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database setup tab
 *
 * SPDX-FileCopyrightText: 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupdatabase.h"

// Qt includes

#include <QApplication>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QHelpEvent>
#include <QGroupBox>
#include <QCursor>
#include <QStyle>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "albummanager.h"
#include "applicationsettings.h"
#include "coredbschemaupdater.h"
#include "databaseserverstarter.h"
#include "dbengineparameters.h"
#include "dbsettingswidget.h"
#include "digikam_debug.h"
#include "scancontroller.h"

namespace Digikam
{

class Q_DECL_HIDDEN SetupDatabase::Private
{
public:

    Private() = default;

public:

    DatabaseSettingsWidget* databaseWidget  = nullptr;
    QGroupBox*              updateBox       = nullptr;
    QPushButton*            hashesButton    = nullptr;
};

SetupDatabase::SetupDatabase(QWidget* const parent)
    : QScrollArea(parent),
      d          (new Private)
{
    const int spacing    = layoutSpacing();


    QWidget* const panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    // --------------------------------------------------------

    QVBoxLayout* const layout = new QVBoxLayout(panel);
    d->databaseWidget         = new DatabaseSettingsWidget(panel);
    layout->addWidget(d->databaseWidget);

    if (!CoreDbSchemaUpdater::isUniqueHashUpToDate())
    {
        createUpdateBox();
        layout->addStretch(10);
        layout->addWidget(d->updateBox);
    }

    layout->setContentsMargins(spacing, spacing, spacing, spacing);
    layout->setSpacing(spacing);

    // --------------------------------------------------------

    readSettings();
    adjustSize();
}

SetupDatabase::~SetupDatabase()
{
    delete d;
}

void SetupDatabase::applySettings()
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    if (d->databaseWidget->getDbEngineParameters() == d->databaseWidget->orgDatabasePrm())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "No DB settings changes. Do nothing...";
        return;
    }

    if (!d->databaseWidget->checkDatabaseSettings())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "DB settings check invalid. Do nothing...";
        return;
    }

    switch (d->databaseWidget->databaseType())
    {
        case DatabaseSettingsWidget::SQlite:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Switch to SQlite DB config...";
            DbEngineParameters params = d->databaseWidget->getDbEngineParameters();
            settings->setDbEngineParameters(params);
            settings->saveSettings();
            AlbumManager::instance()->changeDatabase(params);
            break;
        }

        case DatabaseSettingsWidget::MysqlInternal:
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Switch to Mysql Internal DB config...";
            DbEngineParameters params = d->databaseWidget->getDbEngineParameters();
            settings->setDbEngineParameters(params);
            settings->saveSettings();
            AlbumManager::instance()->changeDatabase(params);
            break;
        }

        default: // DatabaseSettingsWidget::MysqlServer
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Switch to Mysql server DB config...";
            DbEngineParameters params = d->databaseWidget->getDbEngineParameters();
            settings->setDbEngineParameters(params);
            settings->saveSettings();
            AlbumManager::instance()->changeDatabase(params);
            break;
        }
    }
}

void SetupDatabase::readSettings()
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    d->databaseWidget->setParametersFromSettings(settings);
}

void SetupDatabase::slotUpgradeUniqueHashes()
{
    int result = QMessageBox::warning(this, qApp->applicationName(),
                                      i18nc("@info",
                                            "The process of updating the file hashes takes a few minutes or hours.\n"
                                            "Please ensure that any important collections on removable media are connected.\n"
                                            "After the upgrade you cannot use your database with a digiKam version prior to 8.5.0.\n"
                                            "Do you want to begin the update?"),
                                            QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes)
    {
        ScanController::instance()->updateUniqueHash();
    }
}

void SetupDatabase::createUpdateBox()
{
    d->hashesButton                 = new QPushButton(i18nc("@action:button", "Update File Hashes"));

    QPushButton* const infoBtn      = new QPushButton;
    infoBtn->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));
    infoBtn->setToolTip(i18nc("@info:tooltip", "Get information about <b>Update File Hashes</b>"));
    infoBtn->setWhatsThis(i18nc("@info:tooltip",
                                "File hashes are used to identify identical files and to display thumbnails. "
                                "A new, improved algorithm to create the hash is now used. "
                                "The old algorithm, though, still works quite well, so it is recommended to "
                                "carry out this upgrade, but not required.\n"
                                "After the upgrade you cannot use your database with a digiKam version "
                                "prior to 8.5.0."));

    connect(infoBtn, &QPushButton::clicked,
            this, [infoBtn]()
        {
            qApp->postEvent(infoBtn, new QHelpEvent(QEvent::WhatsThis, QPoint(0, 0), QCursor::pos()));
        }
    );

    QGridLayout* const grid = new QGridLayout;
    grid->addWidget(d->hashesButton, 0, 0);
    grid->addWidget(infoBtn,         0, 2);
    grid->setColumnStretch(1, 10);

    d->updateBox                    = new QGroupBox(i18nc("@title:group", "Updates"));
    d->updateBox->setLayout(grid);

    connect(d->hashesButton, SIGNAL(clicked()),
            this, SLOT(slotUpgradeUniqueHashes()));
}

} // namespace Digikam

#include "moc_setupdatabase.cpp"
