/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-20
 * Description : Managing visibility state with animations
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QAbstractAnimation>
#include <QVariant>

// Local includes

#include "digikam_export.h"

class QEasingCurve;
class QPropertyAnimation;

namespace Digikam
{

class DIGIKAM_EXPORT ItemVisibilityController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool  shallBeShown READ shallBeShown WRITE setShallBeShown)
    Q_PROPERTY(bool  visible      READ isVisible    WRITE setVisible)

public:

    /**
     * @brief This class handles complex visibility situations for items.
     * There is a 3-tiered approach:
     * 1) shallBeShown determines if the items shall at any time be shown.
     *    If it is false, items will never be shown.
     *    Default is true, so you can ignore this setting.
     * 2) visible determines if the items shall be shown now.
     *    Only takes effect if shallBeShown is true.
     *    Default is false: Initially, controlled items are hidden.
     * 3) Opacity and individual item visibility:
     *    When showing, items are first set to individually visible,
     *    then their opacity is increased from 0 to 1.
     *    When hiding, opacity is first decreased from 1 to 0,
     *    then they are set individually to hidden.
     * Different types of items can be handled:
     * - a group of items with an "opacity" and "visible" property
     * - a single item with an "opacity" and "visible" property
     * - a proxy object with these properties (see above)
     */

    enum State
    {
        Hidden,
        FadingIn,
        Visible,
        FadingOut
    };
    Q_ENUM(State)

    enum IncludeFadingOutMode
    {
        /// In addition to items visible or fading in, return those fading out
        IncludeFadingOut,

        /// Do not return those items currently fading out (soon to be hidden)
        ExcludeFadingOut
    };

public:

    explicit ItemVisibilityController(QObject* const parent = nullptr);
    ~ItemVisibilityController() override;

    bool  shallBeShown()                                                        const;
    bool  isVisible()                                                           const;
    State state()                                                               const;

    /**
     * @return the "result" of isVisible and shallBeShown:
     * Something is indeed visible on the scene.
     * Also returns false if no items are available.
     */
    bool  hasVisibleItems(IncludeFadingOutMode mode = IncludeFadingOut)         const;

    /**
     * @brief Remove all animations.
     */
    void clear();

    /**
     * @brief Add and remove objects. The given objects shall provide
     * an "opacity" and a "visible" property.
     * You can, for convenience, use a ItemVisibilityControllerPropertyObject
     * as a value container, if your items do not provide these properties directly.
     * No ownership is taken, so the objects should live as long as this object
     * is used.
     */
    void addItem(QObject* const object);
    void removeItem(QObject* const object);

    /**
     * @return all items under control
     */
    QList<QObject*> items()                                                     const;

    /**
     * @return all currently visible items.
     */
    QList<QObject*> visibleItems(IncludeFadingOutMode mode = IncludeFadingOut)  const;

    /**
     * @brief Allows to change the default parameters of all animations.
     */
    void setEasingCurve(const QEasingCurve& easing);
    void setAnimationDuration(int msecs);

Q_SIGNALS:

    /**
     * @brief Emitted when the (main) transition has finished
     */
    void propertiesAssigned(bool visible);

    /**
     * @brief Emitted when a transition for a single item finished
     * (see setItemVisible())
     */
    void propertiesAssignedToItem(QObject* item, bool visible);

    /**
     * @brief Emitted when hideAndRemoveItem has finished
     */
    void hiddenAndRemoved(QObject* item);

public Q_SLOTS:

    /**
     * @brief Adjusts the first condition - the items are shown if shallBeShown is true and isVisible is true
     */
    void setShallBeShown(bool shallBeShown);
    void setShallBeShownDirectly(bool shallBeShown);

    /**
     * @brief Sets a single item to be shown. Calling setVisible() will effectively
     * effect only this single item, as if calling setItemVisible().
     * Reset by calling with 0 or setShallBeShown().
     */
    void setItemThatShallBeShown(QObject* item);

    /**
     * @brief Adjusts the main condition.
     * All items are affected.
     * If any items were shown or hidden separately, they will be resynchronized.
     * "Directly" means no animation is employed.
     */
    void show();
    void hide();
    void setVisible(bool visible);
    void setDirectlyVisible(bool visible);

    /**
     * @brief Shows or hides a single item.
     * The item's status is changed individually.
     * The next call to the "global" method will take precedence again.
     * "Directly" means no animation is employed.
     */
    void showItem(QObject* item);
    void hideItem(QObject* item);
    void setItemVisible(QObject* item, bool visible);
    void setItemDirectlyVisible(QObject* item, bool visible);

    /**
     * @brief Hide the item, and then remove it.
     * When finished, hiddenAndRemoved() is emitted.
     */
    void hideAndRemoveItem(QObject* item);

protected:

    /**
     * @brief Creates the animation for showing and hiding the given item.
     * The item is given for information only, you do not need to use it.
     * The default implementation creates and animation for "opacity"
     * from 0.0 to 1.0, using default easing curve and duration,
     * which can and will be changed by setEasingCurve and setAnimationDuration.
     */
    virtual QPropertyAnimation* createAnimation(QObject* item);

protected Q_SLOTS:

    void animationFinished();
    void objectDestroyed(QObject*);

private:

    /// @note disabled
    ItemVisibilityController(const ItemVisibilityController&)                         = delete;
    ItemVisibilityController& operator=(const ItemVisibilityController&)              = delete;

private:

    class Private;
    Private* const d = nullptr;
};

// ------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT ItemVisibilityControllerPropertyObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity   WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(bool  visible READ isVisible WRITE setVisible NOTIFY visibleChanged)

public:

    /**
     * @brief You can use this object as a container providing the properties
     * set by ItemVisibilityController.
     * Connect to the signals accordingly, e.g. to trigger a repaint.
     */

    explicit ItemVisibilityControllerPropertyObject(QObject* const parent = nullptr);

    qreal opacity()                 const;
    void setOpacity(qreal opacity);

    bool isVisible()                const;
    void setVisible(bool visible);

Q_SIGNALS:

    void opacityChanged();
    void visibleChanged();

protected:

    qreal m_opacity = 0.0;
    bool  m_visible = false;
};

// ------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT AnimatedVisibility : public ItemVisibilityControllerPropertyObject
{
    Q_OBJECT

public:

    /**
     * @brief A convenience class:
     * The property object brings its own controller.
     * Ready to use: Just construct an object and connect to the signals.
     * Please note the difference between controller()->setVisible() and setVisible():
     * You want to call the controller's method!
     */

    explicit AnimatedVisibility(QObject* const parent = nullptr);

    ItemVisibilityController* controller() const;

protected:

    ItemVisibilityController* m_controller = nullptr;
};

// ------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT HidingStateChanger : public ItemVisibilityController
{
    Q_OBJECT

public:

    /**
     * @brief This class provides a state change while fading in and out:
     * When changeValue is called, first the items are hidden,
     * when this is finished, the property is assigned to the object.
     * Afterwards, the items are shown again.
     * Note that the targetObject is not necessarily a controlled item!
     */

    explicit HidingStateChanger(QObject* const parent = nullptr);

    /**
     * @brief Convenience constructor: Sets target and property name
     */
    HidingStateChanger(QObject* const target, const QByteArray& property, QObject* const parent = nullptr);

    void setTargetObject(QObject* const object);
    void setPropertyName(const QByteArray& propertyName);

public Q_SLOTS:

    void changeValue(const QVariant& value);

Q_SIGNALS:

    /**
     * @brief Emitted when the items were hidden and the target object's property changed
     */
    void stateChanged();

    /**
     * @brief Emitted when the items were hidden, the target object's property changed, and the items shown again
     */
    void finished();

protected Q_SLOTS:

    void slotPropertiesAssigned(bool);

protected:

    QObject*   m_object     = nullptr;
    QByteArray m_property;
    QVariant   m_value;
};

} // namespace Digikam
