/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-03-22
 * Description : a widget to manage sidebar in GUI - Multi-tab bar tab implementation.
 *
 * SPDX-FileCopyrightText: 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2001-2003 by Joseph Wenninger <jowenn at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "sidebar_p.h"

namespace Digikam
{

DMultiTabBarTab::DMultiTabBarTab(const QIcon& pic, const QString& text,
                                       int id, QWidget* const parent,
                                       Qt::Edge pos,
                                       DMultiTabBar::TextStyle style)
    : DMultiTabBarButton(pic, text, id, parent),
      d                 (new Private)
{
    d->style    = style;
    d->position = pos;
    setToolTip(text);
    setCheckable(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

     // shrink down to icon only, but prefer to show text if it's there
}

DMultiTabBarTab::~DMultiTabBarTab()
{
    delete d;
}

void DMultiTabBarTab::setPosition(Qt::Edge pos)
{
    d->position = pos;
    updateGeometry();
}

void DMultiTabBarTab::setStyle(DMultiTabBar::TextStyle style)
{
    d->style = style;
    updateGeometry();
}

QPixmap DMultiTabBarTab::iconPixmap() const
{
    int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, this);

    return icon().pixmap(iconSize);
}

void DMultiTabBarTab::initButtonStyleOption(QStyleOptionToolButton* opt) const
{
    opt->initFrom(this);

    // Setup icon

    if (!icon().isNull())
    {
        opt->iconSize = iconSize();
        opt->icon     = icon();
    }

    // Should we draw text?

    if (shouldDrawText())
    {
        opt->text = text();
    }

    opt->state |= QStyle::State_AutoRaise;

    if (!isChecked() && !isDown())
    {
        opt->state |= QStyle::State_Raised;
    }

    if (isDown())
    {
        opt->state |= QStyle::State_Sunken;
    }

    if (isChecked())
    {
        opt->state |= QStyle::State_On;
    }

    opt->font            = font();
    opt->toolButtonStyle = shouldDrawText() ? Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly;
    opt->subControls     = QStyle::SC_ToolButton;
}

QSize DMultiTabBarTab::sizeHint() const
{
    return computeSizeHint(shouldDrawText());
}

QSize DMultiTabBarTab::minimumSizeHint() const
{
    return computeSizeHint(false);
}

void DMultiTabBarTab::computeMargins(int* hMargin, int* vMargin) const
{
    // Unfortunately, QStyle does not give us enough information to figure out
    // where to place things, so we try to reverse-engineer it

    QStyleOptionToolButton opt;
    initButtonStyleOption(&opt);

    QPixmap iconPix  = iconPixmap();
    QSize trialSize  = iconPix.size();
    QSize expandSize = style()->sizeFromContents(QStyle::CT_ToolButton, &opt, trialSize, this);

    *hMargin         = (expandSize.width()  - trialSize.width())/2;
    *vMargin         = (expandSize.height() - trialSize.height())/2;
}

QSize DMultiTabBarTab::computeSizeHint(bool withText) const
{
    // Compute as horizontal first, then flip around if need be.

    QStyleOptionToolButton opt;
    initButtonStyleOption(&opt);

    int hMargin, vMargin;
    computeMargins(&hMargin, &vMargin);

    // Compute interior size, starting from pixmap..

    QPixmap iconPix = iconPixmap();
    QSize size      = iconPix.size();

    // Always include text height in computation, to avoid resizing the minor direction
    // when expanding text..

    QSize textSize  = fontMetrics().size(0, text());
    size.setHeight(qMax(size.height(), textSize.height()));

    // Pick margins for major/minor direction, depending on orientation

    int majorMargin = isVertical() ? vMargin : hMargin;
    int minorMargin = isVertical() ? hMargin : vMargin;

    size.setWidth (size.width()  + 2*majorMargin);
    size.setHeight(size.height() + 2*minorMargin);

    if (withText)
    {
        // Add enough room for the text, and an extra major margin.

        size.setWidth(size.width() + textSize.width() + majorMargin);
    }

    if (isVertical())
    {
        return QSize(size.height(), size.width());
    }

    return size;
}

void DMultiTabBarTab::setState(bool newState)
{
    setChecked(newState);
    updateGeometry();
}

void DMultiTabBarTab::setIcon(const QString& icon)
{
    setIcon(QIcon::fromTheme(icon));
}

void DMultiTabBarTab::setIcon(const QIcon& icon)
{
    QPushButton::setIcon(icon);
}

bool DMultiTabBarTab::shouldDrawText() const
{
    return ((d->style == DMultiTabBar::AllIconsText) || isChecked());
}

bool DMultiTabBarTab::isVertical() const
{
    return ((d->position == Qt::RightEdge) || (d->position == Qt::LeftEdge));
}

void DMultiTabBarTab::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    QStyleOptionToolButton opt;
    initButtonStyleOption(&opt);

    // Paint bevel..

    if (underMouse() || isChecked())
    {
        opt.text.clear();
        opt.icon = QIcon();
        style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &painter, this);
    }

    int hMargin, vMargin;
    computeMargins(&hMargin, &vMargin);

    // We first figure out how much room we have for the text, based on
    // icon size and margin, try to fit in by eliding, and perhaps
    // give up on drawing the text entirely if we're too short on room

    QPixmap icon = iconPixmap();
    int textRoom = 0;
    int iconRoom = 0;

    QString t;

    if (shouldDrawText())
    {
        if (isVertical())
        {
            iconRoom = icon.height() + 2*vMargin;
            textRoom = height() - iconRoom - vMargin;
        }
        else
        {
            iconRoom = icon.width() + 2*hMargin;
            textRoom = width() - iconRoom - hMargin;
        }

        t = painter.fontMetrics().elidedText(text(), Qt::ElideRight, textRoom);

        // See whether anything is left. Qt will return either
        // ... or the ellipsis unicode character, 0x2026

        if ((t == QLatin1String("...")) || (t == QString(QChar(0x2026))))
        {
            t.clear();
        }
    }

    // Label time.... Simple case: no text, so just plop down the icon right in the center
    // We only do this when the button never draws the text, to avoid jumps in icon position
    // when resizing

    if (!shouldDrawText())
    {
        style()->drawItemPixmap(&painter, rect(), Qt::AlignCenter | Qt::AlignVCenter, icon);
        return;
    }

    // Now where the icon/text goes depends on text direction and tab position

    QRect iconArea;
    QRect labelArea;

    bool bottomIcon = false;
    bool rtl        = layoutDirection() == Qt::RightToLeft;

    if (isVertical())
    {
        if ((d->position == Qt::LeftEdge) && !rtl)
        {
            bottomIcon = true;
        }

        if ((d->position == Qt::RightEdge) && rtl)
        {
           bottomIcon = true;
        }

        if (bottomIcon)
        {
            labelArea = QRect(0, vMargin, width(), textRoom);
            iconArea  = QRect(0, vMargin + textRoom, width(), iconRoom);
        }
        else
        {
            labelArea = QRect(0, iconRoom, width(), textRoom);
            iconArea  = QRect(0, 0, width(), iconRoom);
        }
    }
    else
    {
        // Pretty simple --- depends only on RTL/LTR

        if (rtl)
        {
            labelArea = QRect(hMargin, 0, textRoom, height());
            iconArea  = QRect(hMargin + textRoom, 0, iconRoom, height());
        }
        else
        {
            labelArea = QRect(iconRoom, 0, textRoom, height());
            iconArea  = QRect(0, 0, iconRoom, height());
        }
    }

    style()->drawItemPixmap(&painter, iconArea, Qt::AlignCenter | Qt::AlignVCenter, icon);

    if (t.isEmpty())
    {
        return;
    }

    QRect labelPaintArea = labelArea;

    if (isVertical())
    {
        // If we're vertical, we paint to a simple 0,0 origin rect,
        // and get the transformations to get us in the right place

        labelPaintArea = QRect(0, 0, labelArea.height(), labelArea.width());

        QTransform tr;

        if (bottomIcon)
        {
            tr.translate(labelArea.x(), labelPaintArea.width() + labelArea.y());
            tr.rotate(-90);
        }
        else
        {
            tr.translate(labelPaintArea.height() + labelArea.x(), labelArea.y());
            tr.rotate(90);
        }

        painter.setTransform(tr);
    }

    style()->drawItemText(&painter, labelPaintArea, Qt::AlignLeading | Qt::AlignVCenter,
                          palette(), true, t, QPalette::ButtonText);
}

} // namespace Digikam
