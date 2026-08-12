/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-09
 * Description : a test for the AdvancedRename utility
 *
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

// Local includes

#include "advancedrenamemanager.h"
#include "defaultrenameparser.h"
#include "parsesettings.h"

class AdvancedRenameTest : public QObject
{
    Q_OBJECT

public:

    explicit AdvancedRenameTest(QObject* const parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:

    /**
     * TOKEN TESTS
     */
    void testFileNameToken();

    void testFileExtensionToken();
    void testFileExtensionToken_data();

    void testFileOwnerToken();

    void testFileGroupToken();

    void testDirectoryNameToken();
    void testDirectoryNameToken_data();

    void testNumberToken();
    void testNumberToken_data();

    /**
     * MODIFIER TESTS
     */
/*
    void testUniqueModifier();
*/
    void testReplaceModifier();
    void testReplaceModifier_data();

    void testRangeModifier();
    void testRangeModifier_data();

    void testDefaultValueModifier();
    void testDefaultValueModifier_data();

    void testUppercaseModifier();

    void testLowercaseModifier();

    void testFirstLetterOfEachWordUppercaseModifier_data();
    void testFirstLetterOfEachWordUppercaseModifier();

    void testChainedModifiers();
    void testChainedModifiers_data();

    void testUniqueModifier();

    /**
     * MANAGER TESTS
     */
    void addFiles_should_only_add_files();
    void addFiles_should_only_add_files2();
    void reset_removes_everything();
    void parseFiles_does_nothing_without_assigned_widget();
    void setStartIndex_invalid_index();
    void setStartIndex_sequencenumber_no_custom_start();
    void setStartIndex_sequencenumber_with_custom_start();

    void sequencenumber_tests_data();
    void sequencenumber_tests();
    void sequencenumber_tests_startIndex_data();
    void sequencenumber_tests_startIndex();

    void newFileList_tests_data();
    void newFileList_tests();

    void indexOfFile_sorting_data();
    void indexOfFile_sorting();
    void indexOfFile_invalid_input_returns_minus_one();
    void indexOfFolder_invalid_input_returns_minus_one();
    void indexOfFileGroup_invalid_input_returns_minus_one();

    void sortAction_custom_asc_should_not_sort();
    void sortAction_custom_desc_should_not_sort();
    void sortAction_name_asc();
    void sortAction_name_desc();
    void sortAction_size_asc();
    void sortAction_size_desc();
/*
    void sortAction_date_asc();
    void sortAction_date_desc();
*/

    /**
     * OTHER TESTS
     */
    void testEmptyParseString();
};
