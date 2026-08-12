/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2016-09-29
 * Description : migration page from digikam4
 *
 * SPDX-FileCopyrightText: 2016 by Antonio Larrosa <alarrosa at suse dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "migratefromdigikam4page.h"

// Qt includes

#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QStringList>
#include <QVBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>
#include <kdelibs4configmigrator.h>
#include <kdelibs4migration.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dbengineparameters.h"
#include "dlayoutbox.h"

namespace Digikam
{

class Q_DECL_HIDDEN MigrateFromDigikam4Page::Private
{
public:

    Private() = default;

public:

    QButtonGroup* migrateBehavior = nullptr;
    QRadioButton* migrate         = nullptr;
    QRadioButton* createnew       = nullptr;
};

MigrateFromDigikam4Page::MigrateFromDigikam4Page(QWizard* const dlg)
    : DWizardPage(dlg, i18n("Migration from digiKam 4")),
      d          (new Private)
{
    const int spacing        = layoutSpacing();

    DVBox* const vbox        = new DVBox(this);
    QLabel* const title      = new QLabel(vbox);
    title->setWordWrap(true);
    title->setText(i18n("<qt>"
                        "<p><h1><b>Migrate configuration and metadata from digiKam 4</b></h1></p>"
                        "<p>You can choose here if you want to use the configuration and albums from digiKam 4 in new digiKam. "
                        "Please note the following warnings:</p>"
                        "<p>Migration is done <b>at your own risk</b>. digiKam developers "
                        "do not recommend it and do not support it. On the other hand, creating "
                        "a new configuration might result in loss of tags and other metadata that was not embedded inside "
                        "the pictures and was only available in digiKam 4's database.</p>"
                        "<p>In either case you are recommended to backup "
                        "the configuration files and databases before proceeding.</p>"
                        "</qt>"));

    QWidget* const btns      = new QWidget(vbox);
    QVBoxLayout* const vlay  = new QVBoxLayout(btns);

    d->migrateBehavior       = new QButtonGroup(btns);
    d->migrate               = new QRadioButton(btns);
    d->migrate->setText(i18n("Migrate configuration from digiKam 4"));
    d->migrate->setChecked(true);

    connect(d->migrate, SIGNAL(toggled(bool)),
            this, SLOT(migrationToggled(bool)));

    d->migrateBehavior->addButton(d->migrate);

    d->createnew             = new QRadioButton(btns);
    d->createnew->setText(i18n("Create a new configuration"));
    d->migrateBehavior->addButton(d->createnew);

    vlay->addWidget(d->migrate);
    vlay->addWidget(d->createnew);
    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(spacing);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))

    connect(d->migrateBehavior, SIGNAL(idClicked(int)),
            this, SIGNAL(completeChanged()));

#else

    connect(d->migrateBehavior, SIGNAL(buttonClicked(int)),
            this, SIGNAL(completeChanged()));

#endif

    setPageWidget(vbox);
}

MigrateFromDigikam4Page::~MigrateFromDigikam4Page()
{
    delete d;
}

void MigrateFromDigikam4Page::doMigration()
{
    // Migrate digiKam config files from $KDEHOME/share/config/

    Kdelibs4ConfigMigrator migrator(QLatin1String("digikam"));
    QStringList configFiles;
    configFiles << QLatin1String("digikamrc")
                << QLatin1String("digikam_tagsmanagerrc")
                << QLatin1String("showfotorc");
    migrator.setConfigFiles(configFiles);
    migrator.migrate();

    // Migrate digiKam config files from $KDEHOME/share/apps/digikam/

    Kdelibs4Migration migration;
    QString oldappdatadir   = migration.locateLocal("data", QLatin1String("digikam"));
    QStringList oldAppFiles = QDir(oldappdatadir).entryList(QDir::Files | QDir::Readable | QDir::NoDotAndDotDot);

    for (const QString& configFileName : std::as_const(oldAppFiles))
    {
        const QString newConfigLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                                          QLatin1Char('/') + configFileName + QLatin1Char('5');

        if (QFile(newConfigLocation).exists())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << newConfigLocation << " already exists. Skipping";
            continue;
        }

        QFileInfo fileInfo(newConfigLocation);
        QDir().mkpath(fileInfo.absolutePath());

        const QString oldConfigFile = oldappdatadir + QLatin1Char('/') + configFileName;

        if (!oldConfigFile.isEmpty())
        {
            if (QFile(oldConfigFile).copy(newConfigLocation))
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Config file" << oldConfigFile << "was migrated to" << newConfigLocation;
            }
        }
    }

    // Migrate $KDEHOME/share/apps/kipi/geobookmarks.xml to ./.local/share/digikam/geobookmarks.xml

    QString oldGeobookmarksFile       = migration.locateLocal("data", QLatin1String("kipi/geobookmarks.xml"));
    const QString newGeobookmarksFile = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                                        QLatin1String("/geobookmarks.xml");

    if (QFile(newGeobookmarksFile).exists())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << newGeobookmarksFile << " already exists. Skipping";
    }
    else
    {
        QFileInfo fileInfo(newGeobookmarksFile);
        QDir().mkpath(fileInfo.absolutePath());

        if (!oldGeobookmarksFile.isEmpty())
        {
            if (QFile(oldGeobookmarksFile).copy(newGeobookmarksFile))
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Config file" << oldGeobookmarksFile << "was migrated to" << newGeobookmarksFile;
            }
        }
    }

    // Fix albumroot identifier since digiKam 5 doesn't interpret correctly
    // values like volumeid:?path=%2Fhome%2Fantonio%2FPictures and it needs
    // to be url-decoded.

    DbEngineParameters parameters = DbEngineParameters::parametersFromConfig();
    QSqlDatabase databaseHandler  = QSqlDatabase::addDatabase(parameters.databaseType, QLatin1String("digikam4migration"));

    databaseHandler.setHostName(parameters.hostName);
    databaseHandler.setPort(parameters.port);
    databaseHandler.setDatabaseName(parameters.databaseNameCore);
    databaseHandler.setUserName(parameters.userName);
    databaseHandler.setPassword(parameters.password);
    databaseHandler.setConnectOptions(parameters.connectOptions);

    if (!databaseHandler.open())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot open database:" << databaseHandler.lastError().text();
        return;
    }

    QSqlQuery query(QLatin1String("SELECT id,identifier FROM albumroots"), databaseHandler);

    while (query.next())
    {
        int id             = query.value(0).toInt();
        QString identifier = query.value(1).toString();

        if (identifier.startsWith(QLatin1String("volumeid:?path=%2F")))
        {
            QUrl url(identifier);
            url.setQuery(url.query(QUrl::FullyDecoded), QUrl::DecodedMode);
            qCDebug(DIGIKAM_GENERAL_LOG) << "Updating albumroot " << id << " from " << identifier << " to " << url.toString();
            QSqlQuery uquery(QLatin1String("UPDATE albumroots SET identifier=? WHERE id=?"), databaseHandler);
            uquery.bindValue(0, url.toString());
            uquery.bindValue(1, id);
            bool ret = uquery.exec();

            if (!ret)
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot query database for migration:" << databaseHandler.lastError().text();
            }
        }
    }

    databaseHandler.close();

    qCDebug(DIGIKAM_GENERAL_LOG) << "Migration finished";
}

bool MigrateFromDigikam4Page::isMigrationChecked() const
{
    return d->migrate->isChecked();
}

void MigrateFromDigikam4Page::migrationToggled(bool b)
{
    setFinalPage(b);
}

int MigrateFromDigikam4Page::nextId() const
{
    if (d->migrate->isChecked())
    {
        return (-1);
    }
    else
    {
        return QWizardPage::nextId();
    }
}

bool MigrateFromDigikam4Page::checkForMigration()
{

#ifdef Q_OS_LINUX

    ::Kdelibs4Migration migration;

    // If there's a digikamrc file in $KDEHOME/share/config,
    // then we create the migration page in the wizard

    return (!migration.locateLocal("config", QLatin1String("digikamrc")).isEmpty());

#else

    return false;

#endif

}

} // namespace Digikam

#include "moc_migratefromdigikam4page.cpp"
