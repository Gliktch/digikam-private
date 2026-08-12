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

#pragma once

// Qt includes

#include <QWidget>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class ApplicationSettings;
class DbEngineParameters;

class DIGIKAM_GUI_EXPORT DatabaseSettingsWidget : public QWidget
{
    Q_OBJECT

public:

    enum DatabaseType
    {
        SQlite          = 0,
        MysqlInternal   = 1,
        MysqlServer     = 2
    };

public:

    explicit DatabaseSettingsWidget(QWidget* const parent = nullptr);
    ~DatabaseSettingsWidget() override;

private:

    void setupMainArea();

private Q_SLOTS:

    void slotUpdateSqlInit();

// --- Properties ----------------------------------------

public:

    void setParametersFromSettings(const ApplicationSettings* const settings,
                                   const bool& migration = false);
    DbEngineParameters getDbEngineParameters()  const;

    void    setDatabaseType(int type);
    int     databaseType()                      const;

    QString databaseBackend()                   const;

    void setDatabasePath(const QString& path);
    QString databasePath()                      const;

    DbEngineParameters orgDatabasePrm()         const;

    /**
     * For Sqlite or MysqlInternal, check properties of local path to store database files.
     * For MysqlServer, check the network connection and database names.
     */
    bool checkDatabaseSettings();

private:

    void handleInternalServer(int index);
    void setDatabaseInputFields(int index);
    bool isNotEqualToThumbName(const QString& name);
    bool checkDatabasePath();

private Q_SLOTS:

    void slotHandleDBTypeIndexChanged(int index);
    void slotDatabasePathEditedDelayed();
    void slotDatabasePathEdited();

// --- Mysql ----------------------------------------------

private:

    bool checkMysqlServerConnection(QString& error);
    bool checkMysqlServerConnectionConfig(QString& error);
    bool checkMysqlServerDbNamesConfig(QString& error);

private Q_SLOTS:

    void slotCheckMysqlServerConnection();
    void slotResetMysqlServerDBNames();

// --------------------------------------------------------

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
