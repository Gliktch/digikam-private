/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2023-05-15
 * Description : geolocation engine based on Marble.
 *               (c) 2007-2022 Marble Team
 *               https://invent.kde.org/education/marble/-/raw/master/data/credits_authors.html
 *
 * SPDX-FileCopyrightText: 2023-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QPointF>
#include <QSizeF>

// Local includes

#include "RenderPlugin.h"
#include "FrameGraphicsItem.h"
#include "digikam_export.h"

class QContextMenuEvent;
class QHelpEvent;
class QMenu;
class QWidget;
class QFont;
class QPen;

namespace Marble
{

class AbstractFloatItemPrivate;

/**
 * @brief The abstract class for float item plugins
 *
 * Float Item is a variant of Marble render plugins
 * It keeps floating on top of the map at a given screen position
 *
 * Good examples are Overview Map, License
 *
 */

class DIGIKAM_EXPORT AbstractFloatItem : public RenderPlugin,
                                         public FrameGraphicsItem
{
    Q_OBJECT

public:

    explicit AbstractFloatItem(const MarbleModel* marbleModel,
                               const QPointF& point = QPointF(10.0, 10.0),
                               const QSizeF& size = QSizeF(150.0, 50.0));
    ~AbstractFloatItem()                                                      override;

    QHash<QString, QVariant> settings()                                 const override;
    void setSettings(const QHash<QString, QVariant>& settings)                override;

    RenderType renderType()                                             const override;

    /**
     * @brief current pen for rendering
     * @return pen
     */
    QPen pen()                                                          const;

    /**
     * @brief setting current pen for rendering
     * @param pen
     */
    void setPen(const QPen& pen);

    /**
     * @brief current font for rendering
     * @return font
     */
    QFont font()                                                        const;

    /**
     * @brief setting current font for rendering
     * @param font
     */
    void setFont(const QFont& font);

    /**
     * @brief Paints the float item on the map.
     * @deprecated Do not override this method since it won't be called any longer.
     *             Override one of FrameGraphicsItem's paint methods instead.
     */
    bool render(GeoPainter* painter, ViewportParams* viewport,
                const QString& renderPos = QLatin1String("FLOAT_ITEM"),
                GeoSceneLayer* layer = nullptr)                               override;

    QString renderPolicy()                                              const override;

    /**
     * @brief Returns the rendering position of this float item.
     * @deprecated The return value of method is ignored. The float item's rendering position
     *             will always be "FLOAT_ITEM".
     */
    QStringList renderPosition()                                        const override;

    /**
     * @brief Set visibility of the float item
     *
     * Float items can be visible or invisible.
     * It's possible to check visibility with @see visible
     *
     * @param visible visibility of the item
     */
    // cppcheck-suppress duplInheritedMember
    void setVisible(bool visible);

    /**
     * @brief Check visibility of the float item
     *
     * Float items can be visible or invisible.
     * It's possible to set visibility with @see setVisible
     *
     * @return visible or not
     */
    // cppcheck-suppress duplInheritedMember
    bool visible()                                                      const;

    /**
     * @brief Check is position locked
     *
     * Float Item position can be locked. If it is,
     * the item cannot be moved with the cursor (in the UI)
     *
     * To set it use @see setPositionLocked
     *
     * @return position locked or not
     */
    bool positionLocked()                                               const;

public Q_SLOTS:

    /**
     * @brief Set is position locked
     * @param lock is locked?
     *
     * Float Item position can be locked. If it is,
     * item cannot be moved with cursor (in UI)
     *
     * To check it use @see positionLocked
     *
     */
    void setPositionLocked(bool lock);

    /**
     * @brief Show the item
     *
     * If the item was hidden this function will show it
     *
     */
    void show();        // cppcheck-suppress duplInheritedMember

    /**
     * @brief Hide the item
     *
     * If the item was shown this function will hide it
     *
     */
    void hide();        // cppcheck-suppress duplInheritedMember

protected:

    bool eventFilter(QObject* object, QEvent* e)                              override;
    virtual void contextMenuEvent(QWidget* w, QContextMenuEvent* e);
    virtual void toolTipEvent(QHelpEvent* e);
    QMenu* contextMenu();

private:

    Q_DISABLE_COPY(AbstractFloatItem)

private:

    AbstractFloatItemPrivate* const d = nullptr;
};

} // Namespace Marble
