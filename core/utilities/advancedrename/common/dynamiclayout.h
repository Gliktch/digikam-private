/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-10-22
 * Description : a dynamic layout manager
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QLayout>

class QLayoutItem;
class QRect;
class QWidget;

namespace Digikam
{

class DynamicLayout : public QLayout
{
    Q_OBJECT

public:

    explicit DynamicLayout(QWidget* const parent, int margin = -1,
                           int hSpacing = 0, int vSpacing = 0);
    explicit DynamicLayout(int margin = -1, int hSpacing = 0,
                           int vSpacing = 0);
    ~DynamicLayout()                              override;

    void addItem(QLayoutItem* layItem)            override;

    int  horizontalSpacing()                const;
    int  verticalSpacing()                  const;

    Qt::Orientations expandingDirections()  const override;

    bool hasHeightForWidth()                const override;
    int  heightForWidth(int)                const override;

    int count()                             const override;

    QLayoutItem* itemAt(int index)          const override;
    QLayoutItem* takeAt(int index)                override;

    QSize minimumSize()                     const override;

    void  setGeometry(const QRect& rect)          override;
    QSize sizeHint()                        const override;


private:

    int reLayout(const QRect& rect,
                 bool testOnly)             const;

private:

    /// @note disabled
    DynamicLayout(const DynamicLayout&)            = delete;
    DynamicLayout& operator=(const DynamicLayout&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
