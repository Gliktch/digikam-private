/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015
 * Description : a test for timestamp trigger for re-reading metadata from image
 *
 * SPDX-FileCopyrightText: 2015 by Kristian Karl <kristian dot hermann dot karl at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QTest>

class TimeStampUpdateTest : public QObject
{
    Q_OBJECT

public:

    explicit TimeStampUpdateTest(QObject* const parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:

    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void testRescanImageIfModifiedSet2True();
    void testRescanImageIfModifiedSet2False();

private:

    QString tempFileName(const QString& purpose) const;
    QString tempFilePath(const QString& purpose) const;

private:

    QString          dbFile;
    QList<qlonglong> ids;
};
