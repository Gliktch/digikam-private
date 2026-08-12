/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : unit test to switch digiKam database from sqlite to mysql
 *
 * SPDX-FileCopyrightText: 2010-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTest>
#include <QDir>
#include <QString>

class DatabaseSwitchTest : public QObject
{
    Q_OBJECT

public:

    explicit DatabaseSwitchTest(QObject* const parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:

    void testFromSqliteToMysql();
    void testFromMysqlToSqlite();

    void initTestCase();
    void cleanupTestCase();

private:

    void startStopSqlite(const QDir& dbDir);
    void startStopMysql(const QDir& dbDir);

private:

    QString m_filesPath;              ///< Path to images to import in database.
    QString m_sqlitePath;             ///< Path to sqlite database files.
    QString m_db1Path;                ///< Temporary path to original database files.
    QDir    m_db1Dir;                 ///< Same that previous as QDir object.
    QString m_db2Path;                ///< Temporary path to target database files.
    QDir    m_db2Dir;                 ///< Same that previous as QDir object.
};
