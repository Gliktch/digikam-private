/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database settings widget
 *
 * SPDX-FileCopyrightText: 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbsettingswidget_p.h"

namespace Digikam
{

int DatabaseSettingsWidget::databaseType() const
{
    return d->dbType->currentData().toInt();
}

QString DatabaseSettingsWidget::databasePath() const
{
    return d->dbPathEdit->fileDlgPath();
}

void DatabaseSettingsWidget::setDatabasePath(const QString& path)
{
    d->dbPathEdit->setFileDlgPath(path);
}

DbEngineParameters DatabaseSettingsWidget::orgDatabasePrm() const
{
    return d->orgPrms;
}

QString DatabaseSettingsWidget::databaseBackend() const
{
    switch (databaseType())
    {
        case MysqlInternal:
        case MysqlServer:
        {
            return DbEngineParameters::MySQLDatabaseType();
        }

        default: // SQlite
        {
            return DbEngineParameters::SQLiteDatabaseType();
        }
    }
}

void DatabaseSettingsWidget::slotHandleDBTypeIndexChanged(int index)
{
    int dbType = d->dbType->itemData(index).toInt();
    setDatabaseInputFields(dbType);
    handleInternalServer(dbType);
    slotUpdateSqlInit();
}

void DatabaseSettingsWidget::setDatabaseInputFields(int index)
{
    switch (index)
    {
        case SQlite:
        {
            d->dbPathBtn->setVisible(true);
            d->dbPathLbl->setVisible(true);
            d->dbPathEdit->setVisible(true);
            d->walModeCheck->setVisible(true);
            d->walModeBtn->setVisible(true);
            d->mysqlCmdBox->setVisible(false);
            d->tab->setVisible(false);

            connect(d->dbPathEdit->lineEdit(), SIGNAL(textChanged(QString)),
                    this, SLOT(slotDatabasePathEditedDelayed()));

            break;
        }

        case MysqlInternal:
        {
            d->dbPathBtn->setVisible(true);
            d->dbPathLbl->setVisible(true);
            d->dbPathEdit->setVisible(true);
            d->walModeCheck->setVisible(false);
            d->walModeBtn->setVisible(false);
            d->mysqlCmdBox->setVisible(true);
            d->tab->setVisible(false);

            connect(d->dbPathEdit->lineEdit(), SIGNAL(textChanged(QString)),
                    this, SLOT(slotDatabasePathEditedDelayed()));

            break;
        }

        default: // MysqlServer
        {
            d->dbPathBtn->setVisible(false);
            d->dbPathLbl->setVisible(false);
            d->dbPathEdit->setVisible(false);
            d->walModeCheck->setVisible(false);
            d->walModeBtn->setVisible(false);
            d->mysqlCmdBox->setVisible(false);
            d->tab->setVisible(true);

            disconnect(d->dbPathEdit->lineEdit(), SIGNAL(textChanged(QString)),
                       this, SLOT(slotDatabasePathEditedDelayed()));
            break;
        }
    }
}

void DatabaseSettingsWidget::handleInternalServer(int index)
{
    bool internal = (index == MysqlInternal);

    d->hostName->setDisabled(internal);
    d->hostPort->setDisabled(internal);
    d->dbNameCore->setDisabled(internal);
    d->dbNameThumbs->setDisabled(internal);
    d->dbNameFace->setDisabled(internal);
    d->dbNameSimilarity->setDisabled(internal);
    d->userName->setDisabled(internal);
    d->password->setDisabled(internal);
    d->connectOpts->setDisabled(internal);
}


void DatabaseSettingsWidget::setParametersFromSettings(const ApplicationSettings* const settings,
                                                       const bool& migration)
{
    d->orgPrms = settings->getDbEngineParameters();

    if      (d->orgPrms.databaseType == DbEngineParameters::SQLiteDatabaseType())
    {
        d->dbPathEdit->setFileDlgPath(d->orgPrms.getCoreDatabaseNameOrDir());
        d->dbType->setCurrentIndex(d->dbTypeMap[SQlite]);
        d->walModeCheck->setChecked(d->orgPrms.walMode);
        slotResetMysqlServerDBNames();

        if (settings->getDatabaseDirSetAtCmd() && !migration)
        {
            d->dbType->setEnabled(false);
            d->dbTypeBtn->setEnabled(false);
            d->dbPathLbl->setEnabled(false);
            d->dbPathEdit->setEnabled(false);
            d->dbPathBtn->setWhatsThis(d->dbPathBtn->whatsThis() +
                                       i18n("<p>This path was set as a command line"
                                            "option (--database-directory).</p>"));
        }
    }

#ifdef HAVE_MYSQLSUPPORT

#   ifdef HAVE_INTERNALMYSQL

    else if ((d->orgPrms.databaseType == DbEngineParameters::MySQLDatabaseType()) && d->orgPrms.internalServer)
    {
        d->dbPathEdit->setFileDlgPath(d->orgPrms.internalServerPath());
        d->dbType->setCurrentIndex(d->dbTypeMap[MysqlInternal]);
        d->mysqlUpgradeBin.setup(QFileInfo(d->orgPrms.internalServerMysqlUpgradeCmd).absolutePath());
        d->mysqlServerBin.setup(QFileInfo(d->orgPrms.internalServerMysqlServerCmd).absolutePath());
        d->mysqlAdminBin.setup(QFileInfo(d->orgPrms.internalServerMysqlAdminCmd).absolutePath());
        d->mysqlInitBin.setup(QFileInfo(d->orgPrms.internalServerMysqlInitCmd).absolutePath());
        d->dbBinariesWidget->allBinariesFound();
        d->walModeCheck->setChecked(false);
        slotResetMysqlServerDBNames();
    }

#   endif

    else
    {
        d->dbType->setCurrentIndex(d->dbTypeMap[MysqlServer]);
        d->dbNameCore->setText(d->orgPrms.databaseNameCore);
        d->dbNameThumbs->setFileDlgPath(d->orgPrms.databaseNameThumbnails);
        d->dbNameFace->setText(d->orgPrms.databaseNameFace);
        d->dbNameSimilarity->setText(d->orgPrms.databaseNameSimilarity);
        d->hostName->setText(d->orgPrms.hostName);
        d->hostPort->setValue((d->orgPrms.port == -1) ? 3306 : d->orgPrms.port);
        d->connectOpts->setText(d->orgPrms.connectOptions);
        d->userName->setText(d->orgPrms.userName);
        d->password->setText(d->orgPrms.password);
        d->walModeCheck->setChecked(false);
    }

#endif

    slotHandleDBTypeIndexChanged(d->dbType->currentIndex());
}

DbEngineParameters DatabaseSettingsWidget::getDbEngineParameters() const
{
    DbEngineParameters prm;

    switch (databaseType())
    {
        case SQlite:
        {
            prm         = DbEngineParameters::parametersForSQLiteDefaultFile(databasePath());
            prm.walMode = d->walModeCheck->isChecked();
            break;
        }

        case MysqlInternal:
        {
            prm = DbEngineParameters::defaultParameters(databaseBackend());
            prm.setInternalServerPath(databasePath());
            prm.internalServerMysqlUpgradeCmd = d->mysqlUpgradeBin.path();
            prm.internalServerMysqlServerCmd  = d->mysqlServerBin.path();
            prm.internalServerMysqlAdminCmd   = d->mysqlAdminBin.path();
            prm.internalServerMysqlInitCmd    = d->mysqlInitBin.path();
            break;
        }

        default: // MysqlServer
        {
            prm.internalServer         = false;
            prm.databaseType           = databaseBackend();
            prm.databaseNameCore       = d->dbNameCore->text();
            prm.databaseNameThumbnails = d->dbNameThumbs->fileDlgPath();
            prm.databaseNameFace       = d->dbNameFace->text();
            prm.databaseNameSimilarity = d->dbNameSimilarity->text();
            prm.connectOptions         = d->connectOpts->text();
            prm.hostName               = d->hostName->text();
            prm.port                   = d->hostPort->value();
            prm.userName               = d->userName->text();
            prm.password               = d->password->text();
            break;
        }
    }

    return prm;
}

void DatabaseSettingsWidget::slotDatabasePathEditedDelayed()
{
    QTimer::singleShot(300, this, SLOT(slotDatabasePathEdited()));
}

void DatabaseSettingsWidget::slotDatabasePathEdited()
{
    QString newPath = databasePath();

#ifndef Q_OS_WIN

    if (!newPath.isEmpty() && !QDir::isAbsolutePath(newPath))
    {
        d->dbPathEdit->setFileDlgPath(QDir::homePath() + QLatin1Char('/') + newPath);
    }

#endif

    d->dbPathEdit->setFileDlgPath(newPath);
}

bool DatabaseSettingsWidget::checkDatabaseSettings()
{
    switch (databaseType())
    {
        case SQlite:
        {
            return checkDatabasePath();
        }

        case MysqlInternal:
        {
            if (!checkDatabasePath())
            {
                return false;
            }

            if (!d->dbBinariesWidget->allBinariesFound())
            {
                return false;
            }

            return true;
        }

        default:  // MysqlServer
        {
            QString error;

            if (!checkMysqlServerDbNamesConfig(error))
            {
                QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Database Configuration"),
                                      i18n("The database names configuration is not valid. Error is <br><p>%1</p><br>"
                                           "Please check your configuration.",
                                           error));
                return false;
            }

            if (!checkMysqlServerConnection(error))
            {
                QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Database Connection Test"),
                                      i18n("Testing database connection has failed with error<br><p>%1</p><br>"
                                           "Please check your configuration.",
                                           error));
                return false;
            }
        }
    }

    return true;
}

bool DatabaseSettingsWidget::checkDatabasePath()
{
    QString dbFolder = databasePath();
    qCDebug(DIGIKAM_DATABASE_LOG) << "Database directory is : " << dbFolder;

    if (dbFolder.isEmpty())
    {
        QMessageBox::information(qApp->activeWindow(), qApp->applicationName(),
                                i18n("You must select a folder for digiKam to "
                                     "store information and metadata in a database file."));
        return false;
    }

    QDir targetPath(dbFolder);

    if (!targetPath.exists())
    {
        int rc = QMessageBox::question(qApp->activeWindow(), i18nc("@title:window", "Create Database Folder?"),
                                    i18n("<p>The folder to put your database in does not seem to exist:</p>"
                                         "<p><b>%1</b></p>"
                                         "Would you like digiKam to create it for you?", dbFolder));

        if (rc == QMessageBox::No)
        {
            return false;
        }

        if (!targetPath.mkpath(dbFolder))
        {
            QMessageBox::information(qApp->activeWindow(), i18nc("@title:window", "Create Database Folder Failed"),
                                    i18n("<p>digiKam could not create the folder to host your database file.<br>"
                                         "Please select a different location.</p>"
                                         "<p><b>%1</b></p>", dbFolder));
            return false;
        }
    }

    QFileInfo path(dbFolder);

#ifdef Q_OS_WIN

    // Work around bug #189168

    QTemporaryFile temp;
    temp.setFileTemplate(path.filePath() + QLatin1String("/XXXXXX"));

    if (!temp.open())

#else

    if (!path.isWritable())

#endif

    {
        QMessageBox::information(qApp->activeWindow(), i18nc("@title:window", "No Database Write Access"),
                                 i18n("<p>You do not seem to have write access "
                                      "for the folder to host the database file.<br>"
                                      "Please select a different location.</p>"
                                      "<p><b>%1</b></p>", dbFolder));
        return false;
    }

    return true;
}

bool DatabaseSettingsWidget::isNotEqualToThumbName(const QString& name)
{
    QFileInfo thumbDB(d->dbNameThumbs->fileDlgPath());
    bool isDir = (thumbDB.exists() && thumbDB.isDir() && thumbDB.isAbsolute());

    return (!isDir && (d->dbNameThumbs->fileDlgPath() != name));
}

} // namespace Digikam
