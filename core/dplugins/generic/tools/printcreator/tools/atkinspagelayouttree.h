/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-13
 * Description : Layouting photos on a page
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QRectF>
#include <QMap>

/**
 * Implements the algorithm described in
 * "Adaptive Photo Collection Page Layout",
 * C. Brian Atkins
 * Imaging Technology Department
 * HP Labs
 * Palo Alto, CA 94304
 * cbatkins@hpl.hp.com
 *
 * PDF available at this url:
 * hpl.hp.com/research/isl/layout/
 */

/**
 * The classes AtkinsPageLayoutNode and AtkinsPageLayoutTree provide the actual implementation.
 * Do not use these classes directly.
 * Use the AtkinsPageLayout adaptor class.
 */

namespace DigikamGenericPrintCreatorPlugin
{

class AtkinsPageLayoutNode;

class AtkinsPageLayoutTree
{
public:

    AtkinsPageLayoutTree(double aspectRatioPage,
                         double absoluteAreaPage);
    AtkinsPageLayoutTree(const AtkinsPageLayoutTree&);
    ~AtkinsPageLayoutTree();

    AtkinsPageLayoutTree& operator=(const AtkinsPageLayoutTree& other);

    int    addImage(double aspectRatio,
                    double relativeArea);
    QRectF drawingArea(int index,
                       const QRectF& absoluteRectPage);

    int    count() const;
    double score(AtkinsPageLayoutNode* const root,
                 int nodeCount);
    double G() const;

private:

    double absoluteArea(const AtkinsPageLayoutNode* const node);

    /**
     * Lays out a rectangle with given aspect ratio and absolute area inside the given
     * larger rectangle (not in the paper).
     */
    QRectF rectInRect(const QRectF& rect,
                      double aspectRatio,
                      double absoluteArea);

private:

    AtkinsPageLayoutNode* m_root              = nullptr;
    int                   m_count             = 0;

    double                m_aspectRatioPage   = 0.0;
    double                m_absoluteAreaPage  = 0.0;
};

} // Namespace DigikamGenericPrintCreatorPlugin
