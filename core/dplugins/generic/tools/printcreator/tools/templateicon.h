/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-26
 * Description : a tool to print images
 *
 * SPDX-FileCopyrightText: 2008      by Andreas Trink <atrink at nociaro dot org>
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPainter>
#include <QIcon>
#include <QColor>
#include <QSize>

namespace DigikamGenericPrintCreatorPlugin
{

class TemplateIcon
{

public:

    /**
     * Constructor: The height of the icon is <iconHeight>. The width is computed
     * during invocation of method 'begin()' according to the paper-size.
     */
    explicit TemplateIcon(int iconHeight, const QSize& templateSize);
    ~TemplateIcon();

    /**
     * Begin painting the icon
     */
    void begin();

    /**
     * End painting the icon
     */
    void end();

    /**
     * Returns a pointer to the icon.
     */
    QIcon& getIcon() const;

    /**
     * Returns the size of the icon.
     */
    QSize& getSize();

    /**
     * Returns the painter.
     */
    QPainter& getPainter() const;

    /**
     * Draw a filled rectangle with color <color> at position <x>/<y> (relative
     * to template-origin) and width <w> and height <h>.
     */
    void fillRect(int x, int y, int w, int h, const QColor& color);

private:

    /// @note disabled
    TemplateIcon(const TemplateIcon&)            = delete;
    TemplateIcon& operator=(const TemplateIcon&) = delete;

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamGenericPrintCreatorPlugin
