/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-02
 * Description : Test the functions for dealing with DatabaseFields
 *
 * SPDX-FileCopyrightText: 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>

class IOJobsTest : public QObject
{
    Q_OBJECT

public:

    explicit IOJobsTest(QObject* const parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:

    void init();
    void cleanup();

    void copyAndMove();
    void copyAndMove_data();

    void permanentDel();
    void permanentDel_data();
};
