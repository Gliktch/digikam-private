/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-02-20
 * Description : Unit tests for TagsCache class
 *
 * SPDX-FileCopyrightText: 2021 by David Haslam, <dch dot code at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QTest>

// Local includes

#include "tagscache.h"

/**
 * Unit tests for TagsCache class in core/libs/database/tags/tagscache.h
 *
 * TagsCache provides cached access to the hierarchical tag data in the core database.
 * These tests use the public interface to create sample tag hierarchies and then
 * check the access and relationship to the nodes.
 *
 * Uses a temporary in-memory sqlite database, and does not require a GUI.
 */
class TagsCacheTest : public QObject
{
    Q_OBJECT

public:

    explicit TagsCacheTest(QObject* const parent = nullptr);
    ~TagsCacheTest() override;

private Q_SLOTS:

    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testSimpleHierarchy();
    void testComplexHierarchy();
    void testRepeatedNames();
    void testDuplicateTop();


private:

    // utilities

    int countTags();

    // debug tests

    void dumpTables();
    void dumpTags();

private:

    Digikam::TagsCache* tagsCache = nullptr;
};
