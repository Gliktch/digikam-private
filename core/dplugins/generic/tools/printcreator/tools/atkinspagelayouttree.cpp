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

#include "atkinspagelayouttree.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QList>

// Local includes

#include "atkinspagelayoutnode.h"

namespace DigikamGenericPrintCreatorPlugin
{

AtkinsPageLayoutTree::AtkinsPageLayoutTree(double aspectRatioPage,
                                           double absoluteAreaPage)
    : m_aspectRatioPage(aspectRatioPage),
      m_absoluteAreaPage(absoluteAreaPage)
{
}

AtkinsPageLayoutTree::~AtkinsPageLayoutTree()
{
    delete m_root;
}

AtkinsPageLayoutTree::AtkinsPageLayoutTree(const AtkinsPageLayoutTree& other)
{
    (*this) = other;
}

AtkinsPageLayoutTree& AtkinsPageLayoutTree::operator=(const AtkinsPageLayoutTree& other)
{
    if (this != &other)
    {
        delete m_root;
        m_root             = new AtkinsPageLayoutNode(*(other.m_root));
        m_count            = other.m_count;
        m_aspectRatioPage  = other.m_aspectRatioPage;
        m_absoluteAreaPage = other.m_absoluteAreaPage;
    }

    return *this;
}

int AtkinsPageLayoutTree::addImage(double aspectRatio,
                                   double relativeArea)
{
    int index = m_count;

    if (!m_root)
    {
        m_root = new AtkinsPageLayoutNode(aspectRatio,
                                          relativeArea,
                                          index);
        m_count++;

        return index;
    }

    // Section 2.1

    AtkinsPageLayoutNode* bestTree = nullptr;
    double highScore               = 0;

    for (int i = 0 ; i < m_count ; ++i)
    {
        for (int horizontal = 0 ; horizontal < 2 ; ++horizontal)
        {
            // create temporary tree

            AtkinsPageLayoutNode* candidateTree          = new AtkinsPageLayoutNode(*m_root);

            // select the subtree which will be replace by a new internal node

            AtkinsPageLayoutNode* const candidateSubtree = candidateTree->nodeForIndex(i);

            // find parent node

            AtkinsPageLayoutNode* const parentNode       = candidateTree->parentOf(candidateSubtree);

            // create new terminal node

            AtkinsPageLayoutNode* const newTerminalNode  = new AtkinsPageLayoutNode(aspectRatio,
                                                                                    relativeArea,
                                                                                    index);

            // create new internal node

            AtkinsPageLayoutNode* const newInternalNode  = new AtkinsPageLayoutNode(candidateSubtree,
                                                                                    newTerminalNode,
                                                                                    horizontal,
                                                                                    index+1);

            // replace in tree

            if (parentNode)
            {
                // replace in tree

                parentNode->takeAndSetChild(candidateSubtree, newInternalNode);
            }
            else
            {
                // candidateTree is candidateSubtree is root

                candidateTree = newInternalNode;
            }

            // recompute sizes

            candidateTree->computeRelativeSizes();

            double candidateScore = score(candidateTree, m_count+2);

            if (candidateScore > highScore)
            {
                highScore = candidateScore;
                delete bestTree;
                bestTree  = candidateTree;
            }
            else
            {
                delete candidateTree;
            }
        }
    }

    delete m_root;
    m_root = bestTree;

    if (m_root)
    {
        m_root->computeDivisions();
    }

    m_count += 2;

    return index;
}

/// Section 2.2.1
double AtkinsPageLayoutTree::score(AtkinsPageLayoutNode* const root,
                                   int nodeCount)
{
    if (!root)
    {
        return 0;
    }

    double areaSum = 0;

    for (int i = 0 ; i < nodeCount ; ++i)
    {
        const AtkinsPageLayoutNode* const node = root->nodeForIndex(i);

        if (node->type() == AtkinsPageLayoutNode::TerminalNode)
        {
            areaSum += node->relativeArea();
        }
    }

    double minRatioPage = root->aspectRatio() < m_aspectRatioPage ? root->aspectRatio()
                                                                  : m_aspectRatioPage;
    double maxRatioPage = root->aspectRatio() > m_aspectRatioPage ? root->aspectRatio()
                                                                  : m_aspectRatioPage;

    return (G() * (areaSum / root->relativeArea()) * (minRatioPage / maxRatioPage));
}

/// Section 2.2.2
double AtkinsPageLayoutTree::G() const
{
    return (0.95 * 0.95);
}

/// Section 2.2.2
double AtkinsPageLayoutTree::absoluteArea(const AtkinsPageLayoutNode* const node)
{
    // min(a_pbb, a_page), max(a_pbb, a_page)

    double minRatioPage     = m_root->aspectRatio() < m_aspectRatioPage ? m_root->aspectRatio()
                                                                        : m_aspectRatioPage;
    double maxRatioPage     = m_root->aspectRatio() > m_aspectRatioPage ? m_root->aspectRatio()
                                                                        : m_aspectRatioPage;

    // A_pbb

    double absoluteAreaRoot = m_absoluteAreaPage * minRatioPage / maxRatioPage;

    if (node == m_root)
    {
        return absoluteAreaRoot;
    }

    // A_i

    return (G() * node->relativeArea() / m_root->relativeArea() * absoluteAreaRoot);
}

QRectF AtkinsPageLayoutTree::drawingArea(int index, const QRectF& absoluteRectPage)
{
    AtkinsPageLayoutNode* const node = m_root->nodeForIndex(index);

    if (!node)
    {
        return QRectF();
    }

    // find out the "line of ancestry" of the node

    QList<AtkinsPageLayoutNode*> treePath;
    AtkinsPageLayoutNode* parent1 = node;

    while (parent1)
    {
        treePath.prepend(parent1);
        parent1 = m_root->parentOf(parent1);
    }

    // find out the rect of the page bounding box (the rect of the root node in the page rect)

    QRectF absoluteRect = rectInRect(absoluteRectPage,
                                     m_root->aspectRatio(),
                                     absoluteArea(m_root));

    // go along the line of ancestry and narrow down the bounding rectangle,
    // as described in section 2.2.2

    for (int i = 0 ; i < treePath.count() - 1 ; ++i)
    {
        const AtkinsPageLayoutNode* const parent2 = treePath[i];
        const AtkinsPageLayoutNode* const child   = treePath[i+1]; // only iterating to count-1

        if (parent2->type() == AtkinsPageLayoutNode::VerticalDivision) // side by side
        {
            double leftWidth = absoluteRect.width() * parent2->division();

            if (child == parent2->leftChild())
            {
                absoluteRect.setWidth(leftWidth);
            }
            else // right child
            {
                double rightWidth = absoluteRect.width() - leftWidth;
                absoluteRect.setWidth(rightWidth);
                absoluteRect.translate(leftWidth, 0);
            }
        }
        else // horizontal division: one on top of the other
        {
            // left child is topmost

            double upperHeight = absoluteRect.height() * parent2->division();

            if (child == parent2->leftChild())
            {
                absoluteRect.setHeight(upperHeight);
            }
            else // right child
            {
                double lowerHeight = absoluteRect.height() - upperHeight;
                absoluteRect.setHeight(lowerHeight);
                absoluteRect.translate(0, upperHeight);
            }
        }
    }

    return rectInRect(absoluteRect, node->aspectRatio(), absoluteArea(node));
}

QRectF AtkinsPageLayoutTree::rectInRect(const QRectF &rect,
                                        double aspectRatio,
                                        double absoluteArea)
{
    double width  = std::sqrt(absoluteArea / aspectRatio);
    double height = std::sqrt(absoluteArea * aspectRatio);
    double x      = rect.x() + (rect.width()  - width)  / 2;
    double y      = rect.y() + (rect.height() - height) / 2;

    return QRectF(x, y, width, height);
}

} // Namespace DigikamGenericPrintCreatorPlugin
