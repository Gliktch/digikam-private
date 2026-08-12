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

class AtkinsPageLayoutNode
{
public:

    enum Type
    {
        TerminalNode,
        HorizontalDivision, ///< one image on top of the other
        VerticalDivision    ///< images side by side
    };

public:

    AtkinsPageLayoutNode(double aspectRatio,
                         double relativeArea,
                         int index);
    AtkinsPageLayoutNode(AtkinsPageLayoutNode* const subtree,
                         AtkinsPageLayoutNode* const terminalChild,
                         bool horizontal,
                         int index);
    AtkinsPageLayoutNode(const AtkinsPageLayoutNode&);
    ~AtkinsPageLayoutNode();

    AtkinsPageLayoutNode& operator=(const AtkinsPageLayoutNode&);

    double aspectRatio()  const;
    double relativeArea() const;
    double division()     const;
    Type   type()         const;
    int    index()        const;

    AtkinsPageLayoutNode* leftChild()  const;
    AtkinsPageLayoutNode* rightChild() const;

    /**
     * Replace one child with a new one.
     */
    void takeAndSetChild(const AtkinsPageLayoutNode* const oldChild,
                         AtkinsPageLayoutNode* const newChild);

    /**
     * Retrieve the node which has the given index in the hierarchy of this node.
     */
    AtkinsPageLayoutNode* nodeForIndex(int index);

    /**
     * Retrieve the parent node of the given child in the hierarchy of this node.
     */
    AtkinsPageLayoutNode* parentOf(AtkinsPageLayoutNode* const child);

    /**
     * Compute the "aspect ratio" (m_a) and "relative size" (m_e) parameters.
     * Section 2.2.1, (1)-(4).
     */
    void computeRelativeSizes();

    /**
     * Compute the "division" (m_division) parameter.
     * Section 2.2.2
     */
    void computeDivisions();

private:

    double                m_a           = 0.0;        ///< aspect ratio
    double                m_e           = 0.0;        ///< relative size
    double                m_division    = 0.0;

    Type                  m_type        = TerminalNode;

    int                   m_index       = 0;

    AtkinsPageLayoutNode* m_leftChild   = nullptr;
    AtkinsPageLayoutNode* m_rightChild  = nullptr;
};

} // Namespace DigikamGenericPrintCreatorPluginGenericPrintCreatorPlugin
