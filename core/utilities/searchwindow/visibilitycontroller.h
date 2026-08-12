/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : User interface for searches
 *
 * SPDX-FileCopyrightText: 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#pragma once

// Qt includes

#include <QWidget>
#include <QList>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT VisibilityObject
{
public:

    VisibilityObject()                    = default;
    virtual ~VisibilityObject()           = default;
    virtual void setVisible(bool visible) = 0;
    virtual bool isVisible()              = 0;

private:

    Q_DISABLE_COPY(VisibilityObject)
};

// -----------------------------------------------------------------------------------

class DIGIKAM_GUI_EXPORT VisibilityController : public QObject
{
    Q_OBJECT

public:

    enum Status
    {
        Unknown,
        Hidden,
        Showing,
        Shown,
        Hiding
    };

public:

    explicit VisibilityController(QObject* const parent);
    ~VisibilityController() override;

    /**
     * Set the widget containing the widgets added to this controller
     */
    void setContainerWidget(QWidget* const widget);

    /**
     * Add a widget to this controller.
     */
    void addWidget(QWidget* const widget);

    /**
     * Add an object implementing the VisibilityObject interface.
     * You can use this if you have your widgets grouped in intermediate objects.
     */
    void addObject(VisibilityObject* const object);

    /**
     * Returns true if the contained objects are visible or becoming visible.
     */
    bool isVisible() const;

public Q_SLOTS:

    /// Shows/hides all added objects
    void setVisible(bool visible);
    void show();
    void hide();

    /// Shows if hidden and hides if visible.
    void triggerVisibility();

protected:

    void step();
    void allSteps();

    virtual void beginStatusChange();

private:

    class Private;
    Private* const d = nullptr;
};

} // namespace Digikam
