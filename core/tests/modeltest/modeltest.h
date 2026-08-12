/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-07-29
 * Description : This file is part of the test suite of the Qt Toolkit.
 *
 * SPDX-FileCopyrightText: Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QObject>
#include <QAbstractItemModel>
#include <QStack>

class ModelTest : public QObject
{
    Q_OBJECT

public:

    explicit ModelTest(QAbstractItemModel* const model, QObject* const parent = nullptr);

private Q_SLOTS:

    void nonDestructiveBasicTest();
    void rowCount();
    void columnCount();
    void hasIndex();
    void index();
    void parent();
    void data();

protected Q_SLOTS:

    void runAllTests();
    void layoutAboutToBeChanged();
    void layoutChanged();
    void rowsAboutToBeInserted(const QModelIndex& parent, int start, int end);
    void rowsInserted(const QModelIndex& parent, int start, int end);
    void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void rowsRemoved(const QModelIndex& parent, int start, int end);

private:

    void checkChildren(const QModelIndex& parent, int currentDepth = 0);

private:

    struct Changing
    {
        QModelIndex parent;
        int         oldSize = 0;
        QVariant    last;
        QVariant    next;
    };

private:

    QAbstractItemModel*          model          = nullptr;

    QStack<Changing>             insert;
    QStack<Changing>             remove;

    bool                         fetchingMore   = false;

    QList<QPersistentModelIndex> changing;
};
