/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-12
 * Description : A model to hold information about image tags.
 *
 * SPDX-FileCopyrightText: 2010 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPersistentModelIndex>
#include <QList>

// Local includes

#include "gpsitemcontainer.h"

namespace Digikam
{

class TreeBranch
{
public:

    TreeBranch() = default;

    ~TreeBranch()
    {
        qDeleteAll(oldChildren);
    }

public:

    QPersistentModelIndex sourceIndex;
    TreeBranch*           parent        = nullptr;
    QString               data;
    QString               help;
    Type                  type          = TypeChild;
    QList<TreeBranch*>    oldChildren;
    QList<TreeBranch*>    spacerChildren;
    QList<TreeBranch*>    newChildren;

private:

    /// @note disabled
    TreeBranch(const TreeBranch&)            = delete;
    TreeBranch& operator=(const TreeBranch&) = delete;
};

} // namespace Digikam
