/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : unit test program for digiKam Mysql database init
 *
 * SPDX-FileCopyrightText: 2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTest>
#include <QDir>
#include <QString>

class DatabaseMysqlInitTest : public QObject
{
    Q_OBJECT

public:

    explicit DatabaseMysqlInitTest(QObject* const parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:

    void testMysqlInit();

    void initTestCase();
    void cleanupTestCase();

private:

    QString m_filesPath;              ///< Path to image to import in database.
    QString m_tempPath;               ///< The temporary path to store file to process unit test.
    QDir    m_tempDir;                ///< Same that previous as QDir object.
};
