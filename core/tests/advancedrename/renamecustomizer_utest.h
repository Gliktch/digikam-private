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

class RenameCustomizerTest : public QObject
{
    Q_OBJECT

public:

    explicit RenameCustomizerTest(QObject* const parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:

    void newName_should_return_empty_string_with_empty_filename_data();
    void newName_should_return_empty_string_with_empty_filename();

    void setCaseType_set_to_none();
    void setCaseType_set_to_upper();
    void setCaseType_set_to_lower();

    void setUseDefault_true();
    void setUseDefault_false();
    void setUseDefault_case_none_should_deliver_original_filename();
    void setUseDefault_case_upper_should_deliver_uppercase_filename();
    void setUseDefault_case_lower_should_deliver_lowercase_filename();
};
