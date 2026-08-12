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

#include <QWidget>
#include <QPixmap>

// Local includes

#include "digikam_export.h"
#include "dcolorchoosermode.h"

namespace Digikam
{

class DIGIKAM_EXPORT DPointSelect : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int xValue READ xValue WRITE setXValue)
    Q_PROPERTY(int yValue READ yValue WRITE setYValue)

public:

    /**
     * Constructs a two-dimensional selector widget which
     * has a value range of [0..100] in both directions.
     */
    explicit DPointSelect(QWidget* const parent);
    ~DPointSelect()                         override;

    /**
     * Sets the current values in horizontal and
     * vertical direction.
     * @param xPos the horizontal value
     * @param yPos the vertical value
     */
    void setValues(int xPos, int yPos);

    /**
     * Sets the current horizontal value
     * @param xPos the horizontal value
     */
    void setXValue(int xPos);

    /**
     * Sets the current vertical value
     * @param yPos the vertical value
     */
    void setYValue(int yPos);

    /**
     * Sets the range of possible values.
     */
    void setRange(int minX, int minY, int maxX, int maxY);

    /**
     * Sets the color used to draw the marker
     * @param col the color
     */
    void setMarkerColor(const QColor& col);

    /**
     * @return the current value in horizontal direction.
     */
    int xValue()                    const;

    /**
     * @return the current value in vertical direction.
     */
    int yValue()                    const;

    /**
     * @return the rectangle on which subclasses should draw.
     */
    QRect contentsRect()            const;

    /**
     * Reimplemented to give the widget a minimum size
     */
    QSize minimumSizeHint() const override;

Q_SIGNALS:

    /**
     * This signal is emitted whenever the user chooses a value,
     * e.g. by clicking with the mouse on the widget.
     */
    void valueChanged(int x, int y);

protected:

    /**
     * Override this function to draw the contents of the widget.
     * The default implementation does nothing.
     *
     * Draw within contentsRect() only.
     */
    virtual void drawContents(QPainter*)
    {
    };

    /**
     * Override this function to draw the marker which
     * indicates the currently selected value pair.
     */
    virtual void drawMarker(QPainter* p, int xp, int yp);

    void paintEvent(QPaintEvent* e)         override;
    void mousePressEvent(QMouseEvent* e)    override;
    void mouseMoveEvent(QMouseEvent* e)     override;
    void wheelEvent(QWheelEvent*)           override;

    /**
     * Converts a pixel position to its corresponding values.
     */
    void valuesFromPosition(int x, int y, int& xVal, int& yVal) const;

private:

    void setPosition(int xp, int yp);

private:

    DPointSelect(); /// @note disabled default constructor.
    Q_DISABLE_COPY(DPointSelect)

    class Private;
    friend class Private;
    Private* const d = nullptr;
};

// --------------------------------------------------------------------------------

class DIGIKAM_EXPORT DHueSaturationSelector : public DPointSelect
{
    Q_OBJECT

public:

    /**
     * Constructs a hue/saturation selection widget.
     */
    explicit DHueSaturationSelector(QWidget* const parent = nullptr);

    /**
     * Destructor.
     */
    ~DHueSaturationSelector()                   override;

    /**
     * Sets the chooser mode. The allowed modes are defined
     * in DColorChooserMode.
     *
     * @param chooserMode The chooser mode as defined in DColorChooserMode
     */
    void setChooserMode(DColorChooserMode chooserMode);

    /**
     * Returns the chooser mode.
     *
     * @return The chooser mode (defined in DColorChooserMode)
     */
    DColorChooserMode chooserMode() const;


    /**
     * Returns the hue value
     *
     * @return   The hue value (0-360)
     */
    int hue()                       const;

    /**
     * Sets the hue value (0-360)
     *
     * @param  hue  The hue value (0-360)
     */
    void setHue(int hue);

    /**
     * Returns the saturation (0-255)
     *
     * @return The saturation (0-255)
     */
    int saturation()                const;

    /**
     * Sets the saturation (0-255)
     *
     * @param  saturation   The saturation (0-255)
     */
    void setSaturation(int saturation);

    /**
     * Returns the color value (also known as luminosity, 0-255)
     *
     * @return  The color value (0-255)
     */
    int colorValue()                const;

    /**
     * Sets the color value (0-255)
     *
     * @param color The color value (0-255)
     */
    void setColorValue(int color);

    /**
     * Updates the contents
     */
    void updateContents();

protected:

    /**
     * Draws the contents of the widget on a pixmap,
     * which is used for buffering.
     */
    virtual void drawPalette(QPixmap* pixmap);
    void resizeEvent(QResizeEvent*)             override;

    /**
     * Reimplemented from DPointSelect. This drawing is
     * buffered in a pixmap here. As real drawing
     * routine, drawPalette() is used.
     */
    void drawContents(QPainter* painter)        override;

private:

    Q_DISABLE_COPY(DHueSaturationSelector)

    class Private;
    friend class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
