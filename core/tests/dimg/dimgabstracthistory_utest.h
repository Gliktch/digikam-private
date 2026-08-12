/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-08-01
 * Description : a test for the DImageHistory
 *
 * SPDX-FileCopyrightText: 2010 by Marcel Wiesweg <user dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QEventLoop>

// Local includes

#include "dimagehistory.h"
#include "editorcore.h"

using namespace Digikam;

QDebug operator<<(QDebug dbg, const HistoryImageId& id);

class DImgAbstractHistoryTest : public QObject
{
    Q_OBJECT

public:

    explicit DImgAbstractHistoryTest(QObject* const parent = nullptr);

    HistoryImageId id1()                         const;
    HistoryImageId id2()                         const;
    HistoryImageId id3()                         const;
    HistoryImageId id4()                         const;

    FilterAction action1()                       const;
    FilterAction action2()                       const;

    DImageHistory history1()                     const;

    void applyFilters1();
    void applyFilters2();
    void applyFilters3();
    void applyFilters4();

    QString tempFileName(const QString& purpose) const;
    QString tempFilePath(const QString& purpose) const;

    static QString imagePath();

public Q_SLOTS:

    virtual void slotImageLoaded(const QString&, bool);
    virtual void slotImageSaved(const QString&, bool);

protected:

    void initBaseTestCase();
    void cleanupBaseTestCase();

protected:

    QEventLoop  m_loop;
    EditorCore* m_im        = nullptr;
    QString     m_tempFile;
};
