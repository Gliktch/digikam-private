/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View item for DImg
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QtGlobal>
#include <QGraphicsObject>
#include <QObject>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DImg;
class ImageZoomSettings;

class DIGIKAM_EXPORT GraphicsDImgItem : public QGraphicsObject
{
    Q_OBJECT

public:

    explicit GraphicsDImgItem(QGraphicsItem* const parent = nullptr);
    ~GraphicsDImgItem()                                               override;

    /**
     * @brief Sets the DImg to be drawn by this item.
     * @note DImg is explicitly shared, and no copy is automatically taken here.
     */
    void setImage(const DImg& img);
    DImg image()                                                const;

    const ImageZoomSettings* zoomSettings()                     const;
    ImageZoomSettings*       zoomSettings();

    void setEnableUnderExposure(bool b);
    void setEnableOverExposure(bool b);

    void setDisplayWidget(QWidget* const widget);

    void sizeHasChanged();
    void clearCache();

    QRectF boundingRect()                                       const override;
    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget)                                       override;

    virtual QString userLoadingHint()                           const
    {
        return QString();
    }

Q_SIGNALS:

    void showContextMenu(QGraphicsSceneContextMenuEvent* e);
    void imageChanged();
    void imageSizeChanged(const QSizeF& size);

protected:

    void contextMenuEvent(QGraphicsSceneContextMenuEvent* e)          override;

public:

    /**
     * @note Declared public because of DImgPreviewItemPrivate.
     */
    class GraphicsDImgItemPrivate;

protected:

    GraphicsDImgItem(GraphicsDImgItemPrivate& dd, QGraphicsItem* const parent);
    GraphicsDImgItemPrivate* const d_ptr;

protected:

    Q_DECLARE_PRIVATE(GraphicsDImgItem)

private:

    /// @note disabled
    GraphicsDImgItem(QObject*);
};

} // namespace Digikam
