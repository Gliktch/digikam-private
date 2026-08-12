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

void DatabaseSettingsWidget::slotResetMysqlServerDBNames()
{
    d->dbNameCore->setText(QLatin1String("digikam"));
    d->dbNameThumbs->setFileDlgPath(QLatin1String("digikam"));
    d->dbNameFace->setText(QLatin1String("digikam"));
    d->dbNameSimilarity->setText(QLatin1String("digikam"));
}

void DatabaseSettingsWidget::slotCheckMysqlServerConnection()
{
    QString error;

    if (checkMysqlServerConnection(error))
    {
        QMessageBox::information(qApp->activeWindow(), i18nc("@title:window", "Database Connection Test"),
                                 i18n("Database connection test successful."));
    }
    else
    {
        QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Database Connection Test"),
                              i18n("Database connection test was not successful. <p>Error was: %1</p>",
                                   error));
    }
}

bool DatabaseSettingsWidget::checkMysqlServerConnectionConfig(QString& error)
{
    if (d->hostName->text().isEmpty())
    {
        error = i18n("The server hostname is empty");
        return false;
    }

    if (d->userName->text().isEmpty())
    {
        error = i18n("The server user name is empty");
        return false;
    }

    return true;
}

bool DatabaseSettingsWidget::checkMysqlServerDbNamesConfig(QString& error)
{
    if (d->dbNameCore->text().isEmpty())
    {
        error = i18n("The core database name is empty");
        return false;
    }

    if (d->dbNameThumbs->fileDlgPath().isEmpty())
    {
        error = i18n("The thumbnails database name is empty");
        return false;
    }

    if (d->dbNameFace->text().isEmpty())
    {
        error = i18n("The face database name is empty");
        return false;
    }

    if (d->dbNameSimilarity->text().isEmpty())
    {
        error = i18n("The similarity database name is empty");
        return false;
    }

    return true;
}

bool DatabaseSettingsWidget::checkMysqlServerConnection(QString& error)
{
    if (!checkMysqlServerConnectionConfig(error))
    {
        return false;
    }

    bool result = false;

    qApp->setOverrideCursor(Qt::WaitCursor);

    QString databaseID(QLatin1String("ConnectionTest"));

    {
        QSqlDatabase testDatabase = QSqlDatabase::addDatabase(databaseBackend(), databaseID);

        DbEngineParameters prm    = getDbEngineParameters();
        qCDebug(DIGIKAM_DATABASE_LOG) << "Testing DB connection (" << databaseID << ") with these settings:";
        qCDebug(DIGIKAM_DATABASE_LOG) << prm;

        testDatabase.setHostName(prm.hostName);
        testDatabase.setPort(prm.port);
        testDatabase.setUserName(prm.userName);
        testDatabase.setPassword(prm.password);
        testDatabase.setConnectOptions(prm.connectOptions);

        result = testDatabase.open();
        error  = testDatabase.lastError().text();
        testDatabase.close();
    }

    QSqlDatabase::removeDatabase(databaseID);

    qApp->restoreOverrideCursor();

    return result;
}

} // namespace Digikam
