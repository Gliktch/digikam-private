/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2026-05-18
 * Description : a unit-test to verify that GPS coordinates written
 *               to a video XMP sidecar are updated in the database.
 *
 * SPDX-FileCopyrightText: 2026 by Kristian Karl <kristian dot hermann dot karl at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTest>

class DatabaseVideoGpsTest : public QObject
{
    Q_OBJECT

public:

    explicit DatabaseVideoGpsTest(QObject* const parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:

    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void testVideoGpsSidecarToDatabase();

private:

    QString tempFileName(const QString& purpose) const;
    QString tempFilePath(const QString& purpose) const;

private:

    QString          dbFile;
    QString          collectionPath;
    QString          videoFilePath;
    QString          sidecarFilePath;
    QList<qlonglong> ids;
};
