/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-02-20
 * Description : Unit tests for TagsCache class
 *
 * SPDX-FileCopyrightText: 2021 by David Haslam <dch dot code at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QTest>
#include <QDir>

/**
 * Unit tests for HaarIface class in core/libs/database/haar/haariface
 *
 */
class HaarIfaceTest : public QObject
{
    Q_OBJECT

public:

    explicit HaarIfaceTest(QObject* const parent = nullptr);
    ~HaarIfaceTest() override;

private Q_SLOTS:

    void initTestCase();
    void cleanupTestCase();
    void testOriginal();
    void testExcludeRefSelectpotentialDuplicates();
    void testPreferFolderSelectpotentialDuplicates();
    void testPreferNewerCreationDate();
    void testPreferNewerModificationDate();
    void testPreferFolderWhole();
    void testReferenceFolderNotSelected();
    void testReferenceFolderPartlySelected();

private:

    void startSqlite(const QDir& dbDir);
    void stopSql();

private:

    QString              filesPath;
};
