/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-02-14
 * Description : a widget to insert a text over an image.
 *
 * SPDX-FileCopyrightText: 2005-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QColor>
#include <QFont>
#include <QImage>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPixmap>
#include <QRect>
#include <QResizeEvent>
#include <QSize>
#include <QString>
#include <QWidget>
#include <QUrl>

// Local includes

#include "dimg.h"
#include "imageiface.h"

using namespace Digikam;

namespace DigikamEditorInsertTextToolPlugin
{

class InsertTextWidget : public QWidget
{
    Q_OBJECT

public:

    enum Action
    {
        ALIGN_LEFT = 0,
        ALIGN_RIGHT,
        ALIGN_CENTER,
        ALIGN_BLOCK,
        BORDER_TEXT,
        TRANSPARENT_TEXT
    };

    enum TextRotation
    {
        ROTATION_NONE = 0,
        ROTATION_90,
        ROTATION_180,
        ROTATION_270
    };

    enum BorderMode
    {
        BORDER_NONE = 0,
        BORDER_SUPPORT,
        BORDER_NORMAL
    };

public:

    explicit InsertTextWidget(int w, int h, QWidget* const parent = nullptr);
    ~InsertTextWidget()                       override;

    ImageIface* imageIface()            const;
    DImg        makeInsertText();

    /** Set background color for pixmap
     */
    void   setBackgroundColor(const QColor& bg);

    void   setText(const QString& text, const QFont& font, const QColor& color, int opacity,
                   int alignMode, bool border, bool transparent, int rotation);

    void   resetEdit();

    void  setPositionHint(const QRect& hint);
    QRect getPositionHint()             const;

protected:

    void paintEvent(QPaintEvent*)             override;
    void resizeEvent(QResizeEvent*)           override;
    void mousePressEvent(QMouseEvent*)        override;
    void mouseReleaseEvent(QMouseEvent*)      override;
    void mouseMoveEvent(QMouseEvent*)         override;

    void makePixmap();
    QRect composeImage(DImg* const image, QPainter* const destPainter,
                       int x, int y,
                       QFont font, float pointSize, int textRotation, QColor textColor,
                       int textOpacity, int alignMode, const QString& textString,
                       bool transparentBackground, QColor backgroundColor,
                       BorderMode borderMode, int borderWidth, int spacing, float fontScale = 1.0f);

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace DigikamEditorInsertTextToolPlugin
