/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : Test for SimpleTreeModel.
 *
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "simpletreemodel_utest.h"

// Qt includes

#include <QUrl>

// Local includes

#include "digikam_debug.h"
#include "simpletreemodel.h"
#include "modeltest.h"

using namespace Digikam;

/**
 * Description : Dummy test that does nothing
 */
void TestSimpleTreeModel::testNoOp()
{
}

void TestSimpleTreeModel::testModel1()
{
    SimpleTreeModel* const treeModel          = new SimpleTreeModel(1, this);

    new ModelTest(treeModel, this);

    Q_ASSERT(!treeModel->index(0, 0).isValid());
    Q_ASSERT(treeModel->indexToItem(QModelIndex()) == treeModel->rootItem());
    Q_ASSERT(!treeModel->itemToIndex(treeModel->rootItem()).isValid());
    Q_ASSERT(!treeModel->itemToIndex(nullptr).isValid());
    Q_ASSERT(!treeModel->parent(QModelIndex()).isValid());

    // ---

    const SimpleTreeModel::Item* const item1  = treeModel->addItem();

    Q_ASSERT(item1                              != nullptr);

    const QPersistentModelIndex item1Index    = treeModel->itemToIndex(item1);

    Q_ASSERT(item1Index.isValid());
    Q_ASSERT(treeModel->indexToItem(item1Index) == item1);
    Q_ASSERT(!treeModel->parent(item1Index).isValid());

    // ---

    SimpleTreeModel::Item* const item2        = treeModel->addItem();

    Q_ASSERT(item2                              != nullptr);

    const QModelIndex item2Index              = treeModel->itemToIndex(item2);

    Q_ASSERT(item2Index.isValid());
    Q_ASSERT(treeModel->indexToItem(item2Index) == item2);
    Q_ASSERT(!treeModel->parent(item2Index).isValid());

    // ---

    const SimpleTreeModel::Item* const item21 = treeModel->addItem(item2);

    Q_ASSERT(item21                              != nullptr);

    const QModelIndex item21Index            = treeModel->itemToIndex(item21);

    Q_ASSERT(item21Index.isValid());
    Q_ASSERT(treeModel->indexToItem(item21Index) == item21);
    Q_ASSERT(treeModel->parent(item21Index)      == item2Index);
    Q_ASSERT(treeModel->index(0, 0, item2Index)  == item21Index);

    // just make sure another modeltest will test things for consistency in case a signal went missing

    new ModelTest(treeModel, this);

    Q_ASSERT(treeModel->rootItem()                                 == treeModel->indexToItem(QModelIndex()));
    Q_ASSERT(treeModel->indexToItem(treeModel->itemToIndex(item1)) == item1);
    Q_ASSERT(treeModel->hasIndex(0, 0)                             == true);

    QModelIndex topIndex                     = treeModel->index(0, 0, QModelIndex());

    if (treeModel->rowCount(topIndex) > 0)
    {
        QModelIndex childIndex = treeModel->index(0, 0, topIndex);
        qCDebug(DIGIKAM_TESTS_LOG) << childIndex;
        qCDebug(DIGIKAM_TESTS_LOG) << treeModel->parent(childIndex);

        Q_ASSERT(treeModel->parent(childIndex) == topIndex);
    }

    // add another few items

    {
        const SimpleTreeModel::Item* const item22 = treeModel->addItem(item2, 0);

        Q_ASSERT(item22                              != nullptr);

        const QModelIndex item22Index             = treeModel->itemToIndex(item22);

        Q_ASSERT(item22Index.isValid());
        Q_ASSERT(treeModel->indexToItem(item22Index) == item22);
        Q_ASSERT(treeModel->parent(item22Index)      == item2Index);
        Q_ASSERT(treeModel->index(0, 0, item2Index)  == item22Index);
        Q_ASSERT(item22Index.row()                   == 0);
    }

    // add another few items

    {
        const SimpleTreeModel::Item* const item23 = treeModel->addItem(item2, 1);

        Q_ASSERT(item23                              != nullptr);

        const QModelIndex item23Index             = treeModel->itemToIndex(item23);

        Q_ASSERT(item23Index.isValid());
        Q_ASSERT(treeModel->indexToItem(item23Index) == item23);
        Q_ASSERT(treeModel->parent(item23Index)      == item2Index);
        Q_ASSERT(treeModel->index(1, 0, item2Index)  == item23Index);
        Q_ASSERT(item23Index.row()                   == 1);
    }

    new ModelTest(treeModel, this);
}

QTEST_GUILESS_MAIN(TestSimpleTreeModel)

#include "moc_simpletreemodel_utest.cpp"
