/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 1997-02-20
 * Description : color chooser widgets
 *
 * SPDX-FileCopyrightText:      1997 by Martin Jones <mjones at kde dot org>
 * SPDX-FileCopyrightText: 2015-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPixmap>
#include <QAbstractSlider>

// Local includes

#include "dcolorchoosermode.h"
#include "digikam_export.h"

namespace Digikam
{

/**
 * DSelector is the base class for other widgets which
 * provides the ability to choose from a one-dimensional
 * range of values. An example is the KGradientSelector
 * which allows to choose from a range of colors.
 *
 * A custom drawing routine for the widget surface has
 * to be provided by the subclass.
 */
class DIGIKAM_EXPORT DSelector : public QAbstractSlider
{
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue)
    Q_PROPERTY(int minValue READ minimum WRITE setMinimum)
    Q_PROPERTY(int maxValue READ maximum WRITE setMaximum)
    Q_PROPERTY(bool indent READ indent WRITE setIndent)
    Q_PROPERTY(Qt::ArrowType arrowDirection READ arrowDirection WRITE setArrowDirection)

public:

    explicit DSelector(QWidget* const parent = nullptr);
    explicit DSelector(Qt::Orientation o, QWidget* const parent = nullptr);
    ~DSelector()                            override;

    /**
     * @return the rectangle on which subclasses should draw.
     */
    QRect contentsRect()            const;

    /**
     * Sets the indent option of the widget to i.
     * This determines whether a shaded frame is drawn.
     */
    void setIndent(bool i);

    /**
     * @return whether the indent option is set.
     */
    bool indent()                   const;

    /**
     * Sets the arrow direction.
     */
    void setArrowDirection(Qt::ArrowType direction);

    /**
     * @return the current arrow direction
     */
    Qt::ArrowType arrowDirection()  const;

protected:

    /**
     * Override this function to draw the contents of the control.
     * The default implementation does nothing.
     *
     * Draw only within contentsRect().
     */
    virtual void drawContents(QPainter*)
    {
    };

    /**
     * Override this function to draw the cursor which
     * indicates the current value.
     */
    virtual void drawArrow(QPainter* painter, const QPoint& pos);

    void paintEvent(QPaintEvent*)           override;
    void mousePressEvent(QMouseEvent* e)    override;
    void mouseMoveEvent(QMouseEvent* e)     override;
    void mouseReleaseEvent(QMouseEvent* e)  override;
    void wheelEvent(QWheelEvent*)           override;

private:

    QPoint calcArrowPos(int val);
    void moveArrow(const QPoint& pos);

private:

    class Private;
    friend class Private;
    Private* const d = nullptr;

    Q_DISABLE_COPY(DSelector)
};

// -----------------------------------------------------------------------------------

class DIGIKAM_EXPORT DColorValueSelector : public DSelector
{
    Q_OBJECT
    Q_PROPERTY( int hue        READ hue        WRITE setHue)
    Q_PROPERTY( int saturation READ saturation WRITE setSaturation)
    Q_PROPERTY( int colorValue READ colorValue WRITE setColorValue)

public:

    explicit DColorValueSelector(QWidget* const parent = nullptr);
    explicit DColorValueSelector(Qt::Orientation o, QWidget* const parent = nullptr);
    ~DColorValueSelector()          override;

    /**
     * Updates the widget's contents.
     */
    void updateContents();

    /**
     * Returns the current hue value.
     *
     * @return The hue value (0-359)
     */
    int hue()                       const;

    /**
     * Sets the hue value. Doesn't automatically update the widget;
     * you have to call updateContents manually.
     *
     * @param hue Sets the hue value (0-359)
     */
    void setHue(int hue);

    /**
     * Returns the current saturation value.
     *
     * @return The saturation value (0-255)
     */
    int saturation()                const;

    /**
     * Sets the saturation value. Doesn't automatically update the widget;
     * you have to call updateContents manually.
     *
     * @param saturation Sets the saturation value (0-255)
     */
    void setSaturation(int saturation);

    /**
     * Returns the current color value.
     *
     * @return The color value (0-255)
     */
    int colorValue()                const;

    /**
     * Sets the color value. Doesn't automatically update the widget;
     * you have to call updateContents manually.
     *
     * @param colorValue Sets the color value (0-255)
     */
    void setColorValue(int colorValue);

    /**
     * Sets the chooser mode. Doesn't automatically update the widget;
     * you have to call updateContents manually.
     *
     * @param chooserMode Sets the chooser mode (one of the DColorChooserMode constants)
     */
    void setChooserMode(DColorChooserMode chooserMode);

    /**
     * Returns the current chooser mode.
     *
     * @return The chooser mode (one of the DColorChooserMode constants)
     */
    DColorChooserMode chooserMode() const;

protected:

    /**
     * Draws the contents of the widget on a pixmap,
     * which is used for buffering.
     */
    virtual void drawPalette(QPixmap*);
    void resizeEvent(QResizeEvent*) override;

    /**
     * Reimplemented from DSelector. The drawing is
     * buffered in a pixmap here. As real drawing
     * routine, drawPalette() is used.
     */
    void drawContents(QPainter*)    override;

private:

    class Private;
    friend class Private;
    Private* const d = nullptr;

    Q_DISABLE_COPY(DColorValueSelector)
};

} // namespace Digikam
