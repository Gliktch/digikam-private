/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-13
 * Description : Layouting photos on a page
 *
 * SPDX-FileCopyrightText: 2007-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "atkinspagelayout.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QList>

// Local includes

#include "atkinspagelayouttree.h"

namespace DigikamGenericPrintCreatorPlugin
{

class Q_DECL_HIDDEN AtkinsPageLayout::Private
{
public:

    Private() = default;

    QMap<int, int>        indexMap;
    AtkinsPageLayoutTree* tree = nullptr;
    QRectF                pageRect;
};

AtkinsPageLayout::AtkinsPageLayout(const QRectF& pageRect)
    : d(new Private)
{
    d->pageRect = pageRect;
    d->tree     = new AtkinsPageLayoutTree(aspectRatio(d->pageRect.size()),
                                         absoluteArea(d->pageRect.size()));
}

AtkinsPageLayout::~AtkinsPageLayout()
{
    delete d->tree;
    delete d;
}

double AtkinsPageLayout::aspectRatio(const QSizeF& size)
{
    return size.height() / size.width();
}

double AtkinsPageLayout::absoluteArea(const QSizeF &size)
{
    return size.height() * size.width();
}

void AtkinsPageLayout::addLayoutItem(int key, const QSizeF& size)
{
    double relativeArea = absoluteArea(size) / absoluteArea(d->pageRect.size());
    addLayoutItem(key, aspectRatio(size), relativeArea);
}

void AtkinsPageLayout::addLayoutItem(int key, double aspectRatio, double relativeArea)
{
    int index        = d->tree->addImage(aspectRatio, relativeArea);
    d->indexMap[key] = index;
}

QRectF AtkinsPageLayout::itemRect(int key)
{
    QMap<int, int>::iterator it = d->indexMap.find(key);

    if (it != d->indexMap.end())
    {
        // get rect relative to 0,0

        QRectF rect = d->tree->drawingArea(*it, d->pageRect);

        // translate to page rect origin

        rect.translate(d->pageRect.topLeft());

        return rect;
    }

    return QRectF();
}

} // Namespace DigikamGenericPrintCreatorPlugin
