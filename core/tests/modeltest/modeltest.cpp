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

#include "modeltest.h"

// Qt includes

#include <QVariant>
#include <QTest>

// Local includes

#include "digikam_debug.h"

#undef Q_ASSERT
#define Q_ASSERT  QVERIFY

Q_DECLARE_METATYPE(QModelIndex)

/**
 * Connect to all of the models signals.  Whenever anything happens recheck everything.
 */
ModelTest::ModelTest(QAbstractItemModel* const _model, QObject* const parent)
    : QObject     (parent),
      model       (_model),
      fetchingMore(false)
{
    Q_ASSERT(model);

    connect(model, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(runAllTests()));

    connect(model, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(runAllTests()));

    connect(model, SIGNAL(columnsInserted(QModelIndex,int,int)),
            this, SLOT(runAllTests()));

    connect(model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
            this, SLOT(runAllTests()));

    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(runAllTests()));

    connect(model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
            this, SLOT(runAllTests()));

    connect(model, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(runAllTests()));

    connect(model, SIGNAL(layoutChanged()),
            this, SLOT(runAllTests()));

    connect(model, SIGNAL(modelReset()),
            this, SLOT(runAllTests()));

    connect(model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(runAllTests()));

    connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(runAllTests()));

    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(runAllTests()));

    connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(runAllTests()));

    // Special checks for inserting/removing

    connect(model, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(layoutAboutToBeChanged()));

    connect(model, SIGNAL(layoutChanged()),
            this, SLOT(layoutChanged()));

    connect(model, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(rowsAboutToBeInserted(QModelIndex,int,int)));

    connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));

    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(rowsInserted(QModelIndex,int,int)));

    connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(rowsRemoved(QModelIndex,int,int)));

    runAllTests();
}

void ModelTest::runAllTests()
{
    if (fetchingMore)
    {
        return;
    }

    nonDestructiveBasicTest();
    rowCount();
    columnCount();
    hasIndex();
    index();
    parent();
    data();
}

/**
 * nonDestructiveBasicTest tries to call a number of the basic functions (not all)
 * to make sure the model doesn't outright segfault, testing the functions that makes sense.
 */
void ModelTest::nonDestructiveBasicTest()
{
    Q_ASSERT(model->buddy(QModelIndex()) == QModelIndex());

    model->canFetchMore(QModelIndex());

    Q_ASSERT(model->columnCount(QModelIndex()) >= 0);
    Q_ASSERT(model->data(QModelIndex()) == QVariant());

    fetchingMore = true;
    model->fetchMore(QModelIndex());
    fetchingMore = false;
    Qt::ItemFlags flags = model->flags(QModelIndex());

    Q_ASSERT((flags == Qt::ItemIsDropEnabled) || (flags == 0));

    model->hasChildren(QModelIndex());
    model->hasIndex(0, 0);
    model->headerData(0, Qt::Horizontal);
    model->index(0, 0);
    model->itemData(QModelIndex());
    QVariant cache;
    model->match(QModelIndex(), -1, cache);
    model->mimeTypes();

    Q_ASSERT(model->parent(QModelIndex()) == QModelIndex());
    Q_ASSERT(model->rowCount() >= 0);

    QVariant variant;
    model->setData(QModelIndex(), variant, -1);
    model->setHeaderData(-1, Qt::Horizontal, QVariant());
    model->setHeaderData(999999, Qt::Horizontal, QVariant());
    model->sibling(0, 0, QModelIndex());
    model->span(QModelIndex());
    model->supportedDropActions();
}

/**
 * Tests model's implementation of QAbstractItemModel::rowCount() and hasChildren()
 * Models that are dynamically populated are not as fully tested here.
 */
void ModelTest::rowCount()
{
//     qCDebug(DIGIKAM_TESTS_LOG) << "rc";

    // check top row

    QModelIndex topIndex         = model->index(0, 0, QModelIndex());
    int rows                     = model->rowCount(topIndex);

    Q_ASSERT(rows >= 0);

    if (rows > 0)
    {
        Q_ASSERT(model->hasChildren(topIndex) == true);
    }

    QModelIndex secondLevelIndex = model->index(0, 0, topIndex);

    if (secondLevelIndex.isValid())
    {
        // not the top level
        // check a row count where parent is valid

        rows = model->rowCount(secondLevelIndex);

        Q_ASSERT(rows >= 0);

        if (rows > 0)
        {
            Q_ASSERT(model->hasChildren(secondLevelIndex) == true);
        }
    }

    // The models rowCount() is tested more extensively in checkChildren(),
    // but this catches the big mistakes
}

/**
 * Tests model's implementation of QAbstractItemModel::columnCount() and hasChildren()
 */
void ModelTest::columnCount()
{
    // check top row

    QModelIndex topIndex = model->index(0, 0, QModelIndex());

    Q_ASSERT(model->columnCount(topIndex) >= 0);

    // check a column count where parent is valid

    QModelIndex childIndex = model->index(0, 0, topIndex);

    if (childIndex.isValid())
    {
        Q_ASSERT(model->columnCount(childIndex) >= 0);
    }

    // columnCount() is tested more extensively in checkChildren(),
    // but this catches the big mistakes
}

/**
 * Tests model's implementation of QAbstractItemModel::hasIndex()
 */
void ModelTest::hasIndex()
{
//     qCDebug(DIGIKAM_TESTS_LOG) << "hi";

    // Make sure that invalid values returns an invalid index

    Q_ASSERT(model->hasIndex(-2, -2) == false);
    Q_ASSERT(model->hasIndex(-2, 0) == false);
    Q_ASSERT(model->hasIndex(0, -2) == false);

    int rows    = model->rowCount();
    int columns = model->columnCount();

    // check out of bounds

    Q_ASSERT(model->hasIndex(rows, columns) == false);
    Q_ASSERT(model->hasIndex(rows + 1, columns + 1) == false);

    if (rows > 0)
    {
        Q_ASSERT(model->hasIndex(0, 0) == true);
    }

    // hasIndex() is tested more extensively in checkChildren(),
    // but this catches the big mistakes
}

/**
 * Tests model's implementation of QAbstractItemModel::index()
 */
void ModelTest::index()
{
//     qCDebug(DIGIKAM_TESTS_LOG) << "i";

    // Make sure that invalid values returns an invalid index

    Q_ASSERT(model->index(-2, -2) == QModelIndex());
    Q_ASSERT(model->index(-2, 0) == QModelIndex());
    Q_ASSERT(model->index(0, -2) == QModelIndex());

    int rows    = model->rowCount();
    int columns = model->columnCount();

    if (rows == 0)
    {
        return;
    }

    // Catch off by one errors

    Q_ASSERT(model->index(rows, columns) == QModelIndex());
    Q_ASSERT(model->index(0, 0).isValid() == true);

    // Make sure that the same index is *always* returned

    QModelIndex a = model->index(0, 0);
    QModelIndex b = model->index(0, 0);

    Q_ASSERT(a == b);

    // index() is tested more extensively in checkChildren(),
    // but this catches the big mistakes
}

/**
 * Tests model's implementation of QAbstractItemModel::parent()
 */
void ModelTest::parent()
{
//     qCDebug(DIGIKAM_TESTS_LOG) << "p";

    // Make sure the model wont crash and will return an invalid QModelIndex
    // when asked for the parent of an invalid index.

    Q_ASSERT(model->parent(QModelIndex()) == QModelIndex());

    if (model->rowCount() == 0)
    {
        return;
    }

    // Column 0                | Column 1    |
    // QModelIndex()           |             |
    //    \- topIndex          | topIndex1   |
    //         \- childIndex   | childIndex1 |

    // Common error test #1, make sure that a top level index has a parent
    // that is a invalid QModelIndex.

    QModelIndex topIndex = model->index(0, 0, QModelIndex());

    Q_ASSERT(model->parent(topIndex) == QModelIndex());

    // Common error test #2, make sure that a second level index has a parent
    // that is the first level index.

    if (model->rowCount(topIndex) > 0)
    {
        QModelIndex childIndex = model->index(0, 0, topIndex);

        Q_ASSERT(model->parent(childIndex) == topIndex);
    }

    // Common error test #3, the second column should NOT have the same children
    // as the first column in a row.
    // Usually the second column shouldn't have children.

    QModelIndex topIndex1 = model->index(0, 1, QModelIndex());

    if (model->rowCount(topIndex1) > 0)
    {
        QModelIndex childIndex  = model->index(0, 0, topIndex);
        QModelIndex childIndex1 = model->index(0, 0, topIndex1);

        Q_ASSERT(childIndex != childIndex1);
    }

    // Full test, walk n levels deep through the model making sure that all
    // parent's children correctly specify their parent.

    checkChildren(QModelIndex());
}

/**
 * Called from the parent() test.
 *
 * A model that returns an index of parent X should also return X when asking
 * for the parent of the index.
 *
 * This recursive function does pretty extensive testing on the whole model in an
 * effort to catch edge cases.
 *
 * This function assumes that rowCount(), columnCount() and index() already work.
 * If they have a bug it will point it out, but the above tests should have already
 * found the basic bugs because it is easier to figure out the problem in
 * those tests then this one.
 */
void ModelTest::checkChildren(const QModelIndex& parent, int currentDepth)
{
    // First just try walking back up the tree.

    QModelIndex p = parent;

    while(p.isValid())
    {
        p = p.parent();
    }

    // For models that are dynamically populated

    if (model->canFetchMore(parent))
    {
        fetchingMore = true;
        model->fetchMore(parent);
        fetchingMore = false;
    }

    int rows    = model->rowCount(parent);
    int columns = model->columnCount(parent);

    if (rows > 0)
    {
        Q_ASSERT(model->hasChildren(parent));
    }

    // Some further testing against rows(), columns(), and hasChildren()

    Q_ASSERT(rows >= 0);
    Q_ASSERT(columns >= 0);

    if (rows > 0)
    {
        Q_ASSERT(model->hasChildren(parent) == true);
    }

    //qCDebug(DIGIKAM_TESTS_LOG) << "parent:" << model->data(parent).toString() << "rows:" << rows
    //         << "columns:" << columns << "parent column:" << parent.column();

    Q_ASSERT(model->hasIndex(rows + 1, 0, parent) == false);

    for (int r = 0 ; r < rows ; ++r)
    {
        if (model->canFetchMore(parent))
        {
            fetchingMore = true;
            model->fetchMore(parent);
            fetchingMore = false;
        }

        Q_ASSERT(model->hasIndex(r, columns + 1, parent) == false);

        for (int c = 0 ; c < columns ; ++c)
        {
            Q_ASSERT(model->hasIndex(r, c, parent) == true);

            QModelIndex ind = model->index(r, c, parent);

            // rowCount() and columnCount() said that it existed...

            Q_ASSERT(ind.isValid() == true);

            // index() should always return the same index when called twice in a row

            QModelIndex modifiedIndex = model->index(r, c, parent);

            Q_ASSERT(ind == modifiedIndex);

            // Make sure we get the same index if we request it twice in a row

            QModelIndex a = model->index(r, c, parent);
            QModelIndex b = model->index(r, c, parent);

            Q_ASSERT(a == b);

            // Some basic checking on the index that is returned

            Q_ASSERT(ind.model() == model);
            Q_ASSERT(ind.row() == r);
            Q_ASSERT(ind.column() == c);

            // While you can technically return a QVariant usually this is a sign
            // of an bug in data()  Disable if this really is ok in your model.

//            Q_ASSERT(model->data(ind, Qt::DisplayRole).isValid() == true);

            // If the next test fails here is some somewhat useful debug you play with.

            if (model->parent(ind) != parent)
            {
                qCDebug(DIGIKAM_TESTS_LOG) << r << c << currentDepth << model->data(ind).toString()
                                           << model->data(parent).toString();

                qCDebug(DIGIKAM_TESTS_LOG) << ind << parent << model->parent(ind);

//                 And a view that you can even use to show the model.
//                 QTreeView view;
//                 view.setModel(model);
//                 view.show();
            }

            // Check that we can get back our real parent.

//             qCDebug(DIGIKAM_TESTS_LOG) << model->parent(ind) << parent ;

            Q_ASSERT(model->parent(ind) == parent);

            // recursively go down the children

            if (model->hasChildren(ind) && (currentDepth < 10))
            {
                //qCDebug(DIGIKAM_TESTS_LOG) << r << c << "has children" << model->rowCount(ind);

                checkChildren(ind, ++currentDepth);
            }
/*
 *          else { if (currentDepth >= 10) qCDebug(DIGIKAM_TESTS_LOG) << "checked 10 deep"; };
*/

            // make sure that after testing the children that the index doesn't change.

            QModelIndex newerIndex = model->index(r, c, parent);

            Q_ASSERT(ind == newerIndex);
        }
    }
}

/**
 * Tests model's implementation of QAbstractItemModel::data()
 */
void ModelTest::data()
{
    // Invalid index should return an invalid qvariant

    Q_ASSERT(!model->data(QModelIndex()).isValid());

    if (model->rowCount() == 0)
    {
        return;
    }

    // A valid index should have a valid QVariant data

    Q_ASSERT(model->index(0, 0).isValid());

    // shouldn't be able to set data on an invalid index

    Q_ASSERT(model->setData(QModelIndex(), QLatin1String("foo"), Qt::DisplayRole) == false);

    // General Purpose roles that should return a QString

    QVariant variant = model->data(model->index(0, 0), Qt::ToolTipRole);

    if (variant.isValid())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        Q_ASSERT(variant.canConvert(QMetaType(QMetaType::QString)));

#else

        Q_ASSERT(variant.canConvert(QMetaType::QString));

#endif

    }

    variant = model->data(model->index(0, 0), Qt::StatusTipRole);

    if (variant.isValid())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        Q_ASSERT(variant.canConvert(QMetaType(QMetaType::QString)));

#else

        Q_ASSERT(variant.canConvert(QMetaType::QString));

#endif

    }

    variant = model->data(model->index(0, 0), Qt::WhatsThisRole);

    if (variant.isValid())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        Q_ASSERT(variant.canConvert(QMetaType(QMetaType::QString)));

#else

        Q_ASSERT(variant.canConvert(QMetaType::QString));

#endif

    }

    // General Purpose roles that should return a QSize

    variant = model->data(model->index(0, 0), Qt::SizeHintRole);

    if (variant.isValid())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        Q_ASSERT(variant.canConvert(QMetaType(QMetaType::QSize)));

#else

        Q_ASSERT(variant.canConvert(QMetaType::QSize));

#endif

    }

    // General Purpose roles that should return a QFont

    QVariant fontVariant = model->data(model->index(0, 0), Qt::FontRole);

    if (fontVariant.isValid())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        Q_ASSERT(fontVariant.canConvert(QMetaType(QMetaType::QFont)));

#else

        Q_ASSERT(fontVariant.canConvert(QMetaType::QFont));

#endif

    }

    // Check that the alignment is one we know about

    QVariant textAlignmentVariant = model->data(model->index(0, 0), Qt::TextAlignmentRole);

    if (textAlignmentVariant.isValid())
    {
        unsigned int alignment = textAlignmentVariant.toInt();

        Q_ASSERT(alignment ==(alignment &(Qt::AlignHorizontal_Mask | Qt::AlignVertical_Mask)));
    }

    // General Purpose roles that should return a QColor

    QVariant colorVariant = model->data(model->index(0, 0), Qt::BackgroundRole);

    if (colorVariant.isValid())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        Q_ASSERT(colorVariant.canConvert(QMetaType(QMetaType::QColor)));

#else

        Q_ASSERT(colorVariant.canConvert(QMetaType::QColor));

#endif

    }

    colorVariant = model->data(model->index(0, 0), Qt::ForegroundRole);

    if (colorVariant.isValid())
    {

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))

        Q_ASSERT(colorVariant.canConvert(QMetaType(QMetaType::QColor)));

#else

        Q_ASSERT(colorVariant.canConvert(QMetaType::QColor));

#endif

    }

    // Check that the "check state" is one we know about.

    QVariant checkStateVariant = model->data(model->index(0, 0), Qt::CheckStateRole);

    if (checkStateVariant.isValid())
    {
        int state = checkStateVariant.toInt();

        Q_ASSERT(
                 (state == Qt::Unchecked)        ||
                 (state == Qt::PartiallyChecked) ||
                 (state == Qt::Checked)
                );
    }
}

/**
 * Store what is about to be inserted to make sure it actually happens
 *
 * \sa rowsInserted()
 */
void ModelTest::rowsAboutToBeInserted(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(end);

//    qCDebug(DIGIKAM_TESTS_LOG) << "rowsAboutToBeInserted" << "start=" << start << "end=" << end << "parent=" << model->data(parent).toString()
//    << "current count of parent=" << model->rowCount(parent); // << "display of last=" << model->data( model->index(start-1, 0, parent));
//     qCDebug(DIGIKAM_TESTS_LOG) << model->index(start-1, 0, parent) << model->data( model->index(start-1, 0, parent));

    Changing c;
    c.parent  = parent;
    c.oldSize = model->rowCount(parent);
    c.last    = model->data(model->index(start - 1, 0, parent));
    c.next    = model->data(model->index(start, 0, parent));
    insert.push(c);
}

/**
 * Confirm that what was said was going to happen actually did
 *
 * \sa rowsAboutToBeInserted()
 */
void ModelTest::rowsInserted(const QModelIndex& parent, int start, int end)
{
    Changing c = insert.pop();

    Q_ASSERT(c.parent == parent);

//    qCDebug(DIGIKAM_TESTS_LOG) << "rowsInserted"  << "start=" << start << "end=" << end << "oldsize=" << c.oldSize
//    << "parent=" << model->data(parent).toString() << "current rowcount of parent=" << model->rowCount(parent);

//    for (int ii=start; ii <= end; ii++)
//    {
//      qCDebug(DIGIKAM_TESTS_LOG) << "itemWasInserted:" << ii << model->data(model->index(ii, 0, parent));
//    }
//    qCDebug(DIGIKAM_TESTS_LOG);

    Q_ASSERT(c.oldSize +(end - start + 1) == model->rowCount(parent));
    Q_ASSERT(c.last == model->data(model->index(start - 1, 0, c.parent)));

    if (c.next != model->data(model->index(end + 1, 0, c.parent)))
    {
        qCDebug(DIGIKAM_TESTS_LOG) << start << end;

        for (int i = 0 ; i < model->rowCount() ; ++i)
        {
            qCDebug(DIGIKAM_TESTS_LOG) << model->index(i, 0).data().toString();
        }

        qCDebug(DIGIKAM_TESTS_LOG) << c.next << model->data(model->index(end + 1, 0, c.parent));
    }

    Q_ASSERT(c.next == model->data(model->index(end + 1, 0, c.parent)));
}

void ModelTest::layoutAboutToBeChanged()
{
    for (int i = 0 ; i < qBound(0, model->rowCount(), 100) ; ++i)
    {
        changing.append(QPersistentModelIndex(model->index(i, 0)));
    }
}

void ModelTest::layoutChanged()
{
    for (int i = 0 ; i < changing.count() ; ++i)
    {
        QPersistentModelIndex p = changing[i];

        Q_ASSERT(p == model->index(p.row(), p.column(), p.parent()));
    }

    changing.clear();
}

/**
 * Store what is about to be inserted to make sure it actually happens
 *
 * \sa rowsRemoved()
 */
void ModelTest::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "ratbr" << parent << start << end;
    Changing c;
    c.parent  = parent;
    c.oldSize = model->rowCount(parent);
    c.last    = model->data(model->index(start - 1, 0, parent));
    c.next    = model->data(model->index(end   + 1, 0, parent));
    remove.push(c);
}

/**
 * Confirm that what was said was going to happen actually did
 *
 * \sa rowsAboutToBeRemoved()
 */
void ModelTest::rowsRemoved(const QModelIndex& parent, int start, int end)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "rr" << parent << start << end;
    Changing c = remove.pop();

    Q_ASSERT(c.parent == parent);
    Q_ASSERT(c.oldSize -(end - start + 1) == model->rowCount(parent));
    Q_ASSERT(c.last == model->data(model->index(start - 1, 0, c.parent)));
    Q_ASSERT(c.next == model->data(model->index(start, 0, c.parent)));
}

#include "moc_modeltest.cpp"
